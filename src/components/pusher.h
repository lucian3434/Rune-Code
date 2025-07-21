#pragma once

#include "./../config.h"
#include "debounce/button.h"

// so we can pass the update wheel state function to the function
// this is a workaround and at some point i plan to reorganize the code base again to be entirely class based
// i was thinking about doing that in this branch, but this is taking a while and i said i would have solenoid code done this weekend
typedef void(*wheelUpdateCallback_t)(wheelState_t);

namespace Rune {
    // Prototype class for a pusher module. Not for direct use.
    class PusherGeneric {
        protected:
            wheelUpdateCallback_t updateWheelState;
            firemode_t **firemode;
            uint16_t shotsFired;
        public:
            enum pusherState_t {
                RUNNING,
                STOPPED
            };
            PusherGeneric();
            PusherGeneric(wheelUpdateCallback_t callback, firemode_t **firemode_curr);
            bool init();
            void triggerRisingEdge();
            void triggerFallingEdge();
            void pusherTick();
            void updatePusherState();
    };
}