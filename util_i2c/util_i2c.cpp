#include "util_i2c.hpp"
#include <cstdio>

namespace i2c_util {

static const char* TAG = "i2c_util";

void i2c_init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = { .clk_speed = I2C_MASTER_FREQ },
        .clk_flags = 0,
    };

    // Configure and Install
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0));

    ESP_LOGI(TAG, "I2C Initialized (Port: %d, SDA: %d, SCL: %d)", 
             I2C_MASTER_NUM, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO);
}

void i2c_scan() {
    printf("\n>> Scanning I2C Bus %d <<\n", I2C_MASTER_NUM);
    int devices_found = 0;

    // Standard 7-bit address range
    for (uint8_t addr = 1; addr < 127; ++addr) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);

        // Execute with timeout
        esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(50));
        i2c_cmd_link_delete(cmd);

        if (ret == ESP_OK) {
            printf(" - Device found at address: 0x%02x\n", addr);
            devices_found++;
        }
    }
    printf(">> Scan Complete: %d device(s) found <<\n\n", devices_found);
}

} // namespace i2c_util