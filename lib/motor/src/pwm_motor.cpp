#include "motor/pwm_motor.h"
/*
Motor::PWMMotor::PWMMotor(uint8_t gpioPin) {
    pin = gpioPin;
}*/

void Motor::PWMMotor::init() {
    // init pwm slice, adjust timer config
    gpio_set_function(pin, GPIO_FUNC_PWM);
    pwmSlice = pwm_gpio_to_slice_num(pin);
    pwm_set_wrap(pwmSlice, 2000); // set timer to reset at 2000 counts
    pwm_set_clkdiv(pwmSlice, 125.0); // with clock divider of 125, for 500hz reset rate
    pwm_set_gpio_level(pin, 1000); // start off at 0 throttle (50% duty cycle)
    pwm_set_enabled(pwmSlice, true);
    printf("Motor initialized\r\n");
}


void Motor::PWMMotor::setThrottle(float throttle) {
    if (throttle < 0) throttle = 0;
    else if (throttle > 1) throttle = 1; // clamp between 0 and 1
    uint16_t compareLevel = (uint16_t)(throttle * 1000) + 1000; // range 1000-2000
    pwm_set_gpio_level(pin, compareLevel); // update timer compare value
}