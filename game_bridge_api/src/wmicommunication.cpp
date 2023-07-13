#include "wmicommunication.h"

#include <iostream>
#include <chrono>
#include <comdef.h>
#include <filesystem>
#include <sstream>
#pragma comment(lib, "wbemuuid.lib")

static inline ProcessDetectionData detection_data;
// Thread stuff
// Todo May need a mutex when setting the callback, leaving it out for now
PTP_WORK_CALLBACK workcallback = ProcessEnumerationCallback;
WinThreadPool thread_pool;

loading_data payload64;

struct ThreadSpecificData
{
    ProcessDetectionData* dData;
    VARIANT vtProp;
    IWbemClassObject* apObjectl;
};

void WmiSearchCallback(PTP_CALLBACK_INSTANCE instance, PVOID parameter, PTP_WORK work)
{
    ThreadSpecificData* data = reinterpret_cast<ThreadSpecificData*>(parameter);
    VARIANT vtProp = data->vtProp;
    IWbemClassObject* pProcessObj = nullptr;

    auto c_before = std::chrono::high_resolution_clock::now();
    if (vtProp.punkVal->QueryInterface(IID_IWbemClassObject, reinterpret_cast<void**>(&pProcessObj)) >= 0)
    {
        // Get process data
        VARIANT vtProcessId{};
        Win32ProcessData process_data;
        if (pProcessObj->Get(L"ProcessId", 0, &vtProcessId, 0, 0) >= 0 && vtProcessId.punkVal != nullptr)
        {
            process_data.pid = vtProcessId.intVal;
        }

        VARIANT vtName{};
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

        // Do injection here
        std::string exe_name = std::filesystem::path(process_data.executable_path).filename().string();
        std::for_each(data->dData->supported_titles.begin(), data->dData->supported_titles.end(), [&](std::string a) {
            if (exe_name.compare(a) == 0) {
                // Performance check
                auto b_before = std::chrono::high_resolution_clock::now();
                InjectIntoApplication(process_data.pid, payload64);

                // Performance check
                auto time_after = std::chrono::high_resolution_clock::now();

                std::stringstream ss; ss << "injection time: " << std::chrono::duration_cast<std::chrono::milliseconds>(time_after - b_before).count() << " --- " << exe_name << std::endl;
                ss << "wmi process detect time: " << std::chrono::duration_cast<std::chrono::milliseconds>(c_before - data->dData->pr_start_tm).count() << std::endl;
                ss << "exe search time micro: " << std::chrono::duration_cast<std::chrono::microseconds>(b_before - c_before).count() << std::endl;
                ss << "ms Total time: " << std::chrono::duration_cast<std::chrono::milliseconds>(time_after - data->dData->pr_start_tm).count() << std::endl;
                std::cout << ss.str();
            }
            });


        VariantClear(&vtProcessId);
        VariantClear(&vtName);
    }
    // Delete process object
    pProcessObj->Release();
    data->apObjectl->Release(); // Release reference set in the Indicate callback function
    delete data;

    // Delete variant
    VariantClear(&vtProp);
}

void ProcessEnumerationCallback(PTP_CALLBACK_INSTANCE instance, PVOID parameter, PTP_WORK work)
{
    // Instance, Parameter, and Work not used in this example.
    //UNREFERENCED_PARAMETER(instance);
    //UNREFERENCED_PARAMETER(parameter);
    UNREFERENCED_PARAMETER(work);

    //
    // Do something when the work callback is invoked.
    //
    DWORD pid = 0;
    auto c_before = std::chrono::high_resolution_clock::now();
    const scoped_handle snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    PROCESSENTRY32W process = { sizeof(process) };
    for (BOOL next = Process32FirstW(snapshot, &process); next; next = Process32NextW(snapshot, &process))
    {
        if (wcscmp(process.szExeFile, L"ULTRAKILL.exe") == 0)
        {
            pid = process.th32ProcessID;
        }
    }

    if (pid)
    {
        auto b_before = std::chrono::high_resolution_clock::now();
        InjectIntoApplication(pid, payload64);

        auto time_after = std::chrono::high_resolution_clock::now();


        ThreadSpecificData* data = reinterpret_cast<ThreadSpecificData*>(parameter);

        std::cout << "exe search time: " << std::chrono::duration_cast<std::chrono::milliseconds>(b_before - c_before).count() << std::endl;
        std::cout << "injection time: " << std::chrono::duration_cast<std::chrono::milliseconds>(time_after - b_before).count() << std::endl;
        std::cout << "wmi process detect time: " << std::chrono::duration_cast<std::chrono::milliseconds>(c_before - data->dData->pr_start_tm).count() << std::endl;
        std::cout << "FOUND Total time: " << std::chrono::duration_cast<std::chrono::milliseconds>(time_after - data->dData->pr_start_tm).count() << std::endl;
    }
    else
    {
        std::cout << "NOT FOUND\n";
    }
}

void SetIndicateEventCallback(void(*work_callback)(PTP_CALLBACK_INSTANCE instance, PVOID parameter, PTP_WORK work))
{
    workcallback = work_callback;
}

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

HRESULT EventSink::Indicate(LONG lObjectCount, IWbemClassObject** apObjArray)
{
    for (LONG i = 0; i < lObjectCount; i++)
    {
        // Create new data here so every thread has their own wmi object data. Todo maybe remove the new call later for extra performance
        ThreadSpecificData* thread_data = new ThreadSpecificData();
        // Detection data is shared between threads, but is only read from. When reloading the game list, make sure no threads are reading from this variable.
        // This way, synchronization should not be necessary.
        thread_data->dData = &detection_data;

        VARIANT vtProp {};
		if (apObjArray[i]->Get(L"TargetInstance", 0, &vtProp, 0, 0) >= 0)
		{
            // Add ref here, release in the worker thread.
            apObjArray[i]->AddRef();
            thread_data->apObjectl = apObjArray[i];
            thread_data->vtProp = vtProp;
            thread_pool.StartWork(workcallback, thread_data);

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

void WMICommunication::initialize_payload(const std::string& sr_binary_path)
{
    CreatePayload(sr_binary_path, payload64);
}

long WMICommunication::InitializeObjects(const char* query)
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
        Deinitialize();
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
        Deinitialize();
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
        Deinitialize();
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
        Deinitialize();
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

    // Check for errors.
    if (FAILED(hres))
    {
        printf("ExecNotificationQueryAsync failed "
            "with = 0x%X\n", hres);
        Deinitialize();
    }

    return hres;
}

long WMICommunication::Deinitialize()
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
    Deinitialize();
}

void WMICommunication::SetDetectionData(const ProcessDetectionData& val)
{
    detection_data = val;
}

ProcessDetectionData& WMICommunication::GetDetectionData() {
    return detection_data;
}
