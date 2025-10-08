#include "wmicommunication.h"

#include <iostream>
#include <chrono>
#include <comdef.h>
#pragma comment(lib, "wbemuuid.lib")

ULONG EventSink::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

ULONG EventSink::Release()
{
    LONG lRef = InterlockedDecrement(&m_lRef);
    if (lRef == 0)
        delete this;
    return lRef;
}

HRESULT EventSink::QueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IUnknown || riid == IID_IWbemObjectSink)
    {
        *ppv = (IWbemObjectSink*)this;
        AddRef();
        return WBEM_S_NO_ERROR;
    }
    else return E_NOINTERFACE;
}

//HRESULT EventSink::Indicate(long lObjectCount, IWbemClassObject** apObjArray)
//{
//    HRESULT hres = S_OK;
//
//    for (int i = 0; i < lObjectCount; i++)
//    {
//        printf("Event occurred\n");
//    }
//
//    return WBEM_S_NO_ERROR;
//}

HRESULT EventSink::Indicate(LONG lObjectCount, IWbemClassObject** apObjArray)
{
    auto time_before = std::chrono::high_resolution_clock::now();
    for (LONG i = 0; i < lObjectCount; i++)
    {
        Win32ProcessData process_data;

        VARIANT vtProp {};
		if (apObjArray[i]->Get(L"TargetInstance", 0, &vtProp, 0, 0) >= 0)
		{
			IWbemClassObject* pProcessObj = nullptr;
			if (vtProp.punkVal->QueryInterface(IID_IWbemClassObject, reinterpret_cast<void**>(&pProcessObj)) >= 0)
			{
				// Get process data
                VARIANT vtProcessId {};
				if (pProcessObj->Get(L"ProcessId", 0, &vtProcessId, 0, 0) >= 0 && vtProcessId.punkVal != nullptr)
				{
					process_data.pid = vtProcessId.intVal;
				}

                VARIANT vtName {};
				if (pProcessObj->Get(L"ExecutablePath", 0, &vtName, 0, 0) >= 0 && vtName.punkVal != nullptr)
				{
					size_t char_number = 0;
					std::wstring wexe_path(vtName.bstrVal);
					CHAR exe_path[MAX_PATH];
					errno_t err = wcstombs_s(&char_number, exe_path, MAX_PATH, wexe_path.c_str(), _TRUNCATE);
					if (err != EINVAL)
					{
						process_data.executable_path = exe_path;
					}
					else
					{
						wprintf(L"Process detection: Couldn't convert executable path to wstring");
					}
				}

                // Add message to queue before releasing pointers
                if (process_data.pid > 0 && !process_data.executable_path.empty())
                {
                    message_queue.push(process_data);
                    semaphore_message_queue.notify();
                }
                else
                {
					wprintf(L"Couldn't enqueue message from wmi\n");
				}

				auto time_after = std::chrono::high_resolution_clock::now();
				std::cout << "ms time of Indicate: " << std::chrono::duration_cast<std::chrono::microseconds>(time_after - time_before).count() << "\n";

				VariantClear(&vtProcessId);
				VariantClear(&vtName);
			}
			// Delete process object
			pProcessObj->Release();

			// Delete variant
			VariantClear(&vtProp);
		}

        // Todo check memory leak in this function
        // This statement makes the thread throw errors and this is not used un Microsoft examples though
		//apObjArray[i]->Release();
    }


	return WBEM_S_NO_ERROR;
}

//// Get the main window handle of the process
//HWND hWnd = FindWindowW(nullptr, vtName.bstrVal);
//if (hWnd != nullptr)
//{
//    std::cout << "Window handle: " << hWnd << std::endl;
//}
//else
//{
//    std::cerr << "Failed to find the main window handle." << std::endl;

HRESULT EventSink::SetStatus(
    /* [in] */ LONG lFlags,
    /* [in] */ HRESULT hResult,
    /* [in] */ BSTR strParam,
    /* [in] */ IWbemClassObject __RPC_FAR* pObjParam
)
{
    if (lFlags == WBEM_STATUS_COMPLETE)
    {
        printf("Call complete. hResult = 0x%X\n", hResult);
    }
    else if (lFlags == WBEM_STATUS_PROGRESS)
    {
        printf("Call in progress.\n");
    }

    return WBEM_S_NO_ERROR;
}    // end of EventSink.cpp

