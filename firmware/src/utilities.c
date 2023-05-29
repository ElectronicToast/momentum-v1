/**
 * @file utilities.c
 * @brief Utility functions
 */


#include "utilities.h"
#include "hardware/rosc.h"


uint32_t rand_powof2(uint8_t n_bits) {
    uint32_t r = 0;
    for (int i = 0; i < n_bits; i++) {
        uint32_t rb = (0x0001 & rosc_hw->randombit);
        r = (r << 1) | rb;
    }
    return r;
}


uint32_t rand_powof2_range(uint8_t n_bits_min, uint8_t n_bits_max) {
    uint32_t r = 0;
    for (int i = 0; i < n_bits_max; i++) {
        uint32_t rb = (0x0001 & rosc_hw->randombit);
        r = (r << 1) | rb;
    }
    while (r > n_bits_min) {
        r >>= 1;
    }
    return r;
}
