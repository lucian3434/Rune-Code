#include "motor/motor.h"

Motor::Motor::Motor(uint8_t gpioPin) {
    pin = gpioPin;
}

void Motor::Motor::init() {
    printf("Wrong Init\r\n");
}

void Motor::Motor::setThrottle(float throttle) {
    printf("Wrong Throttle\r\n");
}