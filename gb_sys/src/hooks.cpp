#include "hooks.h"

// Global variables
HHOOK g_hook = NULL;
HINSTANCE g_hInstance = NULL;

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

    return CallNextHookEx(g_hook, nCode, wParam, lParam);
}

void proc(WPARAM wParam, LPARAM lParam) {
    // Extract the process ID from the window handle
    HWND hWnd = reinterpret_cast<HWND>(lParam);
    DWORD dwProcessId;
    GetWindowThreadProcessId(hWnd, &dwProcessId);

    // Open the process
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId);
    if (hProcess)
    {
        // Get the process name
        TCHAR szProcessName[MAX_PATH];
        if (GetModuleFileNameEx(hProcess, NULL, szProcessName, MAX_PATH))
        {
            std::wcout << L"Process started: " << szProcessName << std::endl;
        }
        else
        {
            std::cerr << "Failed to get process name." << std::endl;
        }

        // Clean up
        CloseHandle(hProcess);
    }
    else
    {
        std::cerr << "Failed to open process." << std::endl;
    }
}

// Callback function to handle process creation events
LRESULT CALLBACK HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode > 0) {
        switch (nCode)
        {
        case HSHELL_WINDOWCREATED: 
        {
            std::cout << "HSHELL_WINDOWCREATED" << "\n";
            proc(wParam, lParam);
            break;
        }
        case HSHELL_WINDOWDESTROYED: std::cout << "HSHELL_WINDOWDESTROYED" << "\n"; break;
        case HSHELL_ACTIVATESHELLWINDOW: std::cout << "HSHELL_ACTIVATESHELLWINDOW" << "\n"; break;
        case HSHELL_WINDOWACTIVATED: std::cout << "HSHELL_WINDOWACTIVATED" << "\n"; break;
        case HSHELL_GETMINRECT: std::cout << "HSHELL_GETMINRECT" << "\n"; break;
        case HSHELL_REDRAW: std::cout << "HSHELL_REDRAW" << "\n"; break;
        case HSHELL_TASKMAN: std::cout << "HSHELL_TASKMAN" << "\n"; break;
        case HSHELL_LANGUAGE: std::cout << "HSHELL_LANGUAGE" << "\n"; break;
        case HSHELL_SYSMENU: std::cout << "HSHELL_SYSMENU" << "\n"; break;
        case HSHELL_ENDTASK: std::cout << "HSHELL_ENDTASK" << "\n"; break;
        case HSHELL_ACCESSIBILITYSTATE: std::cout << "HSHELL_ACCESSIBILITYSTATE" << "\n"; break;
        case HSHELL_APPCOMMAND: std::cout << "HSHELL_APPCOMMAND" << "\n"; break;
        case HSHELL_WINDOWREPLACED: std::cout << "HSHELL_WINDOWREPLACED" << "\n"; break;
        case HSHELL_WINDOWREPLACING: std::cout << "HSHELL_WINDOWREPLACING" << "\n"; break;
        case HSHELL_MONITORCHANGED: std::cout << "HSHELL_MONITORCHANGED" << "\n"; break;
        default:
            break;
        }
    }

    // Pass the event to the next hook procedure
    return CallNextHookEx(g_hook, nCode, wParam, lParam);
}

extern "C" GAME_BRIDGE_API void InstallHook(HINSTANCE hInstance)
{
    // Install the hook
    g_hInstance = hInstance;
    g_hook = SetWindowsHookEx(WH_SHELL, HookCallback, g_hInstance, 0);
    if (!g_hook)
    {
        std::cerr << "Failed to install hook: " << GetLastError() << std::endl;
    }
}

extern "C" GAME_BRIDGE_API void UninstallHook()
{
    // Uninstall the hook
    if (!UnhookWindowsHookEx(g_hook))
    {
        std::cerr << "Failed to uninstall hook." << std::endl;
    }
}

