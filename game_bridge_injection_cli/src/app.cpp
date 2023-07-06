#include "app.h"
#include <Windows.h>

#include "gamebridgeapi.h"
#include "process_injection.h"

namespace game_bridge {
    GameBridgeInjectionCLI::GameBridgeInjectionCLI()
    {
        // Load configuration
        InitializeConfiguration();
        supported_games = LoadConfiguration();

        // Initialize Game Bride Api
        game_bridge::init_api();
        game_bridge::subscribe_to_pocess_events();

        // Keep theapp running
        while (true) {
            Sleep(1000);
        }
    }
}
