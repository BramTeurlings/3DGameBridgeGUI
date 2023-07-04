#include "app.h"
#include "gamebridgeapi.h"
#include <Windows.h>

namespace game_bridge {
    GameBridgeInjectionCLI::GameBridgeInjectionCLI()
    {
        InitializeConfiguration();
        supported_games = LoadConfiguration();

        game_bridge::init_api();
        game_bridge::subscribe_to_pocess_events();

        while (true) {
            Sleep(1000);
        }
    }
}
