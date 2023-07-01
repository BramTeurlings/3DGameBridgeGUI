#include "app.h"
#include "process_detection.h"

namespace game_bridge {
    GameBridgeInjectionCLI::GameBridgeInjectionCLI()
    {
        ProcessCreationListener listener;
        listener.StartListening();

        InitializeConfiguration();
        supported_games = LoadConfiguration();
    }
}
