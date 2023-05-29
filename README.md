# momentum-v1
An elegant circuit board for a more civilized age

<p align="center">
  <b>Star Wars: Revenge of the Ditch<br>
     26 May 2023</b><br>
</p>

## Description
This is a simple prop controller designed for a Ditch Day stack at Caltech. [Ditch Day](https://www.pasadenanow.com/main/ditch-day-on-friday-caltech-seniors-create-a-whirlwind-of-fun-and-challenges) is a tradition where undergraduate seniors design elaborate themed experiences, called "stacks", for the enjoyment of younger students on a date kept secret until the last minute.

This Ditch Day stack featured custom lightsabers with WS2812 LED lighting and motion-reactive sound effects. This repository contains the hardware and firmware used in the lightsabers.

## Hardware
- Raspberry Pi Pico
- MT-3608 boost converter module, permitting flexible power input (designed for 2S D cell batteries)
- 1x WS2812 LED output with 5V buffer
- PAM8302 2.5W Class-D audio amplifier
- 2x pushbutton input with RC debouncing 
- Socket for MPU-6050 IMU

Fabrication output is generated using [KiKit](https://github.com/yaqwsx/KiKit), targeting the JLCPCB assembly service. It is recommended to order panels to lower costs; 10 panels of 4 were fabricated for Ditch Day with a target of 20 production units.

- [Schematic](https://github.com/ElectronicToast/momentum-v1/blob/main/board/doc/momentum.pdf)
- [Interactive BOM](http://htmlpreview.github.io/?https://github.com/ElectronicToast/momentum-v1/blob/main/board/doc/ibom.html)

## Firmware
The code is written using the Raspberry Pi Pico C SDK and has been tested to build and run properly on Ubuntu 22.04 LTS (with the Pico SDK installed per Raspberry Pi's instructions) and Windows (using the [Windows installer](https://github.com/raspberrypi/pico-setup-windows)).  

## Assembly Notes
- The MPU-6050 is intended to fit over the MT-3608 boost converter. Header pins can be soldered to the MPU-6050, then soldered into the row of pins on the PCB. Take care to respect the orientation.
- The LED output pads are not in the same order as a typical WS2812 strip (board errata).
- There is no reverse polarity protection, though the MT-3608 may be tolerant of reverse polarity.

## Usage
`uf2` binaries as used for Ditch Day are provided in `firmware/bin/`. The nomenclature `44k1r2` refers to audio at 44.1 kHz sampling rate and with every sample repeated twice (so PWM clock frequency of 44.1 kHz * 2 * 256).

To modify the code,
1. Install the Pico C SDK, including `pico-extras`. You might also install Visual Studio Code and the relevant extensions (CMake, C/C++, etc.) if you prefer an IDE.
2. Modify definitions in `config.h` as appropriate for e.g. flash-on-clash animations
3. From `firmware/src/`, `mkdir build`
4. `cd build`
5. `cmake ..`
6. `make` (or `ninja` if using the Windows SDK installer)

Upload the code as with any RP2040 binary using the USB bootloader (plug in USB while holding down BOOTSEL on the Pico, and drag-and-drop the `.uf2` file into the `RPI-P2` 'flash drive' that appears.

## Customizing sounds
A converter utility Python script is provided, which takes a set of WAV files (one `poweron.wav` 'ignition' sound, one `poweroff.wav` deactivation sound, one 'hum.wav' idle sound, and any number of `clash0.wav` `clash1.wav`... clash sounds and `swing0.wav` `swing1.wav`... swing sounds) and creates a C header file with `uint8_t` arrays and suitable definitions. 

If you would like to use your own sounds, convert them to WAV, run the Python script on them, and include the generated `.h` file in `speaker.c`.

## Future Improvements
- Single custom PCB with more flash (RP2040 supports up to 16 MB versus Pico stock 2 MB)
- SD card support
- Motion detection uses interrupt features of MPU-6050, which use only the accelerometer. Clash and swing sensing can be improved.
- Due to budget, there is no physical power switch, and WS2812s have high quiescent current. Add a physical power switch or a MOSFET.
- Design for Li-Ion power versus 2S D-cell (chosen for safety)

<p align="center">
  <b>Go to bed frosh, Ditch Day is Tomorrow!<br>
</p>
