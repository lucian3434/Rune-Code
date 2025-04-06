#include "drv/drv824xs.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/binary_info.h"
#include <stdio.h>

// TI format doesn't seem to work?
#define DRV_SPI_MOTOROLA_FORMAT

//yet to implement fault detection, current sense, or PWM

DRV::DRV824xS::DRV824xS(uint8_t in1, uint8_t in2, uint8_t nsleep_pin, uint8_t mosi_pin, uint8_t miso_pin, uint8_t nscs_pin, uint8_t sclk_pin, spi_inst_t* spi_instance) {
    ph = in1;
    en = in2;
    nsleep = nsleep_pin;
    mosi = mosi_pin;
    miso = miso_pin;
    nscs = nscs_pin;
    sclk = sclk_pin;
    spiInstance = spi_instance;
}

void DRV::DRV824xS::init() {
    //initialize io, leave chip asleep for now
    gpio_init(nsleep);
    gpio_set_dir(nsleep, GPIO_OUT);
    gpio_put(nsleep, false);

    gpio_init(ph);
    gpio_set_dir(ph, GPIO_OUT);
    gpio_put(ph, false);

    gpio_init(en);
    gpio_set_dir(en, GPIO_OUT);
    gpio_put(en, false);

    // initiate spi at 500khz
    spi_init(spiInstance, 5e5);
    #ifdef DRV_SPI_TI_FORMAT
    // we have to set this up with the TI SSI frame format, but there's no way to do that with the sdk so we need to manipulate registers directly
    spi_hw_t* spihwreg = get_spi_hw(); // get a pointer to the appropriate set of registers
    spihwreg->cr0 &= ~(0b11 << 4); // clear the frame format bits
    spihwreg->cr0 |= (0b01 << 4); // set the frame format bits to specify TI synchronous serial
    #endif
    gpio_set_function(mosi, GPIO_FUNC_SPI);
    gpio_set_function(miso, GPIO_FUNC_SPI);
    gpio_set_function(sclk, GPIO_FUNC_SPI);

    #ifdef DRV_SPI_MOTOROLA_FORMAT
    gpio_init(nscs);
    gpio_set_dir(nscs, GPIO_OUT);
    gpio_put(nscs, true);
    #endif
    #ifdef DRV_SPI_TI_FORMAT
    gpio_set_function(nscs, GPIO_FUNC_SPI); // apparently only possible because we are in TI SSI mode
    #endif

    spi_set_format(spiInstance, 16, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST); // 16 bit reads and writes
    // special picotool stuff that was in the example code
    // probably not necessary, but it doesn't hurt to have it either
    bi_decl(bi_3pins_with_func(miso, mosi, sclk, GPIO_FUNC_SPI));
    bi_decl(bi_1pin_with_name(nscs, "SPI CS"));

    // TODO: DMA channels for more efficient communication after the device is set up (checking faults, etc)
    // this isn't currently implemented because strictly speaking it isn't necessary for basic operation
}

bool DRV::DRV824xS::wake() {
    // conduct wake up sequence as per docs
    uint64_t timestamp;
    
    gpio_put(nsleep, true); // initiate device wakeup (t0)
    sleep_us(2000); // wait until after device initialization complete (t3)

    uint16_t devID = readWord(0x00); // try to read the device id to see if it woke up
    if ((devID & 0xc0) == 0) { // invalid frame, device is probably not awake
        gpio_put(nsleep, false);
        return false;
    }
    
    writeWord(0x08, (1 << 7)); // send device reset packet (command reg, CLR_FLT = 1)
    devID = readWord(0x00); // check again
    if ((devID & (1 << 13))) {
        // fault flag still set
        gpio_put(nsleep, false);
        return false;
    }

    // if we get to here, we know the device is ready and working. start configuring
    uint16_t tmpReg = readWord(0x0c);
    tmpReg = (tmpReg & ~0x03) | 0x03; // set S_MODE = 0b11 (pwm mode)
    writeWord(0x0c, tmpReg & 0xff);
    // further config TBD but this should work

    return true; // successfully woken device
}

void DRV::DRV824xS::sleep() {
    //nsleep to 0 as per the docs

    gpio_put(nsleep, false);
}

void DRV::DRV824xS::drive() {
    //change order to change direction
    gpio_put(ph, false);
    gpio_put(en, true);
}

void DRV::DRV824xS::brake() {
    gpio_put(ph, false);
    gpio_put(en, false); 
}

void DRV::DRV824xS::coast() {
    //chip coasts high
    gpio_put(ph, true);
    gpio_put(en, true);
}

// "dumb" write where we block until the data is sent
int DRV::DRV824xS::writeWord(uint8_t addr, uint8_t data) {
    uint16_t buf = ((addr & 0x3f) << 8) | data;
    #ifdef DRV_SPI_MOTOROLA_FORMAT
    gpio_put(nscs, false);
    #endif
    int res = spi_write16_blocking(spiInstance, &buf, 1);
    #ifdef DRV_SPI_MOTOROLA_FORMAT
    gpio_put(nscs, true);
    #endif
    sleep_us(10);
    return res;
}

// "dumb" read where we block until all the data is read
// see table 8-21 from the datasheet for the exact format data is returned in
uint16_t DRV::DRV824xS::readWord(uint8_t address) {
    //uint16_t bufTX[2] = { (uint16_t)(((address & 0x3f) | 0x40) << 8), 1 << 14 }; // set read bit and specify address
    uint16_t bufTX = (uint16_t)(((address & 0x3f) | 0x40) << 8);
    uint16_t bufRX;
    //spi_write16_blocking(spiInstance, &bufTX, 1); // tell the device where we want to read from
    //spi_read16_blocking(spiInstance, 0, &bufRX, 1); // get the device's response
    #ifdef DRV_SPI_MOTOROLA_FORMAT
    gpio_put(nscs, false);
    #endif
    spi_write16_read16_blocking(spiInstance, &bufTX, &bufRX, 1);
    #ifdef DRV_SPI_MOTOROLA_FORMAT
    gpio_put(nscs, true);
    #endif
    sleep_us(10);
    return bufRX;
}

inline spi_hw_t* DRV::DRV824xS::get_spi_hw() {
    if (spiInstance == spi0) {
        return spi0_hw;
    }
    else if (spiInstance == spi1) {
        return spi1_hw;
    }
    else {
        return NULL;
    }
}