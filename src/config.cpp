#include "config.h"

struct firemode_t* firemode_curr = nullptr; // pointer to current fire mode

// stuctured as boot/firing mode 1, 2, 3
uint32_t variableFPS[3] = {19500, 24000, 45000}; // target rpm by position of selector switch at boot
uint8_t burstSize[3] = {1, 3, 100}; //maximum number of darts fired per trigger pull
uint8_t burstMode[3] = {0, 0, 0}; //0 for trigger release ends burst, 1 for finish burst amount