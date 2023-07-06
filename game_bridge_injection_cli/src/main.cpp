#include "file_functions.h"
#include "app.h"

#include <Windows.h>
#include <iostream>
#include <regex>
#include "process_injection.h"

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
int main(int argc, char* argv[])
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


    std::string game_name = "Journey.exe";

    
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
    InjectIntoApplication(GetPID(""), simulated_reality_bin_path);
}
