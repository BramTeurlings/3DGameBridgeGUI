#pragma once
#include "gamebridgeapi.h"
#include <Windows.h>

int main() {
    // test
    game_bridge::init_api();
    game_bridge::subscribe_to_pocess_events();

    while (1) {
        Sleep(1000);
    }
}
