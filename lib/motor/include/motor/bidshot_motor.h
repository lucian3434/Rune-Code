#pragma once

#include "motor.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include <unordered_map>

namespace Motor {
    enum DSBitrate {
        // DSHOT150 not in bidir spec
        DSHOT300  = 300000,
        DSHOT600  = 600000,
        DSHOT1200 = 1200000
    };

    class BIDSHOTMotor {
        private:
            uint8_t pin;
            PIO pioInstance;
            uint sm;
            DSBitrate bitrate;
            bool requestTelem;
            static uint programOffset;
            uint8_t numPoles;
            uint32_t rpm;
            uint32_t rawTelem;
            uint dmaChannel;

            // for mapping DMA channels to instances
            static std::unordered_map<uint, BIDSHOTMotor*> instanceMap;

            // handler for telemetry
            static void pio_dma_handler() {
                for (auto& [channel, instance] : instanceMap) {
                    if (dma_channel_get_irq0_status(channel)) {
                        dma_channel_acknowledge_irq0(channel); // clear interrupt
                        dma_channel_transfer_to_buffer_now(channel, &instance->rawTelem, UINT32_MAX); // restart channel
                    }
                }
            };

        public:
            BIDSHOTMotor(uint8_t gpioPin, PIO pio, DSBitrate dshotBitrate, uint8_t poles);
            void init(); // initialize motor
            void setThrottle(float throttle); // set throttle to a fractional value from 0 to 1 (inclusive)
            void requestTelemetry(); // request KISS telemetry after the next packet
            uint32_t readTelemetry(); // returns the current rpm value
    };
}