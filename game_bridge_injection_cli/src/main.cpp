#include "file_functions.h"
#include "app.h"

#include <windows.h>
#include <stdlib.h>
#include <tchar.h>

#include <string.h>
#include <iostream>
#include <regex>

#include "process_injection.h"
#include "threads.h"

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

 // Global variables

 // The main window class name.
static TCHAR szWindowClass[] = _T("DesktopApp");

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("Windows Desktop Guided Tour Application");

// Stored instance handle for use in Win32 API calls such as FindResource
HINSTANCE hInst;

void MessageLoop() {
    // Start the message loop. 

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    TCHAR greeting[] = _T("Hello, Windows desktop!");

    switch (message)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);

        // Here your application is laid out.
        // For this introduction, we just print out "Hello, Windows desktop!"
        // in the top left corner.
        TextOut(hdc,
            5, 5,
            greeting, _tcslen(greeting));
        // End application-specific layout section.

        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}

bool InitWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow) {
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL,
            _T("Call to RegisterClassEx failed!"),
            _T("Windows Desktop Guided Tour"),
            NULL);

        return false;
    }

    // Store instance handle in our global variable
    hInst = hInstance;

    // The parameters to CreateWindowEx explained:
    // WS_EX_OVERLAPPEDWINDOW : An optional extended window style.
    // szWindowClass: the name of the application
    // szTitle: the text that appears in the title bar
    // WS_OVERLAPPEDWINDOW: the type of window to create
    // CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
    // 500, 100: initial size (width, length)
    // NULL: the parent of this window
    // NULL: this application does not have a menu bar
    // hInstance: the first parameter from WinMain
    // NULL: not used in this application
    HWND hWnd = CreateWindowEx(
        WS_EX_OVERLAPPEDWINDOW,
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        500, 100,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hWnd)
    {
        MessageBox(NULL,
            _T("Call to CreateWindow failed!"),
            _T("Windows Desktop Guided Tour"),
            NULL);

        return false;
    }

    // The parameters to ShowWindow explained:
    // hWnd: the value returned from CreateWindow
    // nCmdShow: the fourth parameter from WinMain
    ShowWindow(hWnd,
        nCmdShow);
    UpdateWindow(hWnd);

    return true;
}

using namespace std;
int main(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    //int count;
    //// Display each command-line argument.
    ////cout << "\nCommand-line arguments:\n";
    //for (count = 0; count < argc; count++) {
    //    cout << "  argv[" << count << "]   " << argv[count] << "\n";
    //}

    //bool numberLines = false;    // Default is no line numbers.

    //// If /n is passed to the .exe, display numbered listing
    //// of environment variables.
    //if ((argc == 2) && _stricmp(argv[1], "/n") == 0) {
    //    numberLines = true;
    //}
    
    std::regex path_regex("^Path");
    std::string path_environment_variable = std::getenv("PATH");
    //std::cout << path_environment_variable << "\n";

    std::regex path_search_regex("[a-zA-Z0-9+_\\-\\.:%()\\s\\\\]+");
    std::smatch match_results;
    std::regex_search(path_environment_variable, match_results, path_search_regex);

    auto words_begin = std::sregex_iterator(path_environment_variable.begin(), path_environment_variable.end(), path_search_regex);
    auto words_end = std::sregex_iterator();

    std::string simulated_reality_bin_path;
    for (std::sregex_iterator i = words_begin; i != words_end; ++i)
    {
        std::smatch match = *i;
        std::string match_str = match.str();
        std::cout << match_str << '\n';
        if(match_str.find("Simulated Reality") != std::string::npos)
        {
            simulated_reality_bin_path = match_str;
            break;
        }
    }

    game_bridge::GameBridgeInjectionCLI application;
    //application.RunAutomaticInjector(simulated_reality_bin_path);

    if (InitWindow(hInstance, hPrevInstance, lpszCmdLine, 1)) {

        application.RunMessageInterceptHooks(hInstance, simulated_reality_bin_path);
        MessageLoop();
    }
}
