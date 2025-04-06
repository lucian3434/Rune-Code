#include "motor/bidshot_motor.h"
#include "bidshot_motor.pio.h"

static const uint32_t GCRdecode[32] {
    0x11111, 0x11111, 0x11111, 0x11111, 0x11111, 0x11111, 0x11111, 0x11111,
    0x11111, 0b1001,  0b1010,  0b1011,  0x11111, 0b1101,  0b1110,  0b1111,
    0x11111, 0x11111, 0b0010,  0b0011,  0x11111, 0b0101,  0b0110,  0b0111,
    0x11111, 0b0000,  0b1000,  0b0001,  0x11111, 0b0100,  0b1100,  0x11111
};

std::unordered_map<uint, Motor::BIDSHOTMotor*> Motor::BIDSHOTMotor::instanceMap;
uint Motor::BIDSHOTMotor::programOffset;

Motor::BIDSHOTMotor::BIDSHOTMotor(uint8_t gpioPin, PIO pio, DSBitrate dshotBitrate, uint8_t poles) {
    pin = gpioPin;
    pioInstance = pio;
    bitrate = dshotBitrate;
    numPoles = poles;
    rpm = 0;
}

void Motor::BIDSHOTMotor::init() {
    // check if the program has been loaded into PIO memory yet.
    // as implemented, all motors need to be on the same PIO instance.
    // conveniently, each instance has 4 state machines; exactly as
    // many as we need
    static bool programLoaded = false;
    if (!programLoaded) {
        programOffset = pio_add_program(pioInstance, &bidshot_program);
        programLoaded = true;
    }
    sm = pio_claim_unused_sm(pioInstance, true);
    bidshot_program_init(pioInstance, sm, programOffset, pin, bitrate);
    pio_sm_clear_fifos(pioInstance, sm); 

    // now we set up the DMA channel for data transfer out of the FIFO
    dmaChannel = dma_claim_unused_channel(true); // panic if we can't get a channel
    dma_channel_config dmacfg = dma_channel_get_default_config(dmaChannel);
    channel_config_set_transfer_data_size(&dmacfg, DMA_SIZE_32); // transfer full word at a time
    channel_config_set_read_increment(&dmacfg, false); // do not increment read address (same spot in FIFO)
    channel_config_set_write_increment(&dmacfg, false); // do not increment write address
    channel_config_set_dreq(&dmacfg, pio_get_dreq(pioInstance, sm, false)); // only transfer when data present
    dma_channel_configure(dmaChannel, &dmacfg, &rawTelem, &pioInstance->rxf[sm], UINT32_MAX, false); // write config to channel
    // note: transfers is set to the max allowable number to reduce the frequency of interrupts

    instanceMap[dmaChannel] = this; // add this object to lookup table
    dma_channel_set_irq0_enabled(dmaChannel, true); // enable dma IRQ for this channel so we know when its finished

    static bool irq_registered = false; // only gets set the first time this is called
    if (!irq_registered) { // check to make sure the irq hasnt been registered by another instance
        irq_add_shared_handler(DMA_IRQ_0, &BIDSHOTMotor::pio_dma_handler, PICO_SHARED_IRQ_HANDLER_LOWEST_ORDER_PRIORITY);
        irq_set_enabled(DMA_IRQ_0, true);
        irq_registered = true;
    }

    dma_channel_start(dmaChannel); // finally, start the dma channel now that everything is set up

}


void Motor::BIDSHOTMotor::setThrottle(float throttle) {
    if (throttle < 0.0) throttle = 0.0;
    else if (throttle > 1.0) throttle = 1.0; // clamp between 0 and 1
    uint16_t dshotCommand = (uint16_t)(throttle * 1999) + 48; // range 48-2047
    uint16_t dshotFrame = (dshotCommand << 1) | requestTelem; // throttle and telem request
    uint16_t crc = (~(dshotFrame ^ (dshotFrame >> 4) ^ (dshotFrame >> 8))) & 0x0F; // calculate crc (https://brushlesswhoop.com/dshot-and-bidirectional-dshot/#calculating-the-crc)
    dshotFrame = (dshotFrame << 4) | crc; // shift over data and put in crc to complete the frame

    pio_sm_put(pioInstance, sm, (uint32_t)(dshotFrame) << 16u); // write frame to state machine

    requestTelem = false; // if telemetry has already been requested, dont request again unless asked to
}

// if you want KISS telemetry (rx not currently implemented), may be removed because the name is confusing
void Motor::BIDSHOTMotor::requestTelemetry() {
    requestTelem = true;
}

// returns 0xfffffffx if no telemetry available or other error
// x => f: no telem avail, e: bit read error, d: failed CRC, c: infinite rpm
uint32_t Motor::BIDSHOTMotor::readTelemetry() {
    /*
    if (pio_sm_is_rx_fifo_empty(pioInstance, sm)) return 0xffffffff;
    uint32_t p = pio_sm_get(pioInstance, sm); // raw telemetry packet
    */
    uint32_t p = rawTelem; // pull most recent telemetry packet
    //printf("%u, ", p);
    p = p ^ (p >> 1); // get GCR encoded telemetry packet
    uint32_t value = GCRdecode[p & 0x1f] | (GCRdecode[(p & (0x1f << 5)) >> 5]) << 4 | (GCRdecode[(p & (0x1f << 10)) >> 10]) << 8 | (GCRdecode[(p & (0x1f << 15)) >> 15]) << 12;
    
    if (value & 0x10000) return 0xfffffffe; // check bit read error
    
    uint32_t data = value >> 4;
    uint32_t calculatedCRC = (~(data ^ (data >> 4) ^ (data >> 8))) & 0x0F;
    
    if (calculatedCRC != (value & 0x0f)) return 0xfffffffd; // failed CRC check
    if (data == 0xfff) return 0; // special case for 0rpm, since this protocol can't give exactly 0
    
    data = (data & 0x1ff) << (data >> 9); // erpm period in microseconds
    
    if (data == 0) return 0xfffffffc; // edge case where period is 0 microseconds and thus meaningless
    
    uint32_t erpm = 60000000 / data; // calculate erpm
    
    return (erpm * 2) / numPoles; // calculate and return true rpm
}
