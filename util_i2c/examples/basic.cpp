#include "util_i2c.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" void app_main() { 
    static const char* TAG = "main";

    ESP_LOGI(TAG, "Starting application");

    // 1. Initialize the I2C bus using the new driver
    i2c_util::i2c_init();

    // 2. Perform an initial scan
    i2c_util::i2c_scan();

    // 3. Keep scanning every 10 seconds
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        i2c_util::i2c_scan();
    }
}