#include "drv/drv824xh.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include <stdio.h>

//yet to implement fault detection, current sense, or PWM

DRV::DRV824xH::DRV824xH(uint8_t in1, uint8_t in2, uint8_t nsleep_pin, uint8_t nfault_pin, uint8_t drvoff_pin) {
    ph = in1;
    en = in2;
    nsleep = nsleep_pin;
    nfault = nfault_pin;
    drvoff = drvoff_pin;
}

void DRV::DRV824xH::init() {
    //initialize and set inputs and shutoff to zero
    gpio_init(nsleep);
    gpio_set_dir(nsleep, GPIO_OUT);
    gpio_put(nsleep, false);

    gpio_init(nfault);
    gpio_set_dir(nfault, GPIO_IN);
    //gpio_pull_down(nfault);

    gpio_init(ph);
    gpio_set_dir(ph, GPIO_OUT);
    gpio_put(ph, false);

    gpio_init(en);
    gpio_set_dir(en, GPIO_OUT);
    gpio_put(en, false);

    gpio_init(drvoff);
    gpio_set_dir(drvoff, GPIO_OUT);
    gpio_put(drvoff, false);
}

bool DRV::DRV824xH::wake() {
    // conduct wake up sequence as per docs
    uint64_t timestamp;
    
    gpio_put(nsleep, true); // initiate device wakeup (t0)
    
    timestamp = time_us_64();
    while (gpio_get(nfault)) { // wait for device to respond by dropping nfault (t2)
        if (time_us_64() > timestamp + 1000) { // if 1000us passes and device hasnt responded, reset and exit
            gpio_put(nsleep, false);
            return false;
        }
        sleep_us(10);
    }
    sleep_us(1000); // not sure if this is necessary, but it doesn't hurt to have it
    
    gpio_put(nsleep, false); // acknowledge device wakeup (t4)
    
    timestamp = time_us_64();
    while (!gpio_get(nfault)) { // wait for acknowledgement of acknowledgement (t5)
        if (time_us_64() > timestamp + 50) { // if 50us passes and nfault is still low, assume some issue and exit
            return false;
        }
        sleep_us(5);
    }
    
    gpio_put(nsleep, true); // end ack pulse, now in operational state
    return true; // successfully woken device
}

void DRV::DRV824xH::sleep() {
    //nsleep to 0 as per the docs

    gpio_put(nsleep, false);
}

void DRV::DRV824xH::drive(){
    //change order to change direction
    gpio_put(ph, false);
    gpio_put(en, true);
}


void DRV::DRV824xH::brake(){
    gpio_put(ph, false);
    gpio_put(en, false); 
}


void DRV::DRV824xH::coast(){
    //chip coasts high
    gpio_put(ph, true);
    gpio_put(en, true);
}