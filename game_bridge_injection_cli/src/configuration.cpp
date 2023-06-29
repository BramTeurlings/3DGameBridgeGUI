#include "configuration.h"

#include <filesystem>

#include "file_functions.h"
#include <iostream>

namespace fs = std::filesystem;

namespace game_bridge {
    void InitializeConfiguration()
    {
        if (!fs::exists(SRGB_CONFIGURATION_PATH))
        {
            std::vector<GameConfiguration> single_empty_config;
            single_empty_config.push_back(GameConfiguration());
            SaveConfiguration(single_empty_config);
        }
    }

    void SaveConfiguration(const std::vector<GameConfiguration>& config)
    {
        json json_vec(config);
        std::string data = json_vec.dump(1); // Dump with indentation of 1 for pretty print!
        WriteTextToFile(SRGB_CONFIGURATION_PATH, data);

        std::cout << "No configuration file found, created one!" << "\n";
    }

    std::vector<GameConfiguration> LoadConfiguration()
    {
        std::string read_data = ReadTextFile(SRGB_CONFIGURATION_PATH);

        json p = json::parse(read_data);
        return p.get<std::vector<GameConfiguration>>();
    }
}
