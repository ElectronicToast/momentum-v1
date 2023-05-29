/**
 * @file pinmap.h
 * @brief Raspberry Pi Pico pinout
 */


#ifndef _PINMAP_H_
#define _PINMAP_H_


// WS2812B LED strip
#define PIN_LEDSTRIP            15

// PAM8302 audio amplifier PWM output
#define PIN_SPK_PWM             9

// PAM8302 active high enable
#define PIN_SPK_EN              5     

// Pushbutton (Button A on board)
#define PIN_BTN                 22

// IMU
#define PIN_IMU_SDA             16
#define PIN_IMU_SCL             17
#define PIN_IMU_INT             14


#endif /* _PINMAP_H_ */