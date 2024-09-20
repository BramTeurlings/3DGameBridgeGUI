#include "file_functions.h"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
namespace fs = std::filesystem;

namespace game_bridge {
    bool WriteTextToFile(const std::string& path, const std::string& buffer)
    {
        std::ofstream myfile(path);
        if (myfile.is_open())
        {
            myfile << buffer;
            myfile.close();
        }
        else {
            std::cout << "Unable to open file";
            return false;
        }

        return true;
    }

    std::string ReadTextFile(const std::string& path)
    {
        std::stringstream stream;
        std::string line;
        std::ifstream myfile(path);
        if (myfile.is_open())
        {
            while (getline(myfile, line))
            {
                stream << line;
            }
            myfile.close();
        }
        else {
            std::cout << "Unable to open file";
            return false;
        }

        return stream.str();
    }
}