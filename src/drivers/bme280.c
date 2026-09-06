    #include "BME280.h"

    #define BME280_REG_ADDR 0xF7
    #define CHIP_ID_REG_ADDR 0xD0
    #define CTRL_HUM_REG_ADDR 0xF2
    #define CTRL_MEAS_REG_ADDR 0xF4
    #define CONFIG_REG_ADDR 0xF5
    #define CHIP_ID 0x60

    //Calib registers
    #define CALIB_MEAS_REG_ADDR 0x88
    #define CALIB_HUM_H1_REG_ADDR 0xA1
    #define CALIB_HUM_H2_H6_REG_ADDR 0xE1

    // Calibration data structure from BME280 datasheet, section 4.2.2 "Calibration parameters"
    typedef struct{
        uint16_t dig_T1; 
        int16_t dig_T2; 
        int16_t dig_T3;
        uint16_t dig_P1;
        int16_t dig_P2;
        int16_t dig_P3;
        int16_t dig_P4;
        int16_t dig_P5;
        int16_t dig_P6;
        int16_t dig_P7;
        int16_t dig_P8;
        int16_t dig_P9;
        uint8_t dig_H1;
        int16_t dig_H2;
        uint8_t dig_H3;
        int16_t dig_H4;
        int16_t dig_H5;
        int8_t dig_H6;
    } Calibration_data_t;

    static Calibration_data_t calib_data;
    // t_fine carries fine temperature as global value
    static int32_t t_fine;

    static BME_Status_t load_calibration(void){
        uint8_t m_buf[24];
        if (i2c_read_reg (BME280_I2C_ADDR, CALIB_MEAS_REG_ADDR, m_buf, 24) != I2C_OK) {
            return BME_ERR_READ;
        }

        //Datasheet says this is stored as little endian 16-bit, so should be read as low byte first, then high byte.
        calib_data.dig_T1 = (uint16_t)(m_buf[1] << 8 | m_buf[0]);
        calib_data.dig_T2 = (int16_t)(m_buf[3] << 8 | m_buf[2]);
        calib_data.dig_T3 = (int16_t)(m_buf[5] << 8 | m_buf[4]);

        calib_data.dig_P1 = (uint16_t)(m_buf[7] << 8 | m_buf[6]);
        calib_data.dig_P2 = (int16_t)(m_buf[9] << 8 | m_buf[8]);
        calib_data.dig_P3 = (int16_t)(m_buf[11] << 8 | m_buf[10]);
        calib_data.dig_P4 = (int16_t)(m_buf[13] << 8 | m_buf[12]);
        calib_data.dig_P5 = (int16_t)(m_buf[15] << 8 | m_buf[14]);
        calib_data.dig_P6 = (int16_t)(m_buf[17] << 8 | m_buf[16]);
        calib_data.dig_P7 = (int16_t)(m_buf[19] << 8 | m_buf[18]);
        calib_data.dig_P8 = (int16_t)(m_buf[21] << 8 | m_buf[20]);
        calib_data.dig_P9 = (int16_t)(m_buf[23] << 8 | m_buf[22]);

        uint8_t h1[1];
        uint8_t h_buf[7];
        if (i2c_read_reg (BME280_I2C_ADDR, CALIB_HUM_H1_REG_ADDR, h1, 1) != I2C_OK) {
            return BME_ERR_READ;
        }
        if (i2c_read_reg (BME280_I2C_ADDR, CALIB_HUM_H2_H6_REG_ADDR, h_buf, 7) != I2C_OK) {
            return BME_ERR_READ;
        }

        calib_data.dig_H1 = (uint8_t)(h1[0]);
        calib_data.dig_H2 = (int16_t)(h_buf[1] << 8 | h_buf[0]);
        calib_data.dig_H3 = (uint8_t)(h_buf[2]);
        calib_data.dig_H4 = (int16_t)((h_buf[3] << 4) | (h_buf[4] & 0x0F));
        calib_data.dig_H5 = (int16_t)((h_buf[5] << 4) | (h_buf[4] >> 4));
        calib_data.dig_H6 = (int8_t)(h_buf[6]);


        return BME_OK;
    }

    BME_Status_t bme280_init(void){
        uint8_t chip_id;
        if (i2c_read_reg (BME280_I2C_ADDR, CHIP_ID_REG_ADDR, &chip_id, sizeof(chip_id)) != I2C_OK) {
            return BME_ERR_INIT;
        }
        if (chip_id != CHIP_ID) {
            return BME_ERR_INIT;
        }

        uint8_t ctrl_hum = 0x01;
        uint8_t ctrl_meas = 0xB7;
        uint8_t config = 0xA0;
        if (i2c_write_reg(BME280_I2C_ADDR, CTRL_HUM_REG_ADDR, &ctrl_hum, sizeof(ctrl_hum)) != I2C_OK) {
            return BME_ERR_INIT;
        }
        if (i2c_write_reg(BME280_I2C_ADDR, CTRL_MEAS_REG_ADDR, &ctrl_meas, sizeof(ctrl_meas)) != I2C_OK) {
            return BME_ERR_INIT;
        }
        if (i2c_write_reg(BME280_I2C_ADDR, CONFIG_REG_ADDR, &config, sizeof(config)) != I2C_OK) {
            return BME_ERR_INIT;
        }

        if (load_calibration() != BME_OK) {
            return BME_ERR_INIT;
        }

        return BME_OK;
    }

    // These are copied from the BME280 datasheet, section 4.2.3 "Compensation formula".
    // Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23   DegC.
    static int32_t BME280_compensate_T_int32(int32_t adc_T) {
        int32_t var1, var2, T;
        var1 = ((((adc_T>>3) - ((int32_t)calib_data.dig_T1<<1))) * ((int32_t)calib_data.dig_T2)) >> 11;
        var2 = (((((adc_T>>4) - ((int32_t)calib_data.dig_T1)) * ((adc_T>>4) - ((int32_t)calib_data.dig_T1))) >> 12) * ((int32_t)calib_data.dig_T3)) >> 14;
        t_fine = var1 + var2;
        T = (t_fine * 5 + 128) >> 8;
        return T;
    }

    // Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
    // Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
    static uint32_t BME280_compensate_P_int64(int32_t adc_P)
    {
        int64_t var1, var2, p;
        var1 = ((int64_t)t_fine) - 128000;
        var2 = var1 * var1 * (int64_t)calib_data.dig_P6;
        var2 = var2 + ((var1*(int64_t)calib_data.dig_P5)<<17);
        var2 = var2 + (((int64_t)calib_data.dig_P4)<<35);
        var1 = ((var1 * var1 * (int64_t)calib_data.dig_P3)>>8) + ((var1 * (int64_t)calib_data.dig_P2)<<12);
        var1 = (((((int64_t)1)<<47)+var1))*((int64_t)calib_data.dig_P1)>>33;
        if (var1 == 0){
            return 0; // avoid exception caused by division by zero
        }
        p = 1048576-adc_P;
        p = (((p<<31)-var2)*3125)/var1;
        var1 = (((int64_t)calib_data.dig_P9) * (p>>13) * (p>>13)) >> 25;
        var2 = (((int64_t)calib_data.dig_P8) * p) >> 19;
        p = ((p + var1 + var2) >> 8) + (((int64_t)calib_data.dig_P7)<<4);
        return (uint32_t)p;
    }
    
    // Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits).
    // Output value of “47445” represents 47445/1024 = 46.333 %RH
    static uint32_t bme280_compensate_H_int32(int32_t adc_H)
    {
        int32_t v_x1_u32r;

        v_x1_u32r = (t_fine - ((int32_t)76800));
        v_x1_u32r = (((((adc_H << 14) - (((int32_t)calib_data.dig_H4) << 20) - (((int32_t)calib_data.dig_H5) *
        v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r *
        ((int32_t)calib_data.dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)calib_data.dig_H3)) >> 11) +
        ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)calib_data.dig_H2) +
        8192) >> 14));
        v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
        ((int32_t)calib_data.dig_H1)) >> 4));
        v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
        v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
        return (uint32_t)(v_x1_u32r>>12);
    }

    BME_Status_t bme280_read(Sample_t *sample){
        uint8_t data[8];
        if(i2c_read_reg(BME280_I2C_ADDR, BME280_REG_ADDR, data, 8) != I2C_OK){
            return BME_ERR_READ;
        }

        // BME280 has format as 3 bytes for pressure, 3 bytes for temperature, and 2 bytes for humidity
        // need to shift data[0] left 12 bits, then shift data[1] left 4 bits, shift data[2] right 4 bits,and then OR the 3 bytes together to get a 20-bit value.

        int32_t pressure_adc = (int32_t)(data[0] << 12 | data[1] << 4 | data[2] >> 4);
        int32_t temperature_adc = (int32_t)(data[3] << 12 | data[4] << 4 | data[5] >> 4);
        uint32_t humidity_adc = (uint32_t)(data[6] << 8 | data[7]);
        sample->temperature = BME280_compensate_T_int32(temperature_adc);
        sample->pressure = BME280_compensate_P_int64(pressure_adc);
        sample->humidity = bme280_compensate_H_int32(humidity_adc);

        return BME_OK;
    }
