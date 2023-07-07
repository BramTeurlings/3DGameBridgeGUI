#include "app.h"
#include <Windows.h>
#include <iostream>

#include "wmicommunication.h"
#include "gamebridgeapi.h"
#include "process_injection.h"
#include "gamebridgeapi.h"

namespace game_bridge {
    GameBridgeInjectionCLI::GameBridgeInjectionCLI()
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

        // Keep the app running
        while (true) {
            process_detection.pSink->semaphore_message_queue.wait();
            Win32ProcessData process_data = process_detection.pSink->message_queue.front();
            process_detection.pSink->message_queue.pop();
            std::cout << "pid: " << process_data.pid << " path: " << process_data.executable_path << "\n";
        }
    }
}
