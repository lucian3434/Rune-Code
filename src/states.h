#pragma once
#include <stdint.h>

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