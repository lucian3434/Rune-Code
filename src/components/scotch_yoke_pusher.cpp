#include "scotch_yoke_pusher.h"

Rune::PusherScotchYoke::PusherScotchYoke(wheelUpdateCallback_t callback, firemode_t **firemode_curr, DRV::DRV824xS *drv, Debounce::Button *cycleSwitch) {
  updateWheelState = callback;
  firemode = firemode_curr;
  driver = drv;
  cycle = cycleSwitch;
  psTimeout = NONE;
  pusherState = STOPPED;
}

// returns true if the pusher module was successfully initialized
bool Rune::PusherScotchYoke::init() {
  driver->init();
  cycle->init();
  return true;
}

// handler code for when the trigger is pressed
void Rune::PusherScotchYoke::triggerRisingEdge() {
  // cancel the pusher safety timer now that we've pressed the trigger
  if (psTimeout == WAITING) {
    cancel_repeating_timer(&pusherSafetyCallbackTimer);
    psTimeout = NONE;
  }
}

void Rune::PusherScotchYoke::triggerFallingEdge() {
  add_repeating_timer_ms(-200, pusherSafetyCallback, NULL, &pusherSafetyCallbackTimer);
  psTimeout = WAITING;
}

void Rune::PusherScotchYoke::pusherTick() {
  // if the pusher is running, check to see if it hit the cycle switch before continuing
  if (pusherState == RUNNING) {
    if (cycle->isRisingEdge()) {
      #ifndef USE_RPM_LOGGING
      uprintf("INFO: Cycle switch pressed\r\n");
      #endif
      shotsFired++;
      if ((shotsFired >= firemode_curr->numShots) || ((!trig.isPressed()) && (firemode_curr->burstMode == 0))) {
        // stop the pusher if we've finished the burst or let go of the trigger
        updatePusherState(STOPPED);
        updateWheelState(SLOWING); // and tell the wheels to slow down too
      }
    }
    else {
      drv.drive();
    }
  }
  // brake if the pusher isn't supposed to be moving anymore
  if (pusherState == STOPPED) {
    drv.brake();
  }
}

void Rune::PusherScotchYoke::updatePusherState(Rune::PusherScotchYoke::pusherState_t newState) {
  pusherState = newState;
  if (newState == RUNNING) {
    shotsFired = 0;
  }
}

// check to make sure the pusher isnt stuck on for too long after the trigger is released (dead switch/cannot travel/etc)
bool Rune::PusherScotchYoke::pusherSafetyCallback(repeating_timer_t *rt) {
  // check if the pusher is still running
  if (pusherState == RUNNING) {
    pusherState = STOPPED;
    updateWheelState(SLOWING);
    // give a warning so anything listening to serial knows whats happening
    uprintf("WARNING: Pusher safety timeout triggered. Check your pusher and cycle switch.\r\n");
  }
  psTimeout = NONE; // signal that the timeout has fired
  return false; // do not repeat
}