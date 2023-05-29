# Firmware

The project under `src/` uses the Pico C SDK and includes `.vscode` configurations for compiling/debugging via VSCode. 

## Debugging

The `launch.json` files are configured for use with [pico_debug](https://github.com/essele/pico_debug) by essele, as descrived [here](https://forums.raspberrypi.com/viewtopic.php?t=337284). This is much faster than a stock picoprobe interfacing with OpenOCD's GDB server. The firmware can be flashed onto any picoprobe equivalent target.

Since this is functionally similar to the Black Magic Probe, the launch configuration is named `Pico Magic`.

You may of course use a stock picoprobe, but be sure to change the `launch.json`.
