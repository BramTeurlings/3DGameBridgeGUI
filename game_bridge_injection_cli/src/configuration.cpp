#include "configuration.h"
#include <filesystem>
#include <iostream>

#include "file_functions.h"

namespace fs = std::filesystem;

/*
 * Todo Add exception and error handling
 * File io exceptions
 * Json exceptions,
 *      - If the json file contains unknown values it throws an exceptions
 */

namespace game_bridge {
    void InitializeConfiguration()
    {
        // Check if a config file exists
        if (!fs::exists(SRGB_CONFIGURATION_PATH))
        {
            // Create empty config file
            std::vector<GameConfiguration> single_empty_config;
            single_empty_config.push_back(GameConfiguration());
            SaveConfiguration(single_empty_config);
            std::cout << "Created empty config file" << "\n";
        }
    }

    void SaveConfiguration(const std::vector<GameConfiguration>& config)
    {
        // Convert vector to json format
        json json_vec(config);

        // Dump json string with indentation of 1 for pretty print and write to disk
        WriteTextToFile(SRGB_CONFIGURATION_PATH, json_vec.dump(1));
    }

    std::vector<GameConfiguration> LoadConfiguration()
    {
        // Read configuration file, parse json string and return deserialized object
        json p = json::parse(ReadTextFile(SRGB_CONFIGURATION_PATH));
        return p.get<std::vector<GameConfiguration>>();
    }
}
