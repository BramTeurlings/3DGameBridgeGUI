#include "process_detection.h"

namespace game_bridge
{
    // Global variables
    HHOOK g_hook = NULL;
    HINSTANCE g_hInstance = NULL;
    LRESULT HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
    {
        if (nCode == HC_ACTION && wParam == HSHELL_WINDOWCREATED)
        {
            HWND hWnd = reinterpret_cast<HWND>(lParam);
            char processName[MAX_PATH];
            GetWindowTextA(hWnd, processName, MAX_PATH);
            std::cout << "Process created: " << processName << std::endl;
        }

        return CallNextHookEx(g_hook, nCode, wParam, lParam);
    }

    ProcessCreationListener::ProcessCreationListener()
    {

    }

    void ProcessCreationListener::StartListening()
    {
        // Get the module handle of the current executable
        g_hInstance = GetModuleHandle(NULL);

        // Install the hook
        g_hook = SetWindowsHookEx(WH_SHELL, HookCallback, g_hInstance, 0);
        if (!g_hook)
        {
            DWORD err = GetLastError();
            std::cerr << "Failed to install hook: " << err << std::endl;
        }

        // Message loop to keep the program running
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Uninstall the hook
        UnhookWindowsHookEx(g_hook);
    }
}