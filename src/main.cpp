#include "pico/stdlib.h"
#include <stdint.h>
#include <stdio.h>

#include "config.h"
#include "util.h"

#include "pid.h"
#include "states.h"

#include "components/scotch_yoke_pusher.h"



void init();
bool systemControlLoop(repeating_timer_t *rt);
bool motorControlLoop(repeating_timer_t *rt);
void updateWheelState(wheelState_t newState);
bool pusherSafetyCallback(repeating_timer_t *rt);

repeating_timer_t pusherSafetyCallbackTimer;
pusherSafetyTimeout_t psTimeout = NONE;

uint8_t shotsFired = 0; // helper variable so we know how far into a burst we are

// blaster state variables
wheelState_t wheelState; // this gets set in the init function so that the timestamp is in sync
pusherState_t pusherState = STOPPED;
absolute_time_t lastWheelStateUpdate;

// helper variables for PID control
// these pid values work for the most part. not the greatest, but better than nothing lol
PID mPID[NUM_MOTORS] {};
float pid_p = 0.00025;
float pid_i = 0.0000001;
float pid_d = -0.001;
float throttlePoint[NUM_MOTORS]; // where requested throttle gets stored
float steadyThrottle[NUM_MOTORS]; // last known good throttle, used for ramp down
uint32_t rampDownTime = 500 * 1000; // motor ramp down time in us
int32_t pidFrequency = 4000; // update frequency in hz
int32_t loopTimeus = 1e6 / pidFrequency; // motor control loop time in us

// loop variables for main logic loop
int32_t mainLoopFrequency = 1000; // main logic loop update frequency in hz
int32_t mainLoopTimeus = 1e6 / mainLoopFrequency; // main logic loop time in us

// rpm logging
#ifdef USE_RPM_LOGGING
const uint32_t rpmLogLength = 2000;
uint32_t rpmCache[rpmLogLength][NUM_MOTORS] = {0};
uint16_t throttleCache[rpmLogLength][NUM_MOTORS] = {0}; // float gets converted to an integer [0, 1999]
uint16_t cacheIndex = rpmLogLength + 1;
#endif

// fire mode storage
struct firemode_t firemode_one;
struct firemode_t firemode_two;
struct firemode_t firemode_three;

void init() {
  // initialize generic io and usb
  stdio_init_all();

  // initialize buttons
  trig.init();
  cycle.init();
  sel1.init();
  sel2.init();

  // initialize status LED
  led.init();
  led.setColor(0x0000FF); // blue to signal that we're booting
  led.update();

  // initialize motors
  for (uint8_t i = 0; i < NUM_MOTORS; i++) {
    motors[i].init();
    initPID(&mPID[i], pid_p, pid_i, pid_d);
  }

  uprintf("Initializing DRV...\r\n");
  // set up the pusher driver
  drv.init();

  // initialize and turn on ESC power switch
  gpio_init(ESC_ENABLE);
  gpio_set_dir(ESC_ENABLE, GPIO_OUT);
  gpio_put(ESC_ENABLE, true);
  
  for (uint8_t i = 0; i <NUM_MOTORS; i++){
    motors[i].setThrottle(0.0);
  }

  // set initial value for wheel state
  updateWheelState(IDLE);

  // update fire mode from selector
  sel1.update();
  sel2.update();

  uint32_t setRPM;
  if (sel1.isPressed()) { // forward position
    setRPM = variableFPS[0];
  }
  else if (sel2.isPressed()) { // backward position
    setRPM = variableFPS[2];
  }
  else { // middle position
    setRPM = variableFPS[1];
  }

  firemode_one = { {setRPM, setRPM}, burstSize[0], burstMode[0] };
  firemode_two = { {setRPM, setRPM}, burstSize[1], burstMode[1] };
  firemode_three = { {setRPM, setRPM}, burstSize[2], burstMode[2] };


}

int main() {
  // call init function
  init();
  uint8_t bootStatus = 0;
  
  // the drv8243 WILL wake up
  uprintf("Waking DRV...\r\n");
  while (!drv.wake()) {
    uprintf("Failed to wake DRV. Retrying...\r\n");
    sleep_ms(100);
  }
  uprintf("Successfully woke DRV!\r\n");
  bootStatus |= 0x1;

  // register main logic loop
  repeating_timer_t mainLogicLoopTimer;
  bool logicLoopAdded = add_repeating_timer_us(-mainLoopTimeus, systemControlLoop, NULL, &mainLogicLoopTimer);
  if (logicLoopAdded) {
    uprintf("Main logic loop registered!\r\n");
    bootStatus |= 0x2;
  }
  else {
    uprintf("Failed to register main logic loop\r\n");
  }

  // register motor control loop function to run at the specified pid frequency
  repeating_timer_t motorControlLoopTimer;
  bool timerAdded = add_repeating_timer_us(-loopTimeus, motorControlLoop, NULL, &motorControlLoopTimer);
  if (timerAdded) {
    uprintf("PID loop registered!\r\n");
    bootStatus |= 0x4;
  }
  else {
    uprintf("Failed to register PID loop\r\n");
  }

  if (bootStatus == 0x7) {
    led.setColor(0x3d2700); // yellowish, not too bright
  }
  else {
    led.setColor(0xFF0000); // red so we know somethings fucked up
  }
  led.update();

  // keep execution going
  while (true) {
    tight_loop_contents();
    #ifdef USE_RPM_LOGGING
    // dump cache once full
    if (cacheIndex == rpmLogLength) {
      uprintf("Motor 1 RPM, Motor 2 RPM, Motor 1 Throttle, Motor 2 Throttle, Setpoint\r\n");
      for (uint16_t i = 0; i < rpmLogLength; i++) {
        uprintf("%u, %u, %u, %u, %u\r\n", rpmCache[i][0], rpmCache[i][1], throttleCache[i][0], throttleCache[i][1], SET_RPM);
      }
      cacheIndex++; // increment index again so we don't print this more than once
    }
    sleep_ms(10);
    #endif
  }
}

// this function runs on a timer, and sends commands to the motors
bool motorControlLoop(repeating_timer_t *rt) {
  uint8_t atTarget = 0; // track if each motor is up to speed, and assume they arent
  for (uint8_t i = 0; i < NUM_MOTORS; i++) {
    // get rpm from motor
    uint32_t rpm = motors[i].readTelemetry();

    if (wheelState == SLOWING) {
      // keep updating pid in case we need to start accelerating again
      int32_t rpmOffset = (int32_t)((firemode_curr->targetRPM[i] / (float)rampDownTime) * absolute_time_diff_us(lastWheelStateUpdate, get_absolute_time()));
      uint32_t targetRPM;
      if (rpmOffset > firemode_curr->targetRPM[i]) {
        targetRPM = 0;
      }
      else {
        targetRPM = firemode_curr->targetRPM[i] - rpmOffset;
      }
      throttlePoint[i] = updatePID(&mPID[i], targetRPM, rpm);
    }
    else if (wheelState == ACCELERATING) {
      throttlePoint[i] = updatePID(&mPID[i], firemode_curr->targetRPM[i], rpm);
      if (rpm > firemode_curr->targetRPM[i] - 500) {
        atTarget |= 1 << i;
      } // set bit if around target rpm
    }
    else if (wheelState == STEADY) {
      // same as accelerating, just dont need to update atTarget
      throttlePoint[i] = updatePID(&mPID[i], firemode_curr->targetRPM[i], rpm);
    }
    else { // wheelState == IDLE 
      throttlePoint[i] = 0.0;
    }

    // if we have rpm logging enabled, add the most recent value to the cache
    #ifdef USE_RPM_LOGGING
    if (cacheIndex < rpmLogLength) {
      rpmCache[cacheIndex][i] = rpm;
      if (throttlePoint[i] > 1.0) throttlePoint[i] = 1.0; // normally this gets dealt with in the motor library, but i want to show it capped when logged
      throttleCache[cacheIndex][i] = (uint16_t)(throttlePoint[i] * 1999);
    }
    #endif
    
    // now that we've calculated the throttles, send them to the motors
    motors[i].setThrottle(throttlePoint[i]);
  }

  #ifdef USE_RPM_LOGGING
  // increment cache index if we still need to take data
  if (cacheIndex < rpmLogLength) cacheIndex++;
  #endif
  
  // now for more general checks
  if (wheelState == ACCELERATING) {
    // if all the motors are up to speed, set state variable accordingly
    // note that overshoot isn't checked; tune your PID properly!
    if (atTarget == ((1 << NUM_MOTORS) - 1)) updateWheelState(STEADY);
    // or, if its been more than 200ms, assume something is wrong and switch state anyway
    else if (absolute_time_diff_us(lastWheelStateUpdate, get_absolute_time()) > 200000) updateWheelState(STEADY);
  }
  else if (wheelState == SLOWING) {
    // switch state to idle if we have been slowing down for longer than rampDownTime; throttle should be 0 by now
    if (absolute_time_diff_us(lastWheelStateUpdate, get_absolute_time()) >= rampDownTime) updateWheelState(IDLE);
  }

  return true;
}

// gracefully handle changes between states
void updateWheelState(wheelState_t newState) {
  if (newState == IDLE) {
    // reset pid numbers when returning to idle
    for (uint8_t i = 0; i < NUM_MOTORS; i++) {
      zeroPID(&mPID[i]);
    }
  }
  else if (newState == STEADY) {
    uprintf("INFO: trigger delay: %ums\r\n", to_ms_since_boot(get_absolute_time()) - to_ms_since_boot(lastWheelStateUpdate));
    pusherState = RUNNING; // start the pusher once we are at steady state
  }
  else if (newState == SLOWING) {
    // save the last throttle values so we know where to slow down from
    for (uint8_t i = 0; i < NUM_MOTORS; i++) {
      steadyThrottle[i] = throttlePoint[i]; // TODO: write better code for this
    }
  }

  // set the new state once everything has been handled
  lastWheelStateUpdate = get_absolute_time();
  wheelState = newState;
}

// general control over the system. reads input from switches and runs control logic
bool systemControlLoop(repeating_timer_t *rt) {
  // update button states
  trig.update();
  cycle.update();
  sel1.update();
  sel2.update();

  // update fire mode from selector
  if (sel1.isPressed()) { // forward position
    firemode_curr = &firemode_one;
  }
  else if (sel2.isPressed()) { // backward position
    firemode_curr = &firemode_three;
  }
  else { // middle position
    firemode_curr = &firemode_two;
  }

  // start the firing sequence if the trigger was just pressed
  if (trig.isRisingEdge()) {
    uprintf("INFO: Trigger pressed\r\n");

    #ifdef USE_RPM_LOGGING
    cacheIndex = 0; // reset cache index to start logging
    #endif

    // cancel the pusher safety timer now that we've pressed the trigger
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

  // when the trigger is released, start the pusher safety timeout
  if (trig.isFallingEdge()) {
    add_repeating_timer_ms(-200, pusherSafetyCallback, NULL, &pusherSafetyCallbackTimer);
    psTimeout = WAITING;
  }

  // if the pusher is running, check to see if it hit the cycle switch before continuing
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

  return true; // repeat timer
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
