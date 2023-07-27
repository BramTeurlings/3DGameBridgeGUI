#pragma once

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace game_bridge
{
    bool WriteTextToFile(const std::string& path, const std::string& buffer);
    std::string ReadTextFile(const std::string& path);
    bool CopyFile(const fs::path& source, const fs::path& destination);
}
