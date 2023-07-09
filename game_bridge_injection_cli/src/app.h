#pragma once
#include "configuration.h"
#include <vector>

namespace game_bridge
{
    class GameBridgeInjectionCLI
    {
        std::vector<GameConfiguration> supported_games;

    public:
        GameBridgeInjectionCLI();
        void RunAutomaticInjector(std::string sr_binary_path);
    };
}
