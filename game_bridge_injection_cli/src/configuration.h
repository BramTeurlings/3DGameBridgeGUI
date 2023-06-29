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
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(GameConfiguration, exe_name, unique_hash, title, path_to_exe, selected3_d_method, injection_method, last_updated_time_stamp)
    };

    void InitializeConfiguration();
    void SaveConfiguration(const std::vector<GameConfiguration>& config);
    std::vector<GameConfiguration> LoadConfiguration();

    //void to_json(json& j, const GameConfiguration& config)
    //{
    //    j = json{
    //        {"exe_name", config.exe_name},
    //        {"unique_hash", config.unique_hash},
    //        {"title", config.title},
    //        {"path_to_exe", config.path_to_exe},
    //        {"selected3_d_method", config.exe_name},
    //        {"injection_method", config.exe_name},
    //        {"last_updated_time_stamp", config.last_updated_time_stamp},
    //    };
    //}

    //void from_json(const json& j, GameConfiguration& config)
    //{
    //    j.at("exe_name").get_to(config.exe_name);
    //    j.at("unique_hash").get_to(config.unique_hash);
    //    j.at("title").get_to(config.title);
    //    j.at("path_to_exe").get_to(config.path_to_exe);
    //    j.at("selected3_d_method").get_to(config.selected3_d_method);
    //    j.at("injection_method").get_to(config.injection_method);
    //    j.at("last_updated_time_stamp").get_to(config.last_updated_time_stamp);
    //}
}
