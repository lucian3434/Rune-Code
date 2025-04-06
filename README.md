# Rune Firmware
Reference firmware for the Rune brushless blaster control board. Currently under development, this repository will likely change a good bit in the near future

## TODO
- [ ] Noise rejection in the PID loop so that a D term is actually helpful (or otherwise I re-evaluate my PID tuning ability)
- [ ] \(BIDSHOT code) Support for extended telemetry packets
- [ ] Better error checking when using telemetry data
- [ ] Make a better attempt to get everything set up before saying boot was successful (and add a safety flag so that most code doesn't run until after boot is complete)
- [ ] FPS select on boot (or more broadly, write some code to allow for easy interaction with the flash)
- [ ] Refactor main.cpp to make it easier to understand
- [ ] Provisions for a solenoid pusher (brushed pusher with cycle switch only atm)
- [ ] More detail in the readme
- [ ] Decided what license fits best
- [ ] Figure out what else needs to be added


## Features
- Bidirectional DSHOT by default
- Independent main logic and motor control loops
- Onboard LED indicates whether boot was successful or not
- Safety timeouts to prevent undesirable behaviour
- ...many more to be added

## Usage (TODO)
> [!NOTE]
> Uses Pi Pico SDK in VSCode for development/building.
> To flash a board, hold the boot select button (momentary button next to the USB port) while plugging in your USB cable. When the board is properly connected and in bootloader mode, it should show up as a flash drive on your computer. Drag the .uf2 file from the build folder onto the drive and it will disconnect, restart, and start executing your code.