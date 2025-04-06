#include "motor/dshot_motor.h"
#include "dshot_motor.pio.h"

Motor::DSHOTMotor::DSHOTMotor(uint8_t gpioPin, PIO pio, DSBitrate dshotBitrate) {
    pin = gpioPin;
    pioInstance = pio;
    bitrate = dshotBitrate;
}

void Motor::DSHOTMotor::init() {
    // check if the program has been loaded into PIO memory yet
    // as implemented, all motors need to be on the same PIO instance.
    // conveniently, each instance has 4 state machines; exactly as
    // many as we need
    if (!programOffset) {
        programOffset = pio_add_program(pioInstance, &dshot_program);
    }
    sm = pio_claim_unused_sm(pioInstance, true);
    dshot_program_init(pioInstance, sm, programOffset, pin, bitrate);
}


void Motor::DSHOTMotor::setThrottle(float throttle) {
    if (throttle < 0) throttle = 0;
    else if (throttle > 1) throttle = 1; // clamp between 0 and 1
    uint16_t dshotCommand = (uint16_t)(throttle * 2000) + 48; // range 48-2047
    uint16_t dshotFrame = (dshotCommand << 1) | requestTelem; // throttle and telem request
    uint16_t crc = (dshotFrame ^ (dshotFrame >> 4) ^ (dshotFrame >> 8)) & 0x0F; // calculate crc (https://brushlesswhoop.com/dshot-and-bidirectional-dshot/#calculating-the-crc)
    dshotFrame = (dshotFrame << 4) | crc; // shift over data and put in crc to complete the frame

    pio_sm_put(pioInstance, sm, (uint32_t)(dshotFrame) << 16u); // write frame to state machine

    requestTelem = false; // if telemetry has already been requested, dont request again unless asked to
}

void Motor::DSHOTMotor::requestTelemetry() {
    requestTelem = true;
}