#pragma once

#include <stdint.h>

namespace Solenoid {
    class Solenoid {
        protected:
            uint8_t pin;
        public:
            Solenoid(uint8_t gpioPin);
            void init(); // assumes external pulldown
            void setOutput(bool state); // true to enable
    };
}