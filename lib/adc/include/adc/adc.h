#pragma once

#include <stdint.h>
#include "hardware/adc.h"

namespace ADC {
    class ADC {
        protected:
            uint8_t pinmask;
            uint16_t* buf[5];
            uint16_t buf_depths[5];
        public:
            ADC();
            void init(); 
            void addPin(uint8_t gpio, uint16_t buffer_depth);
    };
}