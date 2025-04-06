#include "adc/adc.h"
#include "pico/stdlib.h"

ADC::ADC::ADC() {
    pinmask = 0;
}

void ADC::ADC::init() {
    adc_init();
}

void ADC::ADC::addPin(uint8_t gpio, uint16_t buffer_depth) {
    adc_gpio_init(gpio);
    pinmask |= 1 << (gpio - 26);
}