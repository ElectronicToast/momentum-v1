/**
 * @file button.c
 * @brief Button driver
 */


#include "pico/stdlib.h"
#include "hardware/sync.h"

#include "button.h"
#include "pinmap.h"


volatile bool __has_short_press = false;
volatile bool __has_long_press = false;
volatile uint32_t __press_count = 0;
volatile bool __btn_prev_down = false;
volatile bool __long_press_handled = false;
volatile uint32_t __btn_debounce_count = BTN_DEBOUNCE_TICK_COUNT;
volatile bool __btn_pressed = false;
volatile bool __btn_prev_pressed = false;


void btn_init() {
    gpio_init(PIN_BTN);
    gpio_set_dir(PIN_BTN, GPIO_IN);
    // Board has physical pull-up
    //gpio_pull_up(PIN_BTN);
}


void btn_clear_press() {
    uint32_t status = save_and_disable_interrupts();
    __has_short_press = false;
    __has_long_press = false;
    restore_interrupts(status);
}


bool btn_has_short_press() {
    bool flag = false;
    uint32_t status = save_and_disable_interrupts();
    if (__has_short_press) {
        __has_short_press = false;
        flag = true;
    }
    restore_interrupts(status);
    return flag;
}


bool btn_has_long_press() {
    bool flag = false;
    uint32_t status = save_and_disable_interrupts();
    if (__has_long_press) {
        __has_long_press = false;
        __long_press_handled = true;
        flag = true;
    }
    restore_interrupts(status);
    return flag;
}


void btn_handler() {

    bool btn_down = (gpio_get(PIN_BTN) == 0) ? true : false;

    // If the button state just changed, restart debouncing
    if (btn_down != __btn_prev_down) {
        __btn_debounce_count = BTN_DEBOUNCE_TICK_COUNT;
    } else {
        // If the button state is stable, decrement the debounce counter
        if (__btn_debounce_count > 0) {
            __btn_debounce_count--;
        }
    }

    // If the debounce counter has hit zero, update the button state
    if (__btn_debounce_count == 0) {
        __btn_pressed = btn_down;
    }

    // If the button is pressed, increment the press count
    if (__btn_pressed) {
        __press_count++;

        if ( (__press_count >= BTN_LONG_PRESS_MS) &&
             (!__long_press_handled) ) {
            __has_long_press = true;
        }
    } else {        
        // If button is debounced to not pressed and press count isn't too
        // high, we have a short press
        if ( (__btn_prev_pressed) &&
             (__press_count < BTN_LONG_PRESS_MS) ) {
            __has_short_press = true;
        }

        __press_count = 0;
        __long_press_handled = false;
    }

    __btn_prev_down = btn_down;
    __btn_prev_pressed = __btn_pressed;



    /*
    // Increment the count if the button is pressed
    if (gpio_get(PIN_BTN) == 0) {
        __btn_prev_down = true;
        __press_count++;

        if ( (__press_count >= BTN_LONG_PRESS_MS) &&
             (!__long_press_handled) ) {
            __has_long_press = true;
        }
    }
    else {
        // On button release, check if it was a short press
        if (__btn_prev_down) {
            if (__press_count >= BTN_SHORT_PRESS_MS && __press_count < BTN_LONG_PRESS_MS) {
                __has_short_press = true;
            }
        }
         __press_count = 0;
        __btn_prev_down = false;
        __long_press_handled = false;
    }
    */
}

