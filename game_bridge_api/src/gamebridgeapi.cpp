#include "gamebridgeapi.h"
#include <iostream>

#include "wmicommunication.h"

namespace game_bridge {
    // Todo put this somewhere else
    WMICommunication wmi("");

    GAME_BRIDGE_API void init_api()
    {
        std::cout << "Initializing events" << std::endl;
        wmi.initializeObjects("");
    }
    GAME_BRIDGE_API void subscribe_to_pocess_events()
    {

    }
    GAME_BRIDGE_API void unsubscribe_from_process_events()
    {

    }
    GAME_BRIDGE_API void set_process_event_callback()
    {

    }
    GAME_BRIDGE_API void inject_into_process()
    {

    }
}
