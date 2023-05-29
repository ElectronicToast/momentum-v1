/**
 * @file button.h
 * @brief Pushutton driver
 */


#ifndef BUTTON_H_
#define BUTTON_H_


#include "pico/stdlib.h"


#define BTN_DEBOUNCE_TICK_COUNT     10

// Number of milliseconds minimum for a short press
#define BTN_SHORT_PRESS_MS  10
// Number of milliseconds minimum for a long press
#define BTN_LONG_PRESS_MS   1000


void btn_init();

void btn_clear_press();
bool btn_has_short_press();
bool btn_has_long_press();

void btn_handler();


#endif /* BUTTON_H_ */
