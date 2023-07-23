#include "configuration.h"
#include <filesystem>
#include <iostream>

#include "file_functions.h"

namespace fs = std::filesystem;
constexpr char DATA_FOLDER[] = { "data" };
constexpr char SUPER_DEPTH_3D_FOLDER_NAME[] = { "SuperDepth3D" };
constexpr char GEO_11_FOLDER_NAME[] = { "Geo-11" };


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

	const char* FixTypeToString(const FixType fix_type)
	{
		switch (fix_type) {
		case SuperDepth: return SUPER_DEPTH_3D_FOLDER_NAME;
		case Geo11: return GEO_11_FOLDER_NAME;
        default: return "";
		}
	}

    std::string DetermineGameFixPath(const std::string& exe_name, const FixType fix_type)
    {
        fs::path fix(fs::current_path() /= DATA_FOLDER);
        fix.append(FixTypeToString(fix_type));
        if (fix_type == SuperDepth) {
            fix /= exe_name;
        }
		return fix.string();
    }
}
