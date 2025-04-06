#include "solenoid/solenoid.h"
#include "pico/stdlib.h"

Solenoid::Solenoid::Solenoid(uint8_t gpioPin) {
    pin = gpioPin;
}

void Solenoid::Solenoid::init() {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, false);
}

void Solenoid::Solenoid::setOutput(bool state) {
    gpio_put(pin, state);
}