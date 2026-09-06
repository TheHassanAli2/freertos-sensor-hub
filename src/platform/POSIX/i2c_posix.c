#include "i2c_hal.h"
#include "bme280.h"
#include <string.h>

// Mocking I2C HAL for POSIX platform. This implementation simulates a BME280 sensor with fixed calibration and data values.

// Calibration bytes, temperature will read ~25°C; pressure and humidity will read 0.
static const uint8_t sim_calib[24] = {
    0x70, 0x6B,   // dig_T1 = 27504
    0x43, 0x67,   // dig_T2 = 26435
    0xCE, 0xFF,   // dig_T3 = -50  
    0, 0, 0, 0, 0, 0, 0, 0,   // P1-P4
    0, 0, 0, 0, 0, 0, 0, 0,   // P5-P8
    0, 0,                      // P9
};

// Raw ADC bytes at 0xF7 — about ~25°C
static const uint8_t sim_data[8] = {
    0x65, 0x5A, 0xC0,   // pressure  (adc_P = 415148) 
    0x7F, 0x50, 0x00,   // temperature (adc_T = 519888)
    0x72, 0x80,         // humidity  (adc_H = 29312)  
};

void i2c_hal_init(void) {}

I2C_Status_t i2c_write_reg(uint8_t dev_addr, uint8_t reg, const uint8_t *data, size_t len){
    (void)dev_addr; (void)reg; (void)data; (void)len;
    return I2C_OK;
}

I2C_Status_t i2c_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *data, size_t len){
    if (dev_addr != BME280_I2C_ADDR)
        return I2C_ERR_NACK;

    switch (reg) {
        case 0xD0:   // CHIP_ID
            data[0] = 0x60;
            break;
        case 0x88:   // CALIB_MEAS
            memcpy(data, sim_calib, len < sizeof(sim_calib) ? len : sizeof(sim_calib));
            break;
        case 0xA1:   // dig_H1
            data[0] = 75;
            break;
        case 0xE1:   // dig_H2-H6
            memset(data, 0, len);
            break;
        case 0xF7:   // sensor data
            memcpy(data, sim_data, len < sizeof(sim_data) ? len : sizeof(sim_data));
            break;
        default:
            memset(data, 0, len);
            break;
    }
    return I2C_OK;
}
