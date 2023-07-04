#include "file_functions.h"
#include "app.h"

#include <Windows.h>
#include <iostream>

typedef void (*InstallHookFunc)();
typedef void (*UninstallHookFunc)();

static HINSTANCE hModule;
static HHOOK hhookSysMsg;

/*
 * Todo:
 * Parse environment vars and arguments
 * Load config file for supported games
 * Display supported games
 * Listen for processes
 * Runtime injection into process
 *
 * Menu?
 */

using namespace std;
int main(int argc,      // Number of strings in array argv
    char* argv[],   // Array of command-line argument strings
    char* envp[])  // Array of environment variable strings
{
    int count;
    // Display each command-line argument.
    //cout << "\nCommand-line arguments:\n";
    for (count = 0; count < argc; count++) {
        //cout << "  argv[" << count << "]   " << argv[count] << "\n";
    }

    bool numberLines = false;    // Default is no line numbers.

    // If /n is passed to the .exe, display numbered listing
    // of environment variables.
    if ((argc == 2) && _stricmp(argv[1], "/n") == 0) {
        numberLines = true;
    }

    // Walk through list of strings until a NULL is encountered.
    for (int i = 0; envp[i] != NULL; ++i)
    {
        if (numberLines) {
            //cout << i << ": "; // Prefix with numbers if /n specified
        }
        //cout << envp[i] << "\n";
    }

    game_bridge::GameBridgeInjectionCLI application;
}

// HelloWindowsDesktop.cpp
// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c
//
//#include <windows.h>
//#include <stdlib.h>
//#include <string.h>
//#include <tchar.h>
//
//// Global variables
//
//// The main window class name.
//static TCHAR szWindowClass[] = _T("DesktopApp");
//
//// The string that appears in the application's title bar.
//static TCHAR szTitle[] = _T("Windows Desktop Guided Tour Application");
//
//// Stored instance handle for use in Win32 API calls such as FindResource
//HINSTANCE hInst;
//
//// Forward declarations of functions included in this code module:
//LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
//
//int WINAPI main(
//    _In_ HINSTANCE hInstance,
//    _In_opt_ HINSTANCE hPrevInstance,
//    _In_ LPSTR     lpCmdLine,
//    _In_ int       nCmdShow
//)
//{
//    WNDCLASSEX wcex;
//
//    wcex.cbSize = sizeof(WNDCLASSEX);
//    wcex.style = CS_HREDRAW | CS_VREDRAW;
//    wcex.lpfnWndProc = WndProc;
//    wcex.cbClsExtra = 0;
//    wcex.cbWndExtra = 0;
//    wcex.hInstance = hInstance;
//    wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
//    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
//    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
//    wcex.lpszMenuName = NULL;
//    wcex.lpszClassName = szWindowClass;
//    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);
//
//    if (!RegisterClassEx(&wcex))
//    {
//        MessageBox(NULL,
//            _T("Call to RegisterClassEx failed!"),
//            _T("Windows Desktop Guided Tour"),
//            NULL);
//
//        return 1;
//    }
//
//    // Store instance handle in our global variable
//    hInst = hInstance;
//
//    // The parameters to CreateWindowEx explained:
//    // WS_EX_OVERLAPPEDWINDOW : An optional extended window style.
//    // szWindowClass: the name of the application
//    // szTitle: the text that appears in the title bar
//    // WS_OVERLAPPEDWINDOW: the type of window to create
//    // CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
//    // 500, 100: initial size (width, length)
//    // NULL: the parent of this window
//    // NULL: this application does not have a menu bar
//    // hInstance: the first parameter from WinMain
//    // NULL: not used in this application
//    HWND hWnd = CreateWindowEx(
//        WS_EX_OVERLAPPEDWINDOW,
//        szWindowClass,
//        szTitle,
//        WS_OVERLAPPEDWINDOW,
//        CW_USEDEFAULT, CW_USEDEFAULT,
//        500, 100,
//        NULL,
//        NULL,
//        hInstance,
//        NULL
//    );
//
//    if (!hWnd)
//    {
//        MessageBox(NULL,
//            _T("Call to CreateWindow failed!"),
//            _T("Windows Desktop Guided Tour"),
//            NULL);
//
//        return 1;
//    }
//
//    // The parameters to ShowWindow explained:
//    // hWnd: the value returned from CreateWindow
//    // nCmdShow: the fourth parameter from WinMain
//    ShowWindow(hWnd, 1);
//    UpdateWindow(hWnd);
//
//    hModule = LoadLibrary("GameBridgeSysd.dll");
//    if (hModule)
//    {
//        InstallHookFunc installHook = reinterpret_cast<InstallHookFunc>(GetProcAddress(hModule, "InstallHook"));
//        UninstallHookFunc uninstallHook = reinterpret_cast<UninstallHookFunc>(GetProcAddress(hModule, "UninstallHook"));
//        HOOKPROC hookProc = (HOOKPROC)GetProcAddress(hModule, "HookCallback");
//
//        // Set hook on the dll to call the ShellProc function
//        hhookSysMsg = SetWindowsHookEx(WH_SHELL, hookProc, hModule, 0);
//        if (!hhookSysMsg)
//        {
//            std::cerr << "Failed to install hook: " << GetLastError() << std::endl;
//        }
//    }
//    else
//    {
//        std::cerr << "Failed to load the hook DLL." << std::endl;
//    }
//
//    // Main message loop:
//    MSG msg;
//    while (GetMessage(&msg, NULL, 0, 0))
//    {
//        TranslateMessage(&msg);
//        DispatchMessage(&msg);
//    }
//
//    return (int)msg.wParam;
//}
//
////  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
////
////  PURPOSE:  Processes messages for the main window.
////
////  WM_PAINT    - Paint the main window
////  WM_DESTROY  - post a quit message and return
//LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
//{
//    PAINTSTRUCT ps;
//    HDC hdc;
//    TCHAR greeting[] = _T("Hello, Windows desktop!");
//
//    switch (message)
//    {
//    case WM_PAINT:
//        hdc = BeginPaint(hWnd, &ps);
//
//        // Here your application is laid out.
//        // For this introduction, we just print out "Hello, Windows desktop!"
//        // in the top left corner.
//        TextOut(hdc,
//            5, 5,
//            greeting, _tcslen(greeting));
//        // End application-specific layout section.
//
//        EndPaint(hWnd, &ps);
//        break;
//    case WM_DESTROY:
//        PostQuitMessage(0);
//        break;
//    default:
//        return DefWindowProc(hWnd, message, wParam, lParam);
//        break;
//    }
//
//    return 0;
//}
