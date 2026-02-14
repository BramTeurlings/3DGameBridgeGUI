#pragma once

#include <iostream>
#include <windows.h>

namespace game_bridge
{
    LRESULT CALLBACK HookCallback(int nCode, WPARAM wParam, LPARAM lParam);

    class ProcessCreationListener
    {

    public:
        ProcessCreationListener();
        void StartListening();
    };
}