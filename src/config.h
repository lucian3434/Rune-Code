#pragma once

#include "pico/stdlib.h"
#include <stdint.h>
#include <stdio.h>

#include "peripherals.h"
#include "states.h"

/* These defines alter program behaviour
 * Make changes here to suit the code to your blaster
 * To enable a setting, uncomment the line
 */

// when enabled, keeps track of spinup performance and prints the result to console in CSV format
//#define USE_RPM_LOGGING

// choose one of these to define which pusher type you are using
#define PUSHER_SCOTCH_YOKE
//#define PUSHER_BASIC_SOLENOID

// choose one of these to select which communication protocol to use with the ESCs
// note that for now this also requires the motors array to be updated to match in peripherals.cpp
// not that anything other than bidirectional dshot is supported atm lol
#define ESC_PROTOCOL_BIDSHOT


// defining fire modes
struct firemode_t {
  uint32_t targetRPM[NUM_MOTORS];
  uint8_t numShots; // there is no way you need more than like 200
  uint8_t burstMode; //what happens when trigger is released
};

extern struct firemode_t* firemode_curr; // pointer to current fire mode

// stuctured as boot/firing mode 1, 2,3
extern uint32_t variableFPS[3];
extern uint8_t burstSize[3]; //maximum number of darts fired per trigger pull
extern uint8_t burstMode[3]; //0 for trigger release ends burst, 1 for finish burst amount
