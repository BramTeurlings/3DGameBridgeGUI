#pragma once
#include "gamebridgeapi.h"
#include <Windows.h>

int main() {
    // test
    gamebridge::init_api();
    gamebridge::subscribe_to_pocess_events();

    while (1) {
        Sleep(1000);
    }
}
