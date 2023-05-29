/**
 * @file utilities.h
 * @brief Utility functions
 */

#ifndef UTILITIES_H
#define UTILITIES_H


#include <stdint.h>

uint32_t rand_powof2(uint8_t n_bits);
uint32_t rand_powof2_range(uint8_t n_bits_min, uint8_t n_bits_max);


#endif // UTILITIES_H