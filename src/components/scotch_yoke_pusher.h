#pragma once

#include "pusher.h"
#include "drv/drv824xs.h"
#include "pico/stdlib.h"
#include "./../config.h"
#include "./../util.h"

namespace Rune {
    class PusherScotchYoke : public PusherGeneric {
        protected:
            // state of the scotch yoke pusher safety timeout
            enum pusherSafetyTimeout_t {
                WAITING,
                NONE
            };
            DRV::DRV824xS *driver;
            Debounce::Button *cycle;
            Debounce::Button *trig;
            Rune::PusherGeneric::pusherState_t pusherState;
            absolute_time_t pusherStateTimer;
            uint32_t safetyTimeoutms = 200;
        public:
            PusherScotchYoke(wheelUpdateCallback_t callback, firemode_t **firemode_curr, DRV::DRV824xS *drv, Debounce::Button *cycleSwitch, Debounce::Button *trigger);
            bool init();
            void triggerRisingEdge();
            void triggerFallingEdge();
            void pusherTick();
            void updatePusherState(Rune::PusherGeneric::pusherState_t newState);
    };
}