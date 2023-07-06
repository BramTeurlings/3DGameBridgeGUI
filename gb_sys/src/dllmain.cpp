// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "hooks.h"

#include <Windows.h>
#include <iostream>
#include <psapi.h>

#ifdef GAME_BRIDGE_API_EXPORTS
#define GAME_BRIDGE_API __declspec(dllexport)
#else
#define GAME_BRIDGE_API __declspec(dllimport)
#endif

namespace game_bridge {
    extern "C" GAME_BRIDGE_API void init_api();

    // Subsribe to process events
    extern "C" GAME_BRIDGE_API void subscribe_to_pocess_events();

    extern "C" GAME_BRIDGE_API void unsubscribe_from_process_events();

    extern "C" GAME_BRIDGE_API void set_process_event_callback();

    extern "C" GAME_BRIDGE_API void inject_into_process();
}

// Global variables
HHOOK g_hook = NULL;

// Callback function to handle process creation events
LRESULT CALLBACK HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && wParam == HSHELL_WINDOWCREATED)
    {
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

    // Pass the event to the next hook procedure
    return CallNextHookEx(g_hook, nCode, wParam, lParam);
}

extern "C" GAME_BRIDGE_API void InstallHook()
{
    // Install the hook
    g_hook = SetWindowsHookEx(WH_SHELL, HookCallback, NULL, 0);
    if (!g_hook)
    {
        std::cerr << "Failed to install hook." << std::endl;
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

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
