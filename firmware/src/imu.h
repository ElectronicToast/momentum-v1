/**
 * @file imu.h
 * @brief IMU driver header file
 */

#ifndef _IMU_H_
#define _IMU_H_


#define MPU6050_REG_ACCEL_CONFIG          0x1c
#define MPU6050_REG_MOT_THR               0x1f
#define MPU6050_REG_MOT_DUR               0x20
#define MPU6050_REG_ZRMOT_THR             0x21
#define MPU6050_REG_ZRMOT_DUR             0x22
#define MPU6050_REG_INT_PIN_CFG           0x37
#define MPU6050_REG_INT_ENABLE            0x38
#define MPU6050_REG_INT_STATUS            0x3a
#define MPU6050_REG_MOT_DETECT_STATUS     0x61
#define MPU6050_REF_MOT_DETECT_CTRL       0x69
#define MPU6050_REG_PWR_MGMT_1            0x6B
#define MPU6050_REG_SIGNAL_PATH_RESET     0x68
#define MPU6050_REG_WHOAMI                0x75


#define IMU_N_RESET_TIMEOUT             10      // Try this many times to reset
#define IMU_N_RESET_DELAY_MS            10      // Delay in ms


void imu_i2c_init();
void imu_reset();
void imu_configure_interrupt();
void imu_goto_sleep();
void imu_wake_up();

bool imu_has_clash();
bool imu_has_swing();

void imu_clear_clash();
void imu_clear_swing();
void imu_clear_motion();

#endif /* _IMU_H_ */

