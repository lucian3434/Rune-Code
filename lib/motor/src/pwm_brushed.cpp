#include "motor/pwm_brushed.h"

void Motor::PWMBrushed::init() {
    // init pwm slice, adjust timer config]
    polarity = false;
    cthrottle = 0.0;
    gpio_set_function(pin, GPIO_FUNC_PWM);
    pwmSlice = pwm_gpio_to_slice_num(pin);
    pwm_set_wrap(pwmSlice, 1000); // set timer to reset at 1000 counts
    pwm_set_clkdiv(pwmSlice, 6.0); // with clock divider of 6, for ~20khz reset rate
    pwm_set_gpio_level(pin, 0); // start off at 0 throttle (0% duty cycle)
    pwm_set_enabled(pwmSlice, true);
    printf("Motor initialized\r\n");
}


void Motor::PWMBrushed::setThrottle(float throttle) {
    if (throttle < 0) throttle = 0;
    else if (throttle > 1) throttle = 1; // clamp between 0 and 1
    cthrottle = throttle;
    if (polarity) throttle = 1 - throttle; // if polarity is inverted, flip throttle
    uint16_t compareLevel = (uint16_t)(throttle * 1000); // range 0-1000
    pwm_set_gpio_level(pin, compareLevel); // update timer compare value
}

/*
 * the more "expected" way of implementing this would be to use the pwm_set_output_polarity(),
 * but this requires you to set the status for both channels of each pwm slice at the same 
 * time. there doesn't seem to be a function written for getting the status of a pwm channel
 * and i don't currently feel like writing this with direct register access so for now this is
 * written with a stored throttle and polarity bit, and inversion is done in the setThrottle
 * function. this means that exact timing is probably different than what you would get with
 * a properly inverted pwm signal, but i don't care since i only want this to drive a motor
*/
void Motor::PWMBrushed::setOutputPolarity(bool invert) {
    polarity = invert;
    setThrottle(cthrottle);
}