#ifndef I2C_HAL_H
#define I2C_HAL_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    I2C_OK = 0,
    I2C_ERR_TIMEOUT = 1,
    I2C_ERR_NACK = 2,
    I2C_ERR_BUS = 3,
} I2C_Status_t;

void i2c_hal_init(void);
I2C_Status_t i2c_write_reg(uint8_t dev_addr, uint8_t reg, const uint8_t *data, size_t len);
I2C_Status_t i2c_read_reg (uint8_t dev_addr, uint8_t reg, uint8_t *data, size_t len);

#endif /* I2C_HAL_H */
