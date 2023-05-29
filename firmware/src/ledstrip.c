/**
 * @file ledstrip.c
 * @brief LED strip control
 */


#include "pico/stdlib.h"
#include "hardware/sync.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"

#include "config.h"
#include "pinmap.h"
#include "utilities.h"

#include "ledstrip.h"


#ifndef SABER_DARK_SIDE
    const uint32_t __LEDSTRIP_COLORS[] = {
        LEDSTRIP_COLOR_BLUE,
        LEDSTRIP_COLOR_GREEN,
        LEDSTRIP_COLOR_PURPLE,
        LEDSTRIP_COLOR_TEAL,
        LEDSTRIP_COLOR_YELLOW,
        LEDSTRIP_COLOR_WHITE,
        LEDSTRIP_COLOR_ORANGE,
        LEDSTRIP_COLOR_PINK,
        LEDSTRIP_COLOR_RED
    };

    const uint32_t __LEDSTRIP_FLASH_COLORS[] = {
        LEDSTRIP_FLASH_BLUE,
        LEDSTRIP_FLASH_GREEN,
        LEDSTRIP_FLASH_PURPLE,
        LEDSTRIP_FLASH_TEAL,
        LEDSTRIP_FLASH_YELLOW,
        LEDSTRIP_FLASH_WHITE,
        LEDSTRIP_FLASH_ORANGE,
        LEDSTRIP_FLASH_PINK,
        LEDSTRIP_FLASH_RED
    };
#else
    const uint32_t __LEDSTRIP_COLORS[] = {
        LEDSTRIP_COLOR_RED
    };
    const uint32_t __LEDSTRIP_FLASH_COLORS[] = {
        LEDSTRIP_FLASH_RED
    };
#endif


volatile bool __do_turn_on = false;
volatile bool __do_turn_off = false;

volatile uint32_t __led_counter = 0;
volatile uint32_t __led_color_idx = 0;

volatile uint32_t __led_tick_count = 0;

volatile uint32_t __led_flash_tick_count = 0;
volatile uint32_t __led_flash_count = 0;
volatile bool __do_flash = false;
volatile bool __flash_on = false;


static inline void __put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline void __fill_pixels(uint32_t pixel_grb, uint32_t n_pixels) {
    for (uint32_t i = 0; i < n_pixels; i++) {
        __put_pixel(pixel_grb);
    }
    for (uint32_t i = n_pixels; i < N_LEDSTRIP_LEDS; i++) {
        __put_pixel(LEDSTRIP_COLOR_OFF);
    }
}


void ledstrip_init() {
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    // Not an RGBW strip
    ws2812_program_init(pio, sm, offset, PIN_LEDSTRIP, 800000, false);

    // If picking a random color, do it
    // Modulo N_LEDSTRIP_COLORS - 1 with last color red, so never red on start
    #ifdef SABER_STARTUP_COLOR_RANDOM
        __led_color_idx = rand_powof2(3) % (N_LEDSTRIP_COLORS - 1);
    #endif

    // Make it red if it's the dark side
    #ifdef SABER_DARK_SIDE
        __led_color_idx = N_LEDSTRIP_COLORS - 1;
    #endif

    __do_turn_on = false;
    __do_turn_off = false;
    __led_counter = 0;
    __led_tick_count = 0;
    __do_flash = false;

    // Turn off all LEDs
    __fill_pixels(LEDSTRIP_COLOR_OFF, N_LEDSTRIP_LEDS);
}


void ledstrip_set_random_color() {
    #ifndef SABER_DARK_SIDE
        __led_color_idx = rand_powof2(3) % (N_LEDSTRIP_COLORS - 1);
    #endif
}


void ledstrip_clear() {
    __do_turn_on = false;
    __do_turn_off = false;
    __led_counter = 0;
    __led_tick_count = 0;
    __fill_pixels(LEDSTRIP_COLOR_OFF, N_LEDSTRIP_LEDS);
}


