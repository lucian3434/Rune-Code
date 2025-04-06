#pragma once

#include <stdint.h>
#include "hardware/spi.h"

namespace DRV {
    // used to drive an SPI (S) variant of the DRV8243, DRV8244, or DRV8245
    // some pins are omitted as implemented on Rune

    // for a register list, see section 8.6.1 of the datasheet

    class DRV824xS {
        protected:
            uint8_t ph;
            uint8_t en;
            uint8_t nsleep;
            uint8_t mosi;
            uint8_t miso;
            uint8_t nscs;
            uint8_t sclk;
            spi_inst_t* spiInstance = nullptr;
            int writeWord(uint8_t address, uint8_t data);
            uint16_t readWord(uint8_t address);
            inline spi_hw_t* get_spi_hw();

        public:
            DRV824xS(uint8_t in1, uint8_t in2, uint8_t nsleep_pin, uint8_t mosi_pin, uint8_t miso_pin, uint8_t nscs_pin, uint8_t sclk_pin, spi_inst_t* spi_instance);
            void init();
            bool wake();
            void sleep();
            void drive();
            void brake();
            void coast();
    };
}