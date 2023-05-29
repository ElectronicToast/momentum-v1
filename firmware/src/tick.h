/**
 * @file tick.h
 * @brief SysTick millisecond tick functions
 */

#ifndef TICK_H
#define TICK_H


#include "pico/stdlib.h"
#include "hardware/structs/systick.h"

void tick_init();

// Define to overload the default Systick interrupt handler in the crt0.S file
extern void isr_systick();
#define isr_tick() isr_systick()


#endif // TICK_H
