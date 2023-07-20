#include "hooks.h"

#include <oleacc.h>
#pragma comment( lib, "oleacc" )

#include <iostream>

// Global variable.
HWINEVENTHOOK g_hook;
HHOOK hHook;

// Initializes COM and sets up the event hook.
//
void InitializeMSAA(HINSTANCE hInstance)
{
    //CoInitialize(NULL);
    //g_hook = SetWinEventHook(
    //    EVENT_OBJECT_CREATE, EVENT_OBJECT_DESTROY,  // Range of events (4 to 5).
    //    NULL,                                          // Handle to DLL.
    //    HandleWinEvent,                                // The callback.
    //    0, 0,              // Process and thread IDs of interest (0 = all)
    //    WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS); // Flags.

    hHook = SetWindowsHookEx(WH_CALLWNDPROCRET, WindowProc, hInstance, 0);
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
        pAcc->get_accName(varChild, &bstrName);
        if (event == EVENT_SYSTEM_MENUSTART)
        {
            std::cout << "Begin: " << std::endl;
        }
        else if (event == EVENT_SYSTEM_MENUEND)
        {
            std::cout << "End:   " << std::endl;
        }
        std::cout << "%S" << bstrName << std::endl;
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