void ledstrip_turn_on() {
    if ((__do_turn_on == false) && (__do_turn_off == false)) {
        __do_turn_on = true;
        __led_counter = 0;
    }
}

void ledstrip_turn_off() {
    if ((__do_turn_on == false) && (__do_turn_off == false)) {
        __do_turn_off = true;
        __led_counter = N_LEDSTRIP_LEDS;
    }
}

// Assumes LEDs are filled in, so fill them 
void ledstrip_next_color() {
// Only change the color if not dark side mode
#ifndef SABER_DARK_SIDE
    __led_color_idx++;
    if (__led_color_idx >= N_LEDSTRIP_COLORS)
        __led_color_idx = 0;
    __fill_pixels(__LEDSTRIP_COLORS[__led_color_idx], N_LEDSTRIP_LEDS);
#endif
}


#ifdef LEDSTRIP_FLASH_ON_CLASH
void ledstrip_flash() {
    uint32_t status = save_and_disable_interrupts();
    __do_flash = true;
    __led_flash_count = rand_powof2_range(0, N_LEDSTRIP_FLASH_MAX) + 1;
    __led_flash_tick_count = rand_powof2_range(N_LEDSTRIP_FLASH_MIN_MS, N_LEDSTRIP_FLASH_MAX_MS);
    __flash_on = false;
    restore_interrupts(status);
}
#endif


// Call in a millisecond interrupt
void ledstrip_handler() {
    // If turning on, turn on one LED at a time when counter hits top
    if (__do_turn_on) {
        __led_tick_count++;
        if (__led_tick_count >= N_LEDSTRIP_TURN_ON_MS) {
            __led_counter++;
            __led_tick_count = 0;
            __fill_pixels(__LEDSTRIP_COLORS[__led_color_idx], __led_counter);
            if (__led_counter == N_LEDSTRIP_LEDS)
                __do_turn_on = false;
        }
    } 

    // If turning off, turn off one LED at a time when counter hits top
    else if (__do_turn_off) {
        __led_tick_count++;
        if (__led_tick_count >= N_LEDSTRIP_TURN_OFF_MS) {
            __led_counter--;
            __led_tick_count = 0;
            __fill_pixels(__LEDSTRIP_COLORS[__led_color_idx], __led_counter);
            if (__led_counter == 0)
                __do_turn_off = false;
        }
    }

    else {
        __led_tick_count = 0;

        #ifdef LEDSTRIP_FLASH_ON_CLASH
        // If flashing, flash the LEDs
        if (__do_flash) {
            __led_flash_tick_count--;
            if (__led_flash_tick_count == 0) {
                // If the flash is currently on, decrement flash count, done with one iteration
                if ( __flash_on) {
                    __led_flash_count--;
                }
                // If flash count has hit zero, done flashing
                if (__led_flash_count == 0) {
                    __do_flash = false;
                    __fill_pixels(__LEDSTRIP_COLORS[__led_color_idx], N_LEDSTRIP_LEDS);
                } else {
                    // If flash is currently high, turn it off
                    if (__flash_on) {
                        // Compute new flash tick count for delay between flashes
                        __led_flash_tick_count = rand_powof2_range(
                                                    N_LEDSTRIP_FLASH_DELAY_MIN_MS, 
                                                    N_LEDSTRIP_FLASH_DELAY_MAX_MS);
                        __fill_pixels(__LEDSTRIP_COLORS[__led_color_idx], N_LEDSTRIP_LEDS);
                        __flash_on = false;
                    } else {
                        // Compute new flash tick count for duration
                        __led_flash_tick_count = rand_powof2_range(
                                                    N_LEDSTRIP_FLASH_MIN_MS, 
                                                    N_LEDSTRIP_FLASH_MAX_MS);
                        __fill_pixels(__LEDSTRIP_FLASH_COLORS[__led_color_idx], N_LEDSTRIP_LEDS);
                        __flash_on = true;
                    }
                }
            }
        }
        #endif
    }
}
