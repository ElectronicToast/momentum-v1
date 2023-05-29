/**
 * @file isr.c
 * @brief Interrupt Service Routines
 */

#include "pico/stdlib.h"

#include "tick.h"
#include "ledstrip.h"
#include "button.h"


#define TICK_COUNT_TOP 100
//static uint32_t __tick_count = 0;


// On tick interrupt, blink Pico onbard LED at TICK_COUNT_TOP*2 Hz
void isr_tick() {
    //__tick_count++;
    //if (__tick_count >= TICK_COUNT_TOP) {
    //    __tick_count = 0;
    //    gpio_xor_mask(1 << PICO_DEFAULT_LED_PIN);
    //}

    ledstrip_handler();
    btn_handler();
}


