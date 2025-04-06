#include "debounce/button.h"
#include "pico/stdlib.h"

Debounce::Button::Button(uint8_t gpioPin) {
    pin = gpioPin;
    timestamp = 0;
    state = 0;
    inverted = false;
    hasPullup = false;
}

Debounce::Button::Button(uint8_t gpioPin, bool invert, bool pullup) {
    pin = gpioPin;
    timestamp = 0;
    state = 0;
    inverted = invert;
    hasPullup = pullup;
}

uint64_t Debounce::Button::timeout = 10 * 1000; // 10ms debounce

void Debounce::Button::init() {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    if (hasPullup) gpio_pull_up(pin);
    else gpio_pull_down(pin);
}

void Debounce::Button::update() {
    if (timestamp == 0 || time_us_64() > timestamp + timeout) {
        // button has never been pressed or it has been longer than the debouncing period
        state = state << 1;
        if (gpio_get(pin) ^ inverted) {
            state |= 0x01; // set lastmost bit if pin is high
        }
        if (((state & 0x02) >> 1) != state & 0x01) { // detect an edge
            timestamp = time_us_64();
        }
    }
    else { // still in debounce timeout -> pretend pin is still in previous state
        state = (state << 1) | (state & 0x01);
    }
}

bool Debounce::Button::isPressed() {
    if (state & 0x01) return true; // if last bit is anything other than 0
    return false;
}

bool Debounce::Button::isRisingEdge() {
    if ((state & 0x03) == 0x01) return true; // if last two bits are 01
    return false;
}

bool Debounce::Button::isFallingEdge() {
    if ((state & 0x03) == 0x02) return true; // if last two bits are 10
    return false;
}