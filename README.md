# Rune Firmware
Reference firmware for the Rune brushless blaster control board. Currently under development, this repository will likely change a good bit in the near future

NOTE: Don't expect massive code changes until after I graduate in December, but new batch of hardware is hopefully coming soon (though we've been saying this for months 😭)

## TODO
- [x] Noise rejection in the PID loop so that a D term is actually helpful (need to take a second look at this)
- [ ] \(BIDSHOT code) Support for extended telemetry packets
- [ ] Better error checking when using telemetry data (urgently needed)
- [ ] Make a better attempt to get everything set up before saying boot was successful (tbd if necessary, usually you can just reboot and it'll work if theres an issue)
- [x] FPS select on boot 
- [ ] Flash read/write for configuration (probably far future, most people will be reflashing code anyway)
- [ ] Refactor main.cpp to make it easier to understand (maybe wip?)
- [ ] Provisions for a solenoid pusher (check dpyo's branch)


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