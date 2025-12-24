#include "i2c_scanner.hpp"
#include <cstdio>

namespace i2c_scanner {

I2cScanner::I2cScanner(const Config& config) : port_(config.i2c_port) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = config.sda_io_num,
        .scl_io_num = config.scl_io_num,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {.clk_speed = config.clk_speed},
        .clk_flags = 0,
    };

    ESP_ERROR_CHECK(i2c_param_config(port_, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(port_, conf.mode, 0, 0, 0));

    ESP_LOGI(TAG, "I2C master initialized on port %d", port_);
}

I2cScanner::~I2cScanner() {
    i2c_driver_delete(port_);
    ESP_LOGI(TAG, "I2C master deinitialized");
}

void I2cScanner::scan() {
    printf("\n=== I2C Scanner ===\n");
    ESP_LOGI(TAG, "Scanning bus on port %d...", port_);

    int devices_found = 0;
    for (uint8_t addr = 1; addr < 127; ++addr) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);

        esp_err_t ret = i2c_master_cmd_begin(port_, cmd, pdMS_TO_TICKS(50));
        i2c_cmd_link_delete(cmd);

        if (ret == ESP_OK) {
            printf("  Found device at: 0x%02x\n", addr);
            ESP_LOGI(TAG, "Found device at 0x%02x", addr);
            devices_found++;
        }
    }

    if (devices_found == 0) {
        printf("  No I2C devices found.\n");
        ESP_LOGW(TAG, "No devices found");
    } else {
        printf("Scan complete. %d device(s) found.\n\n", devices_found);
    }
}

} // namespace i2c_scanner