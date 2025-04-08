#include "pico/stdlib.h"
#include <stdint.h>
#include <stdio.h>

#include "board_def.h"
#include "pid.h"
#include "states.h"
#include "debounce/button.h"
#include "motor/bidshot_motor.h"
#include "drv/drv824xs.h"
#include "led/ws2812.h"

void init();
void uprintf(const char* format, ...);
bool systemControlLoop(repeating_timer_t *rt);
bool motorControlLoop(repeating_timer_t *rt);
void updateWheelState(wheelState_t newState);
bool pusherSafetyCallback(repeating_timer_t *rt);

repeating_timer_t pusherSafetyCallbackTimer;
pusherSafetyTimeout_t psTimeout = NONE;

LED::WS2812 led = LED::WS2812(LED_DATA, pio1);

// create DRV8243 (pusher driver) object
DRV::DRV824xS drv = DRV::DRV824xS(DRV_EN, DRV_PH, DRV_NSLEEP, DRV_MOSI, DRV_MISO, DRV_NSCS, DRV_SCLK, spi0);

// instantiate esc objects
#define NUM_MOTORS 2
Motor::BIDSHOTMotor motors[NUM_MOTORS] {
};

// various switches on the blaster
Debounce::Button cycle = Debounce::Button(IO1, true, true);
Debounce::Button trig = Debounce::Button(IO2, true, true);
Debounce::Button sel1 = Debounce::Button(IO6, true, true);
Debounce::Button sel2 = Debounce::Button(IO5, true, true);
Debounce::Button rev = Debounce::Button(IO3, true, true);

// defining fire modes
struct firemode_t {
  uint32_t targetRPM[NUM_MOTORS];
  uint8_t numShots; // there is no way you need more than like 200
  uint8_t burstMode; //what happens when trigger is released
};

// stuctured as boot/firing mode 1, 2,3
uint32_t VariableFPS[3] = {14000, 24000, 45000};
uint8_t burstSize[3] = {1,3,100}; //maximum amount of darts fired per trigger pull
uint8_t burstMode[3] = {1, 1, 0}; //0 for trigger release ends burst, 1 for finish burst amount

uint32_t SET_RPM; // convenience bc for now i only want one fps setting
uint8_t shotsFired = 0; // so we know how far into a burst we are

struct firemode_t firemode_one = { {SET_RPM, SET_RPM}, 1, 1};
struct firemode_t firemode_two = { {SET_RPM, SET_RPM}, 3, 1};
struct firemode_t firemode_three = { {SET_RPM, SET_RPM}, 100, 0}; 
struct firemode_t* firemode_curr = &firemode_one; // pointer to current fire mode

// blaster state variables
wheelState_t wheelState; // this gets set in the init function so that the timestamp is in sync
pusherState_t pusherState = STOPPED;
absolute_time_t lastWheelStateUpdate;

// helper variables for PID control
// these pid values work for the most part. not the greatest, but better than nothing lol
PID mPID[NUM_MOTORS] {};
float pid_p = 0.00015;
float pid_i = 0.000001 / 4;
float pid_d = 0.0;
float throttlePoint[NUM_MOTORS]; // where requested throttle gets stored
float steadyThrottle[NUM_MOTORS]; // last known good throttle, used for ramp down
uint32_t rampDownTime = 500 * 1000; // motor ramp down time in us
int32_t pidFrequency = 2000; // update frequency in hz
int32_t loopTimeus = 1e6 / pidFrequency; // motor control loop time in us

// loop variables for main logic loop
int32_t mainLoopFrequency = 1000; // main logic loop update frequency in hz
int32_t mainLoopTimeus = 1e6 / mainLoopFrequency; // main logic loop time in us


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

   if (sel1.isPressed()) { // forward position
    SET_RPM = VariableFPS[0];
  }
  else if (sel2.isPressed()) { // backward position
    SET_RPM = VariableFPS[2];
  }
  else { // middle position
    SET_RPM = VariableFPS[1];
  }

  firemode_one = { {SET_RPM, SET_RPM}, burstSize[0], burstMode[0]};
  firemode_two = { {SET_RPM, SET_RPM}, burstSize[1], burstMode[1]};
  firemode_three = { {SET_RPM, SET_RPM}, burstSize[2], burstMode[2]};


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
  }
}

// this function runs on a timer, and sends commands to the motors
bool motorControlLoop(repeating_timer_t *rt) {
  uint8_t atTarget = 0; // track if each motor is up to speed, and assume they arent
  for (uint8_t i = 0; i < NUM_MOTORS; i++) {
    if (wheelState == SLOWING) {
      // keep updating pid in case we need to start accelerating again
      uint32_t rpm = motors[i].readTelemetry();
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
      uint32_t rpm = motors[i].readTelemetry();
      throttlePoint[i] = updatePID(&mPID[i], firemode_curr->targetRPM[i], rpm);
      if (rpm > firemode_curr->targetRPM[i] - 500) {
        atTarget |= 1 << i;
      } // set bit if around target rpm
    }
    else if (wheelState == STEADY) {
      // same as accelerating, just dont need to update atTarget
      uint32_t rpm = motors[i].readTelemetry();
      throttlePoint[i] = updatePID(&mPID[i], firemode_curr->targetRPM[i], rpm);
    }
    else { // wheelState == IDLE 
      throttlePoint[i] = 0.0;
    }
    
    // now that we've calculated the throttles, send them to the motors
    motors[i].setThrottle(throttlePoint[i]);
  }
  
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

    // cancel the pusher safety timer now that we've pressed the trigger
    if (psTimeout == WAITING) {
      cancel_repeating_timer(&pusherSafetyCallbackTimer);
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
      uprintf("INFO: Cycle switch pressed\r\n");
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

  return true;
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

// because printf() over usb blocks if a serial connection isnt active apparently
void uprintf(const char* format, ...) {
  if (!stdio_usb_connected()) {
      return;  // don't do anything if usb isn't connected
  }
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
}