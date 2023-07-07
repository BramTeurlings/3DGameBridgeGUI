#include "app.h"
#include <Windows.h>
#include <iostream>
#include <filesystem>

#include "wmicommunication.h"
#include "gamebridgeapi.h"
#include "process_injection.h"
#include "gamebridgeapi.h"

namespace fs = std::filesystem;

namespace game_bridge {
    GameBridgeInjectionCLI::GameBridgeInjectionCLI()
    {

	}

    void GameBridgeInjectionCLI::RunAutomaticInjector(std::string sr_binary_path)
    {
        // Load configuration
        InitializeConfiguration();
        supported_games = LoadConfiguration();

        // Initialize Game Bride Api
        game_bridge::init_api();
        game_bridge::subscribe_to_pocess_events();

        // Init WMI here for now
        WMICommunication process_detection("");
        process_detection.initializeObjects("");

        std::vector<std::string> executable_names;
        for (GameConfiguration config : supported_games) {
            executable_names.push_back(config.exe_name);
        }

        loading_data payload_64_bit;
        CreatePayload(sr_binary_path, payload_64_bit);

        // Keep the app running
        while (true) {
            process_detection.pSink->semaphore_message_queue.wait();
            Win32ProcessData process_data = process_detection.pSink->message_queue.front();
            process_detection.pSink->message_queue.pop();
            std::cout << "pid: " << process_data.pid << " path: " << process_data.executable_path << "\n";

            fs::path detected_exe(process_data.executable_path);
            std::string filename = detected_exe.filename().string();

            std::for_each(executable_names.begin(), executable_names.end(), [&](std::string a) {
                if (filename.compare(a) == 0) {
                    std::cout << "Found supported game, start injection" << "\n";
                    InjectIntoApplication(process_data.pid, payload_64_bit);
                }
                });
        }
    }
}
