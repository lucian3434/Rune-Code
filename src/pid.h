#pragma once
#include <stdint.h>

const uint8_t rpmCacheLength = 8;

struct PID {
    float P;
    float I;
    float D;
    uint32_t lastRPM[rpmCacheLength];
    uint8_t rpmIndex;
    uint32_t lastAvg;
    float Isum;
    float currThrottle;
};

// returns recommended throttle
float updatePID(PID* pid, uint32_t targetRPM, uint32_t currentRPM);

// resets counters to 0 (i.e. make sure motor stops)
void zeroPID(PID* pid);

// initialize a pid struct with the given constants
void initPID(PID* pid, float p, float i, float d);