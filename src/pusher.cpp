#include "pusher.h"

#ifdef PUSHER_SCOTCH_YOKE
uint8_t shotsFired = 0;

void pusherTriggerRisingEdgeHook() {
  if (psTimeout == WAITING) {
    cancel_repeating_timer(&pusherSafetyCallbackTimer);
    psTimeout = NONE;
  }
  
  // start firing
  if ((firemode_curr->numShots > 0 && wheelState != ACCELERATING)) {
    updateWheelState(ACCELERATING);
    shotsFired = 0;
  }
}

void pusherTriggerFallingEdgeHook() {
  add_repeating_timer_ms(-200, pusherSafetyCallback, NULL, &pusherSafetyCallbackTimer);
  psTimeout = WAITING;
}

void pusherTick() {
    if (pusherState == RUNNING) {
        if (cycle.isRisingEdge()) {
          #ifndef USE_RPM_LOGGING
          uprintf("INFO: Cycle switch pressed\r\n");
          #endif
          shotsFired++;
          if ((shotsFired >= firemode_curr->numShots) || ((!trig.isPressed()) && (firemode_curr->burstMode == 0))) {
            // stop the pusher if we've finished the burst or let go of the trigger
            pusherState = STOPPED;
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

// check to make sure the pusher isnt stuck on for too long after the trigger is released (dead switch/cannot travel/etc)
bool pusherSafetyCallback(repeating_timer_t *rt) {
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
#endif