#include "sens_ens160.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace sens_ens160 {

static const char* TAG = "ENS160";

// ENS160 Registers and Constants based on official driver
#define REG_OPMODE         0x10
#define REG_COMMAND        0x12
#define REG_TEMP_IN        0x13
#define REG_DATA_STATUS    0x20
#define REG_DATA_AQI       0x21

#define OPMODE_RESET       0xF0
#define OPMODE_IDLE        0x01
#define OPMODE_STD         0x02

#define COMMAND_NOP        0x00
#define COMMAND_CLRGPR     0xCC

#define STATUS_NEWDAT      0x02

esp_err_t ENS160::init(i2c_master_bus_handle_t bus_handle) {
    if (bus_handle == nullptr) return ESP_ERR_INVALID_ARG;

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = ENS160_ADDR,
        .scl_speed_hz = 100000,
        .scl_wait_us = 0,
        .flags = { .disable_ack_check = false }
    };

    esp_err_t err = i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle_);
    if (err != ESP_OK) return err;

    // --- OFFICIAL BOOT SEQUENCE ---

    // 1. Software Reset
    uint8_t cmd_reset[2] = { REG_OPMODE, OPMODE_RESET };
    i2c_master_transmit(dev_handle_, cmd_reset, sizeof(cmd_reset), -1);
    vTaskDelay(pdMS_TO_TICKS(15)); // Wait for boot

    // 2. Set to IDLE mode
    uint8_t cmd_idle[2] = { REG_OPMODE, OPMODE_IDLE };
    i2c_master_transmit(dev_handle_, cmd_idle, sizeof(cmd_idle), -1);
    vTaskDelay(pdMS_TO_TICKS(15));

    // 3. Clear Commands (NOP followed by CLRGPR)
    uint8_t cmd_nop[2] = { REG_COMMAND, COMMAND_NOP };
    i2c_master_transmit(dev_handle_, cmd_nop, sizeof(cmd_nop), -1);
    
    uint8_t cmd_clrgpr[2] = { REG_COMMAND, COMMAND_CLRGPR };
    i2c_master_transmit(dev_handle_, cmd_clrgpr, sizeof(cmd_clrgpr), -1);
    vTaskDelay(pdMS_TO_TICKS(15));

    // 4. Set to Standard Operating Mode
    uint8_t cmd_std[2] = { REG_OPMODE, OPMODE_STD };
    err = i2c_master_transmit(dev_handle_, cmd_std, sizeof(cmd_std), -1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set standard mode");
        return err;
    }

    ESP_LOGI(TAG, "ENS160 initialized successfully");
    return ESP_OK;
}

esp_err_t ENS160::set_environment(float temp_c, float rh_percent) {
    if (dev_handle_ == nullptr) return ESP_ERR_INVALID_STATE;

    // Formulas per official driver
    uint16_t t_data = (uint16_t)((temp_c + 273.15f) * 64.0f);
    uint16_t rh_data = (uint16_t)(rh_percent * 512.0f);

    // Official driver writes all 4 bytes sequentially starting at REG_TEMP_IN (0x13)
    uint8_t cmd[5];
    cmd[0] = REG_TEMP_IN;
    cmd[1] = (uint8_t)(t_data & 0xFF);          // Temp LSB
    cmd[2] = (uint8_t)((t_data >> 8) & 0xFF);   // Temp MSB
    cmd[3] = (uint8_t)(rh_data & 0xFF);         // RH LSB
    cmd[4] = (uint8_t)((rh_data >> 8) & 0xFF);  // RH MSB

    return i2c_master_transmit(dev_handle_, cmd, sizeof(cmd), -1);
}

esp_err_t ENS160::read_data(Ens160Data &data) {
    if (dev_handle_ == nullptr) return ESP_ERR_INVALID_STATE;

    // 1. Read the Status Register first
    uint8_t reg_status = REG_DATA_STATUS;
    uint8_t status = 0;
    esp_err_t err = i2c_master_transmit_receive(dev_handle_, &reg_status, 1, &status, 1, -1);
    if (err != ESP_OK) return err;

    // 2. Check the NEWDAT bit. If 0, the sensor hasn't finished measuring.
    if ((status & STATUS_NEWDAT) == 0) {
        return ESP_ERR_INVALID_STATE; // Data not ready
    }

    // 3. New data is ready! Read 7 bytes starting from AQI
    uint8_t reg_aqi = REG_DATA_AQI;
    uint8_t raw_data[7];
    err = i2c_master_transmit_receive(dev_handle_, &reg_aqi, 1, raw_data, 7, -1);
    if (err != ESP_OK) return err;

    // Map data based on official driver logic (Little Endian)
    data.aqi = raw_data[0];
    data.tvoc = raw_data[1] | ((uint16_t)raw_data[2] << 8);
    data.eco2 = raw_data[3] | ((uint16_t)raw_data[4] << 8);

    return ESP_OK;
}

} // namespace sens_ens160