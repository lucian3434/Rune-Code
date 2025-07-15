#pragma once

#include "./../config.h"
#include "debounce/button.h"

// so we can pass the update wheel state function to the function
// this is a workaround and at some point i plan to reorganize the code base again to be entirely class based
// i was thinking about doing that in this branch, but this is taking a while and i said i would have solenoid code done this weekend
typedef void(*wheelUpdateCallback_t)(wheelState_t);

// pusher logic for when the trigger is pulled
void pusherTriggerRisingEdgeHook(wheelUpdateCallback_t updateWheelState);

// pusher logic for when the trigger is released
void pusherTriggerFallingEdgeHook(wheelUpdateCallback_t updateWheelState);

// called every tick of the system control loop
void pusherTick(wheelUpdateCallback_t updateWheelState);

#ifdef PUSHER_SCOTCH_YOKE
// check to make sure the pusher isnt stuck on for too long after the trigger is released (dead switch/cannot travel/etc)
bool pusherSafetyCallback(repeating_timer_t *rt);
#endif

namespace Rune {
    // Prototype class for a pusher module. Not for direct use.
    class PusherGeneric {
        private:
            wheelUpdateCallback_t updatewheelState;
            firemode_t *firemode;
            uint16_t shotsFired;
        public:
            PusherGeneric(wheelUpdateCallback_t callback, firemode_t *firemode_curr);
            bool init();
            void triggerRisingEdge();
            void triggerFallingEdge();
            void pusherTick();
            void updatePusherState();
    };
}