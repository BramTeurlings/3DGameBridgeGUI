#pragma once
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#define SRGB_CONFIGURATION_PATH "./config_file.json"

namespace game_bridge {
    struct GameConfiguration {
        std::string exe_name;
        std::string unique_hash;
        std::string title;
        std::string path_to_exe;
        std::string selected3d_method;
        std::string injection_method;
        std::string last_updated_time_stamp;

        // This only works for structs/classes just used for serialization/deserialization
        // Generates to_json and from_json functions automatically
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(GameConfiguration, exe_name, unique_hash, title, path_to_exe, selected3d_method, injection_method, last_updated_time_stamp)
    };

    /**
     * \brief Intialize the configuration class. It will generate a config file if it doesn't exists next to the executable.
     */
    void InitializeConfiguration();
    /**
     * \brief Save all game configurations to disk in a single Json file next to the executable.
     * \param config Vector of game configurations
     */
    void SaveConfiguration(const std::vector<GameConfiguration>& config);
    /**
     * \brief Loads game configurations from the configuration file next to the executable.
     * \return Returns vector of game configurations
     */
    std::vector<GameConfiguration> LoadConfiguration();

}
