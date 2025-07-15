#pragma once
#include "pico/stdlib.h"

enum wheelState_t {
    IDLE,
    ACCELERATING,
    STEADY,
    SLOWING
};

enum pusherState_t {
    RUNNING,
    STOPPED
};

// state of the scotch yoke pusher safety timeout
enum pusherSafetyTimeout_t {
    WAITING,
    NONE
};

extern wheelState_t wheelState;
extern pusherState_t pusherState;
extern absolute_time_t lastWheelStateUpdate;