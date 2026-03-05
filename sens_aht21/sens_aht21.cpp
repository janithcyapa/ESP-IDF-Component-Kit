#include "sens_aht21.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace sens_aht21 {

static const char* TAG = "AHT21";

esp_err_t AHT21::init(i2c_master_bus_handle_t bus_handle) {
    if (bus_handle == nullptr) {
        ESP_LOGE(TAG, "Invalid I2C bus handle!");
        return ESP_ERR_INVALID_ARG;
    }

    // Configure the AHT21 device on the bus
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = AHT21_ADDR,
        .scl_speed_hz = 100000,
        .scl_wait_us = 0,
        .flags = {
            .disable_ack_check = false,
        }
    };

    // Attach the device to the master bus
    esp_err_t err = i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle_);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add AHT21 to I2C bus");
        return err;
    }

    // Optional but recommended: Wait 40ms after power-on before sending commands
    vTaskDelay(pdMS_TO_TICKS(40));

    ESP_LOGI(TAG, "AHT21 initialized successfully");
    return ESP_OK;
}

esp_err_t AHT21::read(float &temperature, float &humidity) {
    if (dev_handle_ == nullptr) return ESP_ERR_INVALID_STATE;

    // 1. Trigger measurement command: 0xAC, 0x33, 0x00
    uint8_t cmd[3] = {0xAC, 0x33, 0x00};
    esp_err_t err = i2c_master_transmit(dev_handle_, cmd, sizeof(cmd), -1);
    if (err != ESP_OK) return err;

    // 2. Wait for measurement to complete (AHT21 datasheet specifies ~80ms)
    vTaskDelay(pdMS_TO_TICKS(80));

    // 3. Read 7 bytes of data
    // Byte 0: State
    // Byte 1-3: Humidity
    // Byte 3-5: Temperature
    // Byte 6: CRC
    uint8_t data[7];
    err = i2c_master_receive(dev_handle_, data, sizeof(data), -1);
    if (err != ESP_OK) return err;

    // Check if the sensor is still busy (Bit 7 of State byte)
    if ((data[0] & 0x80) != 0) {
        ESP_LOGW(TAG, "Sensor is busy");
        return ESP_ERR_TIMEOUT;
    }

    // 4. Bitwise math to extract 20-bit raw values
    uint32_t h_raw = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | (data[3] >> 4);
    uint32_t t_raw = (((uint32_t)(data[3] & 0x0F)) << 16) | ((uint32_t)data[4] << 8) | data[5];

    // 5. Convert to physical values per datasheet formulas
    humidity = ((float)h_raw / 1048576.0f) * 100.0f;
    temperature = ((float)t_raw / 1048576.0f) * 200.0f - 50.0f;

    return ESP_OK;
}

} // namespace sens_aht21