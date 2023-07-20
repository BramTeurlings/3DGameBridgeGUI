#pragma once
#include "configuration.h"
#include <vector>
#include <Windows.h>

namespace game_bridge
{
    class GameBridgeInjectionCLI
    {
        std::vector<GameConfiguration> supported_games;

    public:
        GameBridgeInjectionCLI();
        void RunAutomaticInjector(std::string sr_binary_path);
        void RunMessageInterceptHooks(HINSTANCE hInstance, std::string sr_binary_path);
    };
}
