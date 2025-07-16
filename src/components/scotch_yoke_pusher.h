#pragma once

#include "pusher.h"
#include "drv/drv824xs.h"
#include "pico/stdlib.h"
#include "./../config.h"
#include "./../util.h"

namespace Rune {
    class PusherScotchYoke : public PusherGeneric {
        public: // this looks stupid here and theres probably a better way to do it
            enum pusherState_t {
                RUNNING,
                STOPPED
            };
        protected:
            // state of the scotch yoke pusher safety timeout
            enum pusherSafetyTimeout_t {
                WAITING,
                NONE
            };
            DRV::DRV824xS *driver;
            Debounce::Button *cycle;
            Rune::PusherScotchYoke::pusherState_t pusherState;
            Rune::PusherScotchYoke::pusherSafetyTimeout_t psTimeout;
            repeating_timer_t pusherSafetyCallbackTimer;

        public:
            PusherScotchYoke(wheelUpdateCallback_t callback, firemode_t **firemode_curr, DRV::DRV824xS *drv, Debounce::Button *cycleSwitch);
            bool init();
            void triggerRisingEdge();
            void triggerFallingEdge();
            void pusherTick();
            void updatePusherState(Rune::PusherScotchYoke::pusherState_t newState);
            bool pusherSafetyCallback(repeating_timer_t *rt);
    };
}