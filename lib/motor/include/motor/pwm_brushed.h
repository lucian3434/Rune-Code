#include "motor.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include <stdio.h>

namespace Motor {
    class PWMBrushed: public Motor {
        private:
            uint pwmSlice;
            bool polarity;
            float cthrottle;
        public:
            PWMBrushed(uint8_t gpioPin) : Motor(gpioPin) {};
            void init(); // initialize motor
            void setThrottle(float throttle); // set throttle to a fractional value from 0 to 1 (inclusive)
            void setOutputPolarity(bool invert); // whether or not to invert the output duty cycle
    };
}