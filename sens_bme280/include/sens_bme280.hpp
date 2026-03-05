void func(void);
#pragma once

#include "driver/i2c_master.h"
#include "esp_err.h"

namespace sens_bme280 {

struct Bme280Data {
    float temperature; // °C
    float humidity;    // %
    float pressure;    // hPa
};

class BME280 {
public:
    /**
     * @param is_alt_addr Set to true if SDO pin is high (0x77), false for default (0x76).
     */
    BME280(bool is_alt_addr = false) : sensor_addr_(is_alt_addr ? 0x77 : 0x76) {}

    esp_err_t init(i2c_master_bus_handle_t bus_handle);
    esp_err_t read_data(Bme280Data &data);

private:
    i2c_master_dev_handle_t dev_handle_ = nullptr;
    uint8_t sensor_addr_;
    
    // Internal compensation parameters
    struct {
        uint16_t dig_T1; int16_t dig_T2, dig_T3;
        uint16_t dig_P1; int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
        uint8_t  dig_H1; int16_t dig_H2; uint8_t dig_H3; int16_t dig_H4, dig_H5; int8_t dig_H6;
    } calib_;

    esp_err_t read_calibration();
    int32_t t_fine; // Required for pressure/humidity compensation
};

} // namespace sens_bme280