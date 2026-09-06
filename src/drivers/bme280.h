#ifndef BME280_H
#define BME280_H

#include "i2c_hal.h"
#include "sensor_hub.h"

#define BME280_I2C_ADDR 0x76

typedef enum {
    BME_OK = 0,
    BME_ERR_INIT = 1,
    BME_ERR_READ = 2
} BME_Status_t;

BME_Status_t bme280_init(void);
BME_Status_t bme280_read(Sample_t *sample);


#endif // BME280_H