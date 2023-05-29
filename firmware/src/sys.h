/**
 * @file sys.h
 * @brief System functions
 */


#ifndef SYS_H
#define SYS_H


#include "pinmap.h"

#define PIN_SYS_WAKEUP  PIN_BTN

void sys_init();        // Set up clocks, etc.
void sys_go_dormant();


#endif /* SYS_H */
