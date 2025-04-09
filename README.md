# Rune Firmware
Reference firmware for the Rune brushless blaster control board. Currently under development, this repository will likely change a good bit in the near future

## TODO
- [x] Noise rejection in the PID loop so that a D term is actually helpful (or otherwise I re-evaluate my PID tuning ability) **update: i need to re-evaluate my pid tuning ability**
- [ ] \(BIDSHOT code) Support for extended telemetry packets
- [ ] Better error checking when using telemetry data
- [ ] Make a better attempt to get everything set up before saying boot was successful (and add a safety flag so that most code doesn't run until after boot is complete)
- [x] FPS select on boot 
- [ ] Flash read/write for configuration
- [ ] Refactor main.cpp to make it easier to understand
- [ ] Provisions for a solenoid pusher (brushed pusher with cycle switch only atm)
- [ ] More detail in the readme
- [x] Decide what license fits best
- [ ] Figure out what else needs to be added


## Features
- Bidirectional DSHOT by default
- Independent main logic and motor control loops
- Onboard LED indicates whether boot was successful or not
- Safety timeouts to prevent undesirable behaviour
- Target FPS set by selector position at boot
- ...many more to be added

## Usage (TODO)
> [!NOTE]
> Uses Pi Pico SDK in VSCode for development/building.
> To flash a board, hold the boot select button (momentary button next to the USB port) while plugging in your USB cable. When the board is properly connected and in bootloader mode, it should show up as a flash drive named RPI-RP2 on your computer. Drag the .uf2 file from the build folder onto the drive and it will disconnect, restart, and start executing your code.