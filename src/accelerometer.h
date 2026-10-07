#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <stdint.h>

// MPU6050 I2C address when AD0 is connected to GND.
// If AD0 is high, use 0x69.
#define MPU6050_I2C_ADDR 0x68

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} acceleration_raw_t;

typedef struct {
    float x;
    float y;
    float z;
} acceleration_t;

/**
 * Initialises the MPU6050.
 * Must be called after initI2C().
 */
void initAccelerometer(void);

/**
 * Returns raw signed accelerometer values.
 */
acceleration_raw_t readAccelerationRaw(void);

/**
 * Returns acceleration in m/s^2 for all 3 axes.
 */
acceleration_t readAcceleration(void);

#endif