long WMICommunication::initializeObjects(const char* query)
{
    HRESULT hres;

    // Step 1: --------------------------------------------------
    // Initialize COM. ------------------------------------------

    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
    {
        std::cout << "Failed to initialize COM library. Error code = 0x"
            << std::hex << hres << std::endl;
        return hres;
    }

    // Step 2: --------------------------------------------------
    // Set general COM security levels --------------------------

    hres = CoInitializeSecurity(
        NULL,
        -1,                          // COM negotiates service
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities 
        NULL                         // Reserved
    );


    if (FAILED(hres))
    {
        std::cout << "Failed to initialize security. Error code = 0x"
            << std::hex << hres << std::endl;
        uninitialize();
        return hres;
    }

    // Step 3: ---------------------------------------------------
    // Obtain the initial locator to WMI -------------------------

    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);

    if (FAILED(hres))
    {
        std::cout << "Failed to create IWbemLocator object. "
            << "Err code = 0x"
            << std::hex << hres << std::endl;
        uninitialize();
        return hres;
    }

    // Step 4: ---------------------------------------------------
    // Connect to WMI through the IWbemLocator::ConnectServer method

    // Connect to the local root\cimv2 namespace
    // and obtain pointer pSvc to make IWbemServices calls.
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL,
        NULL,
        0,
        NULL,
        0,
        0,
        &pSvc
    );

    if (FAILED(hres))
    {
        std::cout << "Could not connect. Error code = 0x"
            << std::hex << hres << std::endl;
        uninitialize();
        return hres;
    }

    std::cout << "Connected to ROOT\\CIMV2 WMI namespace" << std::endl;

    // Step 5: --------------------------------------------------
    // Set security levels on the proxy -------------------------

    hres = CoSetProxyBlanket(
        pSvc,                        // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx 
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx 
        NULL,                        // Server principal name 
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities 
    );

    if (FAILED(hres))
    {
        std::cout << "Could not set proxy blanket. Error code = 0x"
            << std::hex << hres << std::endl;
        uninitialize();
        return hres;
    }

    // Step 6: -------------------------------------------------
    // Receive event notifications -----------------------------

    // Use an unsecured apartment for security
    hres = CoCreateInstance(CLSID_UnsecuredApartment, NULL,
        CLSCTX_LOCAL_SERVER, IID_IUnsecuredApartment,
        (void**)&pUnsecApp);

    // Add reference to pSink
    pSink = new EventSink();
    pSink->AddRef();

    // Something with initializing pStubUnk I think (IUnknown)
    pUnsecApp->CreateObjectStub(pSink, &pStubUnk);

    // What the hell does this?
    pStubUnk->QueryInterface(IID_IWbemObjectSink,
        (void**)&pStubSink);

    // Actual event creation I think
    // The ExecNotificationQueryAsync method will call
    // The EventQuery::Indicate method when an event occurs
    hres = pSvc->ExecNotificationQueryAsync(
        _bstr_t("WQL"),
        _bstr_t("SELECT * "
            "FROM __InstanceCreationEvent WITHIN 1 "
            "WHERE TargetInstance ISA 'Win32_Process'"),
        WBEM_FLAG_SEND_STATUS,
        NULL,
        pStubSink);

    //Sleep(100000);

    // Check for errors.
    if (FAILED(hres))
    {
        printf("ExecNotificationQueryAsync failed "
            "with = 0x%X\n", hres);
        uninitialize();
    }

    return hres;
}

long WMICommunication::uninitialize()
{
    HRESULT hres;
    hres = pSvc->CancelAsyncCall(pStubSink);

    // Cleanup
    // ========

    if (pSvc != NULL) {
        pSvc->Release();
    }
    if (pLoc != NULL) {
        pLoc->Release();
    }
    if (pUnsecApp != NULL) {
        pUnsecApp->Release();
    }
    if (pStubUnk != NULL) {
        pStubUnk->Release();
    }
    if (pSink != NULL) {
        pSink->Release();
    }
    if (pStubSink != NULL) {
        pStubSink->Release();
    }

    CoUninitialize();

    return hres;
}

WMICommunication::WMICommunication(const char* query)
{
}

WMICommunication::~WMICommunication()
{
    uninitialize();
}
