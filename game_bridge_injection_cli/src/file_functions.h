#pragma once
#include <string>

namespace game_bridge
{
    bool WriteTextToFile(const std::string& path, const std::string& buffer);
    std::string ReadTextFile(const std::string& path);
}
