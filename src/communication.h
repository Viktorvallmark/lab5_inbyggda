#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdint.h>

/**
 * Initialises I2C bus, uses controller 0.
 * sdapin = SDA GPIO, sclpin = SCL GPIO
 */
void initI2C(int sdapin, int sclpin);

/**
 * Writes one byte into a register.
 */
void writeI2C(uint8_t address, uint8_t reg, uint8_t data);

/**
 * Reads len bytes and places them into a buffer.
 * buffer must be pre-allocated by the caller.
 */
void readI2C(uint8_t address, uint8_t reg, uint8_t *buffer, int len);

#endif
