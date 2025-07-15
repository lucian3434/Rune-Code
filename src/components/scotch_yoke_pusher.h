#pragma once

#include "pusher.h"
#include "drv/drv824xs.h"

namespace Rune {
    class PusherScotchYoke : public PusherGeneric {
        private:
            DRV::DRV824xS *driver;
            enum pusherState_t {
                RUNNING,
                STOPPED
            };
            pusherState_t pusherState;

        public:
            PusherScotchYoke(wheelUpdateCallback_t callback, firemode_t *firemode_curr, DRV::DRV824xS *drv);
    };
}