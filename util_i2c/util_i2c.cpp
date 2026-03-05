#include "util_i2c.hpp"
#include <cstdio>

namespace i2c_util {

static const char* TAG = "i2c_util";

// Store the handle globally within this namespace so other files can request it
static i2c_master_bus_handle_t bus_handle = NULL; 

void i2c_init() {
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags = {
            .enable_internal_pullup = true,
        }
    };

    // Initialize the new master bus
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    ESP_LOGI(TAG, "I2C Master Bus Initialized (Port: %d, SDA: %d, SCL: %d)", 
             I2C_MASTER_NUM, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO);
}

void i2c_scan() {
    if (bus_handle == NULL) {
        ESP_LOGE(TAG, "I2C bus not initialized! Call i2c_init() first.");
        return;
    }

    printf("\n>> Scanning I2C Bus %d <<\n", I2C_MASTER_NUM);
    int devices_found = 0;

    // Standard 7-bit address range
    for (uint16_t addr = 1; addr < 127; ++addr) {
        // The new API has a built-in probe function! 
        esp_err_t ret = i2c_master_probe(bus_handle, addr, 50);
        
        if (ret == ESP_OK) {
            printf(" - Device found at address: 0x%02x\n", addr);
            devices_found++;
        }
    }
    printf(">> Scan Complete: %d device(s) found <<\n\n", devices_found);
}

i2c_master_bus_handle_t get_bus_handle() {
    return bus_handle;
}

} // namespace i2c_util