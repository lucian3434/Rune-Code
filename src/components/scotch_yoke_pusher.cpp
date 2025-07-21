#include "scotch_yoke_pusher.h"

Rune::PusherScotchYoke::PusherScotchYoke(wheelUpdateCallback_t callback, firemode_t **firemode_curr, DRV::DRV824xS *drv, Debounce::Button *cycleSwitch, Debounce::Button *trigger) {
  updateWheelState = callback;
  firemode = firemode_curr;
  driver = drv;
  cycle = cycleSwitch;
  trig = trigger;
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
}

void Rune::PusherScotchYoke::triggerFallingEdge() {
}

void Rune::PusherScotchYoke::pusherTick() {
  // if the pusher is running, check to see if it hit the cycle switch before continuing
  if (pusherState == RUNNING) {
    if (cycle->isRisingEdge()) {
      pusherStateTimer = make_timeout_time_ms(safetyTimeoutms);
      #ifndef USE_RPM_LOGGING
      uprintf("INFO: Cycle switch pressed\r\n");
      #endif
      shotsFired++;
      if ((shotsFired >= (*firemode)->numShots) || ((!trig->isPressed()) && ((*firemode)->burstMode == 0))) {
        // stop the pusher if we've finished the burst or let go of the trigger
        updatePusherState(STOPPED);
        updateWheelState(SLOWING); // and tell the wheels to slow down too
      }
    }
    else {
      // check to see if the pusher is still running after the safety timeout has expired
      if ((pusherState == RUNNING) && time_reached(pusherStateTimer)) {
        updatePusherState(STOPPED);
        updateWheelState(SLOWING);
        // give a warning so anything listening to serial knows whats happening
        uprintf("WARNING: Pusher safety timeout triggered. Check your pusher and cycle switch.\r\n");
      }
      else {
        driver->drive();
      }
    }
  }
  // brake if the pusher isn't supposed to be moving anymore
  if (pusherState == STOPPED) {
    driver->brake();
  }
}

void Rune::PusherScotchYoke::updatePusherState(Rune::PusherScotchYoke::pusherState_t newState) {
  pusherState = newState;
  if (newState == RUNNING) {
    shotsFired = 0;
    pusherStateTimer = make_timeout_time_ms(safetyTimeoutms);
  }
}
