#include "pid.h"

float updatePID(PID* pid, uint32_t targetRPM, uint32_t currentRPM) {
    pid->Isum += (float)(targetRPM) - (float)(currentRPM);
    float pError = pid->P * ((float)targetRPM - (float)currentRPM);
    float iError = pid->I * pid->Isum;
    float dError = pid->D * ((float)currentRPM - (float)pid->lastRPM[0]);
    pid->lastRPM[0] = currentRPM;
    return pError + iError + dError;
}

void zeroPID(PID* pid) {
    for (uint8_t i = 0; i < rpmCacheLength; i++) {
        pid->lastRPM[i] = 0;
    }
    pid->rpmIndex = 0;
    pid->Isum = 0.0;
    pid->currThrottle = 0.0;
}

void initPID(PID* pid, float p, float i, float d) {
    pid->P = p;
    pid->I = i;
    pid->D = d;
    zeroPID(pid);
}