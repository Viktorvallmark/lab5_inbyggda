#include "accelerometer.h"
#include "communication.h"

#define MPU6050_PWR_MGMT_1   0x6B
#define MPU6050_SMPLRT_DIV   0x19
#define MPU6050_ACCEL_XOUT_H 0x3B

#define ACCEL_SCALE 16384.0f

void initAccelerometer(void)
{
    writeI2C(MPU6050_I2C_ADDR, MPU6050_PWR_MGMT_1, 0x00);

    writeI2C(MPU6050_I2C_ADDR, MPU6050_SMPLRT_DIV, 250);
}

acceleration_raw_t readAccelerationRaw(void)
{
    uint8_t buffer[6];

    // XOUT_H, XOUT_L, YOUT_H, YOUT_L, ZOUT_H, ZOUT_L
    readI2C(MPU6050_I2C_ADDR, MPU6050_ACCEL_XOUT_H, buffer, sizeof(buffer));

    acceleration_raw_t raw;

    raw.x = (int16_t)((buffer[0] << 8) | buffer[1]);
    raw.y = (int16_t)((buffer[2] << 8) | buffer[3]);
    raw.z = (int16_t)((buffer[4] << 8) | buffer[5]);

    return raw;
}

acceleration_t readAcceleration(void)
{
    acceleration_raw_t raw = readAccelerationRaw();

    acceleration_t acc;

    acc.x = ((float)raw.x / ACCEL_SCALE);
    acc.y = ((float)raw.y / ACCEL_SCALE);
    acc.z = ((float)raw.z / ACCEL_SCALE);

    return acc;
}
