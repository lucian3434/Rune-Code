#include "motor.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include <stdio.h>

namespace Motor {
    class PWMMotor: public Motor {
        private:
            uint pwmSlice;
        public:
            PWMMotor(uint8_t gpioPin) : Motor(gpioPin) {};
            void init(); // initialize motor
            void setThrottle(float throttle); // set throttle to a fractional value from 0 to 1 (inclusive)
    };
}