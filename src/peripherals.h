#pragma once

#include "board_def.h"
#include "debounce/button.h"
#include "motor/bidshot_motor.h"
#include "drv/drv824xs.h"
#include "led/ws2812.h"

// define our ESCS and how we communicate with them
#define NUM_MOTORS 2
extern Motor::BIDSHOTMotor motors[NUM_MOTORS];

// configuring IO on the blaster
extern Debounce::Button rev;
// below IO is referenced by name by the fire control logic
extern Debounce::Button cycle;
extern Debounce::Button trig;
extern Debounce::Button sel1;
extern Debounce::Button sel2;

extern LED::WS2812 led;

// create DRV8244 (pusher driver) object
extern DRV::DRV824xS drv;