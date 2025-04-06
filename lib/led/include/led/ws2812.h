#pragma once

#include <stdint.h>
#include <hardware/pio.h>

#define BITRATE 800000.0 // 800kbps for high speed mode

namespace LED {
    class WS2812 {
        /*
         * Rune implementation uses 1 ws2812 led. default seems to be the 800kbps high speed mode
         * 
         * This library is a slightly modified version of one I wrote for another board, so
         * not all functionality in it (i.e. the ability to have a string of these) is actually
         * necessary. Of course, you could attach a string of LEDs to one of the io pins, but
         * as-is the code isn't meant to drive too many of these LEDs. The original board only
         * had two, and iirc this library should act weirdly once you have more than 8 LEDs on
         * one pin. If there is demand I might fix that, but until then this works perfectly
         * fine.
         */
        private:
            WS2812* prev; // towards gpio 
            WS2812* next; // away from gpio
            uint32_t colorState; // 0xGGRRBB
            uint8_t pin;
            PIO pioInstance;
            uint sm;

        public:
            WS2812(uint8_t gpioPin, PIO pio);
            WS2812(WS2812* parent);
            void init();
            void update();
            void setColor(uint32_t color); // 0xRRGGBB
            uint32_t getColor();
    };
}