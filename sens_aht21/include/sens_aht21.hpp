#pragma once

#include "driver/i2c_master.h"
#include "esp_err.h"

namespace sens_aht21 {

class AHT21 {
public:
    /**
     * @brief Initialize the AHT21 sensor and attach it to the I2C bus.
     * @param bus_handle The I2C master bus handle from util_i2c
     * @return ESP_OK on success
     */
    esp_err_t init(i2c_master_bus_handle_t bus_handle);

    /**
     * @brief Trigger a measurement and read the temperature and humidity.
     * @param temperature Reference to store the temperature in Celsius
     * @param humidity Reference to store the relative humidity percentage
     * @return ESP_OK on successful read
     */
    esp_err_t read(float &temperature, float &humidity);

private:
    i2c_master_dev_handle_t dev_handle_ = nullptr;
    const uint8_t AHT21_ADDR = 0x38;
};

} // namespace sens_aht21