#include "scotch_yoke_pusher.h"

Rune::PusherScotchYoke::PusherScotchYoke(wheelUpdateCallback_t callback, firemode_t *firemode_curr, DRV::DRV824xS *drv) {
    updatewheelState = callback;
    firemode = firemode_curr;
    shotsFired = 0;
    driver = drv;
}