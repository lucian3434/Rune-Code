#pragma once

#include <stdint.h>
#include <stdio.h>

namespace Motor {
    class Motor {
        protected:
            uint8_t pin;
        public:
            Motor(uint8_t gpioPin);
            virtual void init(); // initialize motor
            virtual void setThrottle(float throttle); // set throttle to a fractional value from 0 to 1 (inclusive)
    };
}