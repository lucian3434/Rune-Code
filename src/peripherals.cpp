#include "peripherals.h"

// define our ESCS and how we communicate with them
Motor::BIDSHOTMotor motors[NUM_MOTORS] {
  Motor::BIDSHOTMotor(ESC_M1, pio0, Motor::DSHOT600, 14),
  Motor::BIDSHOTMotor(ESC_M2, pio0, Motor::DSHOT600, 14)
};

// configuring IO on the blaster
Debounce::Button rev = Debounce::Button(IO3, true, true);
// below IO is referenced by name by the fire control logic
Debounce::Button cycle = Debounce::Button(IO1, true, true);
Debounce::Button trig = Debounce::Button(IO2, true, true);
Debounce::Button sel1 = Debounce::Button(IO6, true, true);
Debounce::Button sel2 = Debounce::Button(IO5, true, true);

LED::WS2812 led = LED::WS2812(LED_DATA, pio1);

// create DRV8244 (pusher driver) object
DRV::DRV824xS drv = DRV::DRV824xS(DRV_EN, DRV_PH, DRV_NSLEEP, DRV_MOSI, DRV_MISO, DRV_NSCS, DRV_SCLK, spi0);