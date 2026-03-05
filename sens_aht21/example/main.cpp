#include "util_i2c.hpp"
#include "sens_aht21.hpp" // <-- Include the new component
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" void app_main() { 
    static const char* TAG = "main";

    ESP_LOGI(TAG, "Starting AHU Firmware");

    i2c_util::i2c_init();
    
    sens_aht21::AHT21 aht21;
    esp_err_t err = aht21.init(i2c_util::get_bus_handle());
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "AHT21 initialization failed! Halting.");
        return;
    }

    float temperature = 0.0f;
    float humidity = 0.0f;

    // 3. Main reading loop
    while (true) {
        if (aht21.read(temperature, humidity) == ESP_OK) {
            ESP_LOGI(TAG, "Temp: %.2f °C | Humidity: %.2f %%", temperature, humidity);
        } else {
            ESP_LOGW(TAG, "Failed to read from AHT21");
        }

        vTaskDelay(pdMS_TO_TICKS(2000)); // Read every 2 seconds
    }
}