#include "util_i2c.hpp"
#include "sens_aht21.hpp"
#include "sens_ens160.hpp"
#include "sens_bme280.hpp" //
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" void app_main() { 
    static const char* TAG = "main";

    // 1. Initialize shared I2C bus
    i2c_util::i2c_init(); 
    i2c_master_bus_handle_t bus = i2c_util::get_bus_handle();

    // 2. Instantiate sensor
    sens_bme280::BME280 bme280; 
    // 3. Initialize devices
    ESP_ERROR_CHECK(bme280.init(bus));

    sens_bme280::Bme280Data bme_data;

    while (true) {
 

        // Read BME280
        if (bme280.read_data(bme_data) == ESP_OK) {
        ESP_LOGI(TAG, "BME280 | Temp: %.2f C | Hum: %.2f %% | Pres: %.2f hPa", 
                bme_data.temperature, 
                bme_data.humidity, 
                bme_data.pressure);
        }

        printf("--------------------------------------------------\n");
        vTaskDelay(pdMS_TO_TICKS(2000)); 
    }
}