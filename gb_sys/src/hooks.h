#pragma once

#include <Windows.h>
#include <Psapi.h>
#include <iostream>

#ifdef GAME_BRIDGE_API_EXPORTS
#define GAME_BRIDGE_API __declspec(dllexport)
#else
#define GAME_BRIDGE_API __declspec(dllimport)
#endif

namespace game_bridge {
    extern "C" GAME_BRIDGE_API void init_api();

    // Subsribe to process events
    extern "C" GAME_BRIDGE_API void subscribe_to_pocess_events();

    extern "C" GAME_BRIDGE_API void unsubscribe_from_process_events();

    extern "C" GAME_BRIDGE_API void set_process_event_callback();

    extern "C" GAME_BRIDGE_API void inject_into_process();
}

extern "C" GAME_BRIDGE_API void InstallHook();
extern "C" GAME_BRIDGE_API void UninstallHook();
extern "C" GAME_BRIDGE_API LRESULT CALLBACK HookCallback(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam);
