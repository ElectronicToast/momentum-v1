/**
 * @file tick.c
 * @brief SysTick millisecond tick functions
 */

#include "pico/stdlib.h"
#include "hardware/structs/systick.h"
#include "config.h"

#include "tick.h"


void tick_init() {
    systick_hw->csr = 0; 	    // Disable SysTick
    // Set the reload value to 1ms 
	systick_hw->rvr = (uint32_t) (SYS_CLK_FREQ_KHZ - 1);
	systick_hw->cvr = 0;        // Clear count to force reload 
    systick_hw->csr = 0x7;      // Enable, use system clock, enable interrupt
}
