#pragma once

#include <hardware/timer.h>
#include <stdint.h>

namespace Debounce {
    class Button {
        private:
        uint64_t timestamp; // times in microseconds
        static uint64_t timeout;
        uint8_t state; // whether the button is pressed or not
        uint8_t pin;
        bool inverted;
        bool hasPullup;

        public:
        Button(uint8_t gpioPin);
        Button(uint8_t gpioPin, bool invert, bool pullup);
        void init();
        void update();
        bool isPressed();
        bool isRisingEdge();
        bool isFallingEdge();
    };
}