/**
 * @file config.h
 * @brief System configuration definitions
 */

#ifndef CONFIG_H
#define CONFIG_H


// ------------------------------ OVERALL --------------------------------------
// Uncomment for dark side mode - only color is red, cannot change color
//#define SABER_DARK_SIDE

// Uncomment for deliberately slow power-on and power-off; works well with
// dark side
//#define SABER_POWERONOFF_SLOW

// Uncomment to pick a random color on startup. Only applicable to normal / 
// light side mode.
#define SABER_STARTUP_COLOR_RANDOM
// -----------------------------------------------------------------------------

// ------------------------------ SOUNDS ---------------------------------------
// Select which `tunes_xxx.h` file to use. These files should have a poweron,
// poweroff, hum, and multiple swing/clash sounds comprising a length and a 
// uint8_t array. Use the audio conversion script to generate `tunes_xxx.h` 
// files from a set of WAV files.
//
// If none are defined, the original sounds used in development will be used,
// which have only a single swing/clash sound.
//
// This does not impact how the main loop handles playing different swing/clash
// sounds for different motions (if there is any capability of doing such),
// but the definition can be used in the main loop too to configure this.
// Uncomment only one!
//#define TUNES_USE_EP4                   // Dark side suitable
#define TUNES_USE_OBS                   // Normal-sounding
//#define TUNES_USE_CLASSIC               // Also normal-sounding, hum too quiet
//#define TUNES_USE_OBS_CLASSICPWR          // Classic poweron/off, obs others
// Nothing uncommented -- use original poweron and poweroff but with obs sounds

// Playback frquency of the audio samples
// Uncomment only one!
//#define TUNE_22KHZ
//#define TUNE_8KHZ
#define TUNE_44KHZ
// -----------------------------------------------------------------------------

// ----------------------------- MOTION ----------------------------------------
// In case of unstable power supply that causes the IMU to act up, reset it 
// on poweron or color change button press?
//
// NOT WORKING, WAS NOT EVEN DEFINED IN MAIN
//#define IMU_RESET_ON_EVENT   

// Sleep the IMU when saber off
#define IMU_SLEEP

// Threshold for clash detection, in the default accelerometer units of the IMU
// (MPU-6050 motion detection interrupt)
#define IMU_THRESH_CLASH        48
// Duration for clash detection, in milliseconds
#define IMU_N_CLASH_DUR         20

// Threshold for swing detection, in the default accelerometer units of the IMU
// MPU-6050 zero-motion detection interrupt
#define IMU_THRESH_SWING        144
// Duration for swing detection, in units of 64 ms
#define IMU_N_SWING_DUR         8
// -----------------------------------------------------------------------------

// ----------------------------- LED STRIP -------------------------------------
// Number of WS2812B LEDs in the strip.
#define N_LEDSTRIP_LEDS         25

// Flash LED strip on a clash motion?
//#define LEDSTRIP_FLASH_ON_CLASH

// Number of milliseconds it takes for one LED to turn on / off during strip
// turning on or off
#ifdef SABER_POWERONOFF_SLOW
    #define N_LEDSTRIP_TURN_ON_MS   30
    #define N_LEDSTRIP_TURN_OFF_MS  30
#else 
    #define N_LEDSTRIP_TURN_ON_MS   30
    #define N_LEDSTRIP_TURN_OFF_MS  30
#endif

// Maximum number of flashes to do when a clash is detected, as a power of 2
#define N_LEDSTRIP_FLASH_MAX            12

// Minimum duration of flash, in units of power of 2 milliseconds
#define N_LEDSTRIP_FLASH_MIN_MS         6

// Maximum duration of flash
#define N_LEDSTRIP_FLASH_MAX_MS         12

// Minimum delay between flashes
#define N_LEDSTRIP_FLASH_DELAY_MIN_MS   16

// Maximum delay between flashes
#define N_LEDSTRIP_FLASH_DELAY_MAX_MS   32
// -----------------------------------------------------------------------------

// ---------------------------- SYSTEM -----------------------------------------
#define SYS_POWERON_DELAY       1600        // ms, should > poweron tune length

#define SYS_CLK_FREQ_KHZ        27000 // 108000    // 27000

#define SPK_PWM_COUNT_TOP       255         // 8-bit audio, wrap at 8-bit top

// At 44.1 kHz PWM frequency and 4 repetitions

#define SPK_N_REPETITIONS       2           // N times audio sample is repeated
// Default clock of 125 MHz / 256 = 488 kHz, divided by 22 kHz desired sample
// rate is ~ 22.195. For N_REPETITIONS, the desired sample rate is multiplied
// by that number, so the clock divider is by default 22.0 / N_REPETITIONS
// 
// With N_REPETITIONS = 4, the minimum system frequency is 
//      22 kHz * 4 * 256 = 22.528 MHz.
//
// Page 230 of RP2040 datasheet provides a script to find achievable PLL
// output frequencies. However, with 12MHz crystal, the minimum allowed VCO
// frequency is 756 MHz, achieved with a multiplication ratio of 63. 
// So we can run at 756 MHz / (some integer between 1 and 49) using the post
// dividers for lowest power consumption while > 22.528 MHz. For example, 
// 756 MHz / 28 = 27.0 MHz, and then the PWM clock divider should be 1.1985
//
// For now, use 27 MHz clock regardless of PWM frequency, and just modify
// this divisor
#define SPK_PWM_CLKDIV      1.1958f

//#ifdef TUNE_22KHZ
//    #define SPK_PWM_CLKDIV      1.1985f //22.0f / SPK_N_REPETITIONS
//#elif defined TUNE_8KHZ
//    #define SPK_PWM_CLKDIV      3.2959f //88.0f / SPK_N_REPETITIONS
//#endif

// MPU-6050 I2C configuration
#define IMU_I2C_CLK_FREQ_KHZ    400
#define IMU_I2C_ADDR            0x68
// -----------------------------------------------------------------------------

#endif // CONFIG.H
