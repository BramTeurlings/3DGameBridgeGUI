#include "app.h"

namespace game_bridge {
    GameBridgeInjectionCLI::GameBridgeInjectionCLI()
    {
        InitializeConfiguration();
        supported_games = LoadConfiguration();

        //FreeLibrary(hModule);
    }
}
