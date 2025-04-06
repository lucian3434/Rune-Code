#include "led/ws2812.h"
#include "ws2812.pio.h"

LED::WS2812::WS2812(uint8_t gpioPin, PIO pio) {
    prev = nullptr;
    next = nullptr;
    colorState = 0x00000000;
    pin = gpioPin;
    pioInstance = pio;
}

LED::WS2812::WS2812(LED::WS2812* parent) {
    prev = parent;
    parent->next = this;
    next = nullptr;
    colorState = 0x00000000;
}

void LED::WS2812::init() {
    if (prev) { // initialize from first LED in chain
        prev->init();
    }
    else { 
        uint offset = pio_add_program(pioInstance, &ws2812_program);
        sm = pio_claim_unused_sm(pioInstance, true);
        ws2812_program_init(pioInstance, sm, offset, pin, BITRATE);
    }
}

void LED::WS2812::update() {
    if (prev) { // begin update from first LED
        prev->update();
    }
    else {
        // send data to PIO
        if (pio_sm_is_tx_fifo_empty(pioInstance, sm)) { // interface is empty
            LED::WS2812* currLED = this;
            while (currLED) {
                pio_sm_put(pioInstance, sm, currLED->colorState << 8u);
                currLED = currLED->next;
            }
        }
    }
}

// color should be of the format 0xRRGGBB
void LED::WS2812::setColor(uint32_t color) {
    // ws2812 wants GRB color, but i want people to pass RGB color to this function
    // this abomination swaps R and G while keeping B in place
    colorState = ((color & 0xff00) << 8) | ((color & 0xff0000) >> 8) | (color & 0xff);
}

// returns 0xRRGGBB
uint32_t LED::WS2812::getColor() {
    // see setColor for an explanation
    return ((colorState & 0xff00) << 8) | ((colorState & 0xff0000) >> 8) | (colorState & 0xff);
}