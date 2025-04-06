#pragma once

#include <stdint.h>

namespace DRV {
    // used to drive a hardware variant of the DRV8243, DRV8244, DRV8245, or DRV8246
    class DRV824xH {
        protected:
            uint8_t ph;
            uint8_t en;
            uint8_t nsleep;
            uint8_t nfault;
            uint8_t drvoff;

        public:
            DRV824xH(uint8_t pin1, uint8_t pin2, uint8_t nsleep, uint8_t nfault, uint8_t drvoff);
            void init();
            bool wake();
            void sleep();
            void drive();
            void brake();
            void coast();
    };
}