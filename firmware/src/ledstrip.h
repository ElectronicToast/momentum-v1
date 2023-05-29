/**
 * @file ledstrip.h
 * @brief LED strip driver
 */


#ifndef _LEDSTRIP_H_
#define _LEDSTRIP_H_


// Custom 24-bit GRB colors
#define LEDSTRIP_COLOR_OFF      0x000000

#define LEDSTRIP_COLOR_BLUE     0x000020
#define LEDSTRIP_COLOR_GREEN    0x200000
#define LEDSTRIP_COLOR_PURPLE   0x001010
#define LEDSTRIP_COLOR_TEAL     0x100010
#define LEDSTRIP_COLOR_YELLOW   0x202000
#define LEDSTRIP_COLOR_WHITE    0x0d0d0d
#define LEDSTRIP_COLOR_ORANGE   0x082000
#define LEDSTRIP_COLOR_PINK     0x042004

#define LEDSTRIP_COLOR_RED      0x002000

#ifndef SABER_DARK_SIDE
    #define N_LEDSTRIP_COLORS 9
#else
    #define N_LEDSTRIP_COLORS 1
#endif

// Colors for brief higher-intensity clashes
#define LEDSTRIP_FLASH_BLUE     0x202060
#define LEDSTRIP_FLASH_GREEN    0xa00000
#define LEDSTRIP_FLASH_PURPLE   0x206060
#define LEDSTRIP_FLASH_TEAL     0x602060
#define LEDSTRIP_FLASH_YELLOW   0x606000
#define LEDSTRIP_FLASH_WHITE    0x606060
#define LEDSTRIP_FLASH_ORANGE   0x306010
#define LEDSTRIP_FLASH_PINK     0x106010

#define LEDSTRIP_FLASH_RED      0x00a000

#define N_LEDSTRIP_FLASH_TIMEOUT_MS     300





void ledstrip_init();
void ledstrip_clear();
void ledstrip_turn_on();
void ledstrip_turn_off();
void ledstrip_next_color();
void ledstrip_set_random_color();

#ifdef LEDSTRIP_FLASH_ON_CLASH
void ledstrip_flash();
#endif

void ledstrip_handler();


#endif /* _LEDSTRIP_H_ */
