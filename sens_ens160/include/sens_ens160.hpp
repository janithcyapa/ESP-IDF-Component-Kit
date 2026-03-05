#pragma once

#include "driver/i2c_master.h"
#include "esp_err.h"

namespace sens_ens160 {

struct Ens160Data {
    uint8_t aqi;     // Air Quality Index (1-5)
    uint16_t tvoc;   // Total Volatile Organic Compounds (ppb)
    uint16_t eco2;   // Equivalent CO2 (ppm)
};

class ENS160 {
public:
    /**
     * @brief Initialize the ENS160 using the official boot sequence.
     * @param bus_handle The I2C master bus handle from util_i2c
     * @return ESP_OK on success
     */
    esp_err_t init(i2c_master_bus_handle_t bus_handle);

    /**
     * @brief Feed temperature and humidity to the sensor for accurate compensation.
     * @param temp_c Temperature in Celsius
     * @param rh_percent Relative Humidity percentage
     * @return ESP_OK on success
     */
    esp_err_t set_environment(float temp_c, float rh_percent);

    /**
     * @brief Read the compensated gas data.
     * @param data Reference to the struct where data will be stored
     * @return ESP_OK on success, ESP_ERR_INVALID_STATE if data is not ready
     */
    esp_err_t read_data(Ens160Data &data);

private:
    i2c_master_dev_handle_t dev_handle_ = nullptr;
    const uint8_t ENS160_ADDR = 0x53; 
};

} // namespace sens_ens160