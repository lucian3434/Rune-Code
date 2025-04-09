#include "pid.h"

float updatePID(PID* pid, uint32_t targetRPM, uint32_t currentRPM) {
    pid->Isum += (float)(targetRPM) - (float)(currentRPM);
    float iError = pid->I * pid->Isum;
    
    // noise filtering for the D term is currently a basic rolling window, but a better algorithm is probably needed
    pid->lastRPM[pid->rpmIndex] = currentRPM;
    uint32_t avgRPM = 0;
    for (uint8_t i = 0; i < rpmCacheLength; i++) {
        avgRPM += pid->lastRPM[i];
    }
    avgRPM /= rpmCacheLength;
    
    float pError = pid->P * ((float)targetRPM - (float)avgRPM);
    float dError = pid->D * ((float)avgRPM - (float)pid->lastAvg);
    pid->lastAvg = avgRPM;
    pid->rpmIndex++;
    if (pid->rpmIndex == rpmCacheLength) pid->rpmIndex = 0;
    return pError + iError + dError;
}

void zeroPID(PID* pid) {
    for (uint8_t i = 0; i < rpmCacheLength; i++) {
        pid->lastRPM[i] = 0;
    }
    pid->rpmIndex = 0;
    pid->Isum = 0.0;
    pid->currThrottle = 0.0;
    pid->lastAvg = 0;
}

void initPID(PID* pid, float p, float i, float d) {
    pid->P = p;
    pid->I = i;
    pid->D = d;
    zeroPID(pid);
}