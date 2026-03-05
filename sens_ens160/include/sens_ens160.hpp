#pragma once

#include "driver/i2c_master.h"
#include "esp_err.h"

namespace sens_ens160 {

struct Ens160Data {
    uint8_t aqi;
    uint16_t tvoc;
    uint16_t eco2;
};

class ENS160 {
public:
    /**
     * @brief Constructor for ENS160.
     * @param is_alt_addr Set to true if ADD pin is connected to GND (Address 0x52). 
     * Defaults to false (Address 0x53).
     */
    ENS160(bool is_alt_addr = false) {
        ENS160_ADDR = is_alt_addr ? 0x52 : 0x53;
    }

    esp_err_t init(i2c_master_bus_handle_t bus_handle);
    esp_err_t set_environment(float temp_c, float rh_percent);
    esp_err_t read_data(Ens160Data &data);

private:
    i2c_master_dev_handle_t dev_handle_ = nullptr;
    uint8_t ENS160_ADDR; 
};

} // namespace sens_ens160