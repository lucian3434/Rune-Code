#pragma once

// which board to use (LUX_NATIVE_BOARD, LUX_PICO_BOARD, RUNE_V0_1, RUNE_V0_2)
#define RUNE_V0_2

// pin definitions for use with a pico
#ifdef LUX_PICO_BOARD

#define ESC_M2 6
#define ESC_M4 7

#define SOL_EN 13

#define TRIG_1 14

#endif

// pin definitions for the official lux board
#ifdef LUX_NATIVE_BOARD

#define SELSW_1 0
#define SELSW_2 1
#define SELSW_3 2

#define ESC_M1 3
#define ESC_M2 4
#define ESC_M3 5
#define ESC_M4 6

#define ESC_TELEM 7

#define WS2811_OUT 8

#define SOL_EN 11

#define TRIG_1 20
#define TRIG_2 19

#define ADC_BATT 26 // Battery voltage, ADC0
#define ADC_CURR_SENSE 27 // ESC current, ADC1
#define ADC_SOL_CURR 28 // Solenoid current, ADC2

#endif

// pin definitions for Rune V0.1
#ifdef RUNE_V0_1

#define ESC_M1 0
#define ESC_M2 1
#define ESC_M3 2
#define ESC_M4 3

#define ESC_TELEM 4

#define I2C_SCL 5
#define I2C_SDA 6

#define IO2 7
#define IO5 8
#define IO6 9
#define IO1 10
#define IO3 11
#define IO4 12

// a few unused pins here

#define DRV_NSLEEP 17
#define DRV_PH 18
#define DRV_EN 19
#define DRV_NFAULT 20
#define DRV_DRVOFF 21

// and a few more unused pins

#define ESC_ENABLE 25

#define ADC_BATT 26 // Battery voltage, ADC0
#define ADC_CURR_SENSE 27 // ESC current, ADC1
#define ADC_DRV_IPROPI 28 // DRV full bridge current, ADC2

#endif

#ifdef RUNE_V0_2

#define ESC_M1 0
#define ESC_M2 1
#define ESC_M3 2
#define ESC_M4 3

#define ESC_TELEM 4

#define I2C_SCL 5
#define I2C_SDA 6

#define IO2 7
#define IO5 8
#define IO6 9
#define IO1 10
#define IO3 11
#define IO4 12

// a few unused pins here

#define DRV_NSLEEP 16
#define DRV_EN 17
#define DRV_PH 18
#define DRV_MOSI 19
#define DRV_MISO 20
#define DRV_NSCS 21
#define DRV_SCLK 22

// one unused here

#define LED_DATA 24

#define ESC_ENABLE 25

#define ADC_BATT 26 // Battery voltage, ADC0
#define ADC_CURR_SENSE 27 // ESC current, ADC1
#define ADC_DRV_IPROPI 28 // DRV full bridge current, ADC2

#endif