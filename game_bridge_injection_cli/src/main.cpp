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
