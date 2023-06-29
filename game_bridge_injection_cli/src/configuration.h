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
        std::string selected3_d_method;
        std::string injection_method;
        std::string last_updated_time_stamp;

        // This only works for structs/classes just used for serialization/deserialization
        // Creates to_json and from_json functions
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(GameConfiguration, exe_name, unique_hash, title, path_to_exe, selected3_d_method, injection_method, last_updated_time_stamp)
    };

    void InitializeConfiguration();
    void SaveConfiguration(const std::vector<GameConfiguration>& config);
    std::vector<GameConfiguration> LoadConfiguration();

}
