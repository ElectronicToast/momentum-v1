/**
 * @file imu.c
 * @brief IMU driver
 */ 


#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/sync.h"
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "pinmap.h"
#include "imu.h"


#define I2C_IMU_INST i2c0


// Flags for application code to handle at leisure
volatile bool __has_clash = false;
volatile bool __has_swing = false;


static void imu_gpio_handler() {
    // Acknowledge GPIO interrupt
    gpio_acknowledge_irq(PIN_IMU_INT, GPIO_IRQ_EDGE_FALL);

    // Acknowledge interrupt by writing to IMU_I2C_REG_INT_STATUS
    // Should probably not do this in the ISR but rather via flag
    uint8_t buf = MPU6050_REG_INT_STATUS;
    uint8_t intmask;
    i2c_write_blocking(I2C_IMU_INST, IMU_I2C_ADDR, &buf, 1, true);
    i2c_read_blocking(I2C_IMU_INST, IMU_I2C_ADDR, &intmask, 1, true);

    // Bit 6 is motion detect, bit 5 is zero-motion detect
    // Prioritize clash over swing if intmask has both bits set
    if (intmask & 0x40) {
        __has_clash = true;
        //printf("Clash detected!\n");
    } 
    else if (intmask & 0x20) {
        // Read bit 0 of MOT_DETECT_STATUS to see whether it was a motion or zero-motion interrupt
        buf = MPU6050_REG_MOT_DETECT_STATUS;
        uint8_t motion_status;
        i2c_write_blocking(I2C_IMU_INST, IMU_I2C_ADDR, &buf, 1, true);
        i2c_read_blocking(I2C_IMU_INST, IMU_I2C_ADDR, &motion_status, 1, true);
        //printf("MOT_DETECT_STATUS: 0x%02x\n", motion_status);
        //printf("MOT_ZRMOT: %d\n", motion_status & 0x01);

        if (!(motion_status & 0x01)) {
            __has_swing = true;
            //printf("Swing detected!\n");
        }
    }

    //int16_t acceleration[3], gyro[3], temp;
    //mpu6050_read_raw(acceleration, gyro, &temp);

    //printf("Acc. X = %d, Y = %d, Z = %d\n", acceleration[0], acceleration[1], acceleration[2]);
    //printf("Gyro. X = %d, Y = %d, Z = %d\n", gyro[0], gyro[1], gyro[2]);
    //printf("Temp. = %f\n", (temp / 340.0) + 36.53);
}


void imu_i2c_init() {
    i2c_init(I2C_IMU_INST, IMU_I2C_CLK_FREQ_KHZ * 1000);
    gpio_set_function(PIN_IMU_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_IMU_SCL, GPIO_FUNC_I2C);
    // Unnecessary, there are external PU
    //gpio_pull_up(PIN_IMU_SDA);
    //gpio_pull_up(PIN_IMU_SCL);
}


void imu_reset() {
    uint8_t buf[] = {MPU6050_REG_PWR_MGMT_1, 0x80};
    i2c_write_blocking(I2C_IMU_INST, IMU_I2C_ADDR, buf, 2, false);

    // Wait until IMU_I2C_REG_PWR_MGMT_1[7] is cleared
    uint8_t reset;
    uint8_t reset_counter = 0;
    do {
        i2c_write_blocking(I2C_IMU_INST, IMU_I2C_ADDR, &buf[0], 1, true);
        i2c_read_blocking(I2C_IMU_INST, IMU_I2C_ADDR, &reset, 1, false);
        reset_counter++;
        sleep_ms(IMU_N_RESET_DELAY_MS);
        //printf("Reset: 0x%02x\n", reset);
    } while ((reset & 0x80) && (reset_counter < IMU_N_RESET_TIMEOUT));

    // Clear the sleep bit, IMU_I2C_REG_PWR_MGMT_1[6]
    // Oddly even though the prinf above shows the bit is cleared, it isn't
    // and this is needed to get the IMU to work
    buf[1] = reset & ~0x40;
    i2c_write_blocking(I2C_IMU_INST, IMU_I2C_ADDR, buf, 2, false);
    i2c_read_blocking(I2C_IMU_INST, IMU_I2C_ADDR, &reset, 1, false);
    //printf("Reset (after clear SLEEP): 0x%02x\n", reset);

    // Wait a bit as recommended in the register map for SPI mode
    sleep_ms(100);

    // Signal path reset
    buf[0] = MPU6050_REG_SIGNAL_PATH_RESET;
    buf[1] = 0x07;
    i2c_write_blocking(I2C_IMU_INST, IMU_I2C_ADDR, buf, 2, false);

    // Wait a bit as recommended in the register map for SPI mode
    sleep_ms(100);
}


void imu_configure_interrupt() {
    // Active low push pull, latching interrupt signal
    uint8_t buf[] = {MPU6050_REG_INT_PIN_CFG, 0x20};
    i2c_write_blocking(I2C_IMU_INST, IMU_I2C_ADDR, buf, 2, false);

    // Set a 5 Hz digital HPF (lower 3 bits in ACCEL_CONFIG)
    buf[0] = MPU6050_REG_ACCEL_CONFIG;
    buf[1] = 0x01;
    i2c_write_blocking(I2C_IMU_INST, IMU_I2C_ADDR, buf, 2, false);

    // Set motion threshold (8-bit unsigned)
    buf[0] = MPU6050_REG_MOT_THR;
    buf[1] = IMU_THRESH_CLASH;
    i2c_write_blocking(I2C_IMU_INST, IMU_I2C_ADDR, buf, 2, false);

    // Set motion duration in milliseconds
    buf[0] = MPU6050_REG_MOT_DUR;
    buf[1] = IMU_N_CLASH_DUR;
    i2c_write_blocking(I2C_IMU_INST, IMU_I2C_ADDR, buf, 2, false);

    // -------------------------------------------------------------------------
    // Also configure a zero-motion interrupt
    // Set zero-motion threshold (8-bit unsigned)
    buf[0] = MPU6050_REG_ZRMOT_THR;
    buf[1] = IMU_THRESH_SWING;
    i2c_write_blocking(I2C_IMU_INST, IMU_I2C_ADDR, buf, 2, false);

    // Set zero-motion duration in units of 64 milliseconds
    buf[0] = MPU6050_REG_ZRMOT_DUR;
    buf[1] = IMU_N_SWING_DUR;
    i2c_write_blocking(I2C_IMU_INST, IMU_I2C_ADDR, buf, 2, false);
    // -------------------------------------------------------------------------

    // Configure other motion detection settings
    // Upper nibble - 1 ms extra power on delay
    // Lower nibble - free fall and motion detect decrement count rate of 1ms
    buf[0] = MPU6050_REF_MOT_DETECT_CTRL;
    buf[1] = 0x15;
    i2c_write_blocking(I2C_IMU_INST, IMU_I2C_ADDR, buf, 2, false);

    // Enable motion detection interrupt (bit 6) and zero-motion detection interrupt (bit 5)
    buf[0] = MPU6050_REG_INT_ENABLE;
    buf[1] = 0x60;
    i2c_write_blocking(I2C_IMU_INST, IMU_I2C_ADDR, buf, 2, false);

    // Make INT pin active low
    buf[0] = MPU6050_REG_INT_PIN_CFG;
    buf[1] = 0xa0;
    i2c_write_blocking(I2C_IMU_INST, IMU_I2C_ADDR, buf, 2, false);

    // Configure interrupt pin
    gpio_init(PIN_IMU_INT);
    gpio_set_dir(PIN_IMU_INT, GPIO_IN);
    gpio_set_irq_enabled_with_callback(PIN_IMU_INT, GPIO_IRQ_EDGE_FALL, true, &imu_gpio_handler);
}


inline void imu_goto_sleep() {
    // Set sleep bit in PWR_MGMT_1
    uint8_t buf[] = {MPU6050_REG_PWR_MGMT_1, 0x40};
    i2c_write_blocking(I2C_IMU_INST, IMU_I2C_ADDR, buf, 2, false);
}

inline void imu_wake_up() {
    // Clear sleep bit in PWR_MGMT_1
    uint8_t buf[] = {MPU6050_REG_PWR_MGMT_1, 0x00};
    i2c_write_blocking(I2C_IMU_INST, IMU_I2C_ADDR, &buf[0], 1, true);
    i2c_read_blocking(I2C_IMU_INST, IMU_I2C_ADDR, &buf[1], 1, false);
    buf[1] = buf[1] & ~0x40;
    i2c_write_blocking(I2C_IMU_INST, IMU_I2C_ADDR, buf, 2, false);
    return;
}


inline bool imu_has_clash() {
    bool flag = false;
    uint32_t status = save_and_disable_interrupts();
    if (__has_clash) {
        __has_clash = false;
        flag = true;
    }
    restore_interrupts(status);
    return flag;
}

inline bool imu_has_swing() {
    bool flag = false;
    uint32_t status = save_and_disable_interrupts();
    if (__has_swing) {
        __has_swing = false;
        flag = true;
    }
    restore_interrupts(status);
    return flag;
}

inline void imu_clear_clash() {
    __has_clash = false;
}

inline void imu_clear_swing() {
    __has_swing = false;
}

inline void imu_clear_motion() {
    __has_clash = false;
    __has_swing = false;
}
