#include "hooks.h"

#include <oleacc.h>
#pragma comment( lib, "oleacc" )

#include <iostream>
#include <sstream>

// Global variable.
HWINEVENTHOOK g_hook;
HHOOK hHook;

// Initializes COM and sets up the event hook.
//
void InitializeMSAA(HINSTANCE hInstance)
{
    CoInitialize(NULL);
    g_hook = SetWinEventHook(
        EVENT_OBJECT_CREATE, EVENT_OBJECT_DESTROY,  // Range of events (4 to 5).
        NULL,                                          // Handle to DLL.
        HandleWinEvent,                                // The callback.
        0, 0,              // Process and thread IDs of interest (0 = all)
        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS); // Flags.

    //hHook = SetWindowsHookEx(WH_CALLWNDPROCRET, WindowProc, hInstance, 0);
}

// Unhooks the event and shuts down COM.
//
void ShutdownMSAA()
{
    UnhookWinEvent(g_hook);
    CoUninitialize();
}

// Callback function that handles events.
//
void CALLBACK HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
    LONG idObject, LONG idChild,
    DWORD dwEventThread, DWORD dwmsEventTime)
{
    IAccessible* pAcc = NULL;
    VARIANT varChild;
    HRESULT hr = AccessibleObjectFromEvent(hwnd, idObject, idChild, &pAcc, &varChild);
    if ((hr == S_OK) && (pAcc != NULL))
    {
        BSTR bstrName;
        std::wstring gg;
        pAcc->get_accName(varChild, (BSTR*)gg.data());
        gg.resize(CHAR_MAX);
        if (bstrName) {
            //if (event == EVENT_OBJECT_CREATE)
            //{
            //    std::cout << "Begin: " << "\n";
            //}
            //else if (event == EVENT_OBJECT_DESTROY)
            //{
            //    std::cout << "End:   " << "\n";
            //}
            std::wstringstream ss; ss << L"Name: " << bstrName << L"\n";
            std::wcout << ss.str();
        }
        SysFreeString(bstrName);
        pAcc->Release();
    }
}

LRESULT CALLBACK WindowProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        CWPRETSTRUCT* pMsg = (CWPRETSTRUCT*)lParam;
        if (pMsg->message == WM_CREATE)
        {
            // A new window is created
            HWND hWnd = (HWND)pMsg->lParam;
            // Do something with the new window handle
        }
    }

    return CallNextHookEx(hHook, nCode, wParam, lParam);
}
