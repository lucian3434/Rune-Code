#pragma once

#include "motor.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"

namespace Motor {
    enum DSBitrate {
        DSHOT150  = 150000,
        DSHOT300  = 300000,
        DSHOT600  = 600000,
        DSHOT1200 = 1200000
    };

    class DSHOTMotor/*: public Motor*/ { // ill figure out the constructor stuff later
        private:
            uint8_t pin;
            PIO pioInstance;
            uint sm;
            DSBitrate bitrate;
            bool requestTelem;
            uint programOffset;

        public:
            DSHOTMotor(uint8_t gpioPin, PIO pio, DSBitrate dshotBitrate);
            void init(); // initialize motor
            void setThrottle(float throttle); // set throttle to a fractional value from 0 to 1 (inclusive)
            void requestTelemetry(); // request KISS telemetry after the next packet
    };
}