#include "util_i2c.hpp"
#include "sens_aht21.hpp"
#include "sens_ens160.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" void app_main() { 
    static const char* TAG = "main";
    ESP_LOGI(TAG, "Starting Sensor Firmware");

    // 1. Initialize the shared I2C bus
    i2c_util::i2c_init();
    i2c_master_bus_handle_t bus = i2c_util::get_bus_handle();

    // 2. Initialize both sensors
    sens_aht21::AHT21 aht21;
    sens_ens160::ENS160 ens160;  // (0x53)
    sens_ens160::ENS160 ens160(true); //(0x52) ADD pin tied to VCC

    ESP_ERROR_CHECK(aht21.init(bus));
    ESP_ERROR_CHECK(ens160.init(bus));

    float temperature = 0.0f;
    float humidity = 0.0f;
    sens_ens160::Ens160Data gas_data;

    // NOTE: The ENS160 requires up to 3 minutes of warm-up time upon power-on 
    // before the eCO2 and TVOC readings become stable and accurate.
    ESP_LOGI(TAG, "Sensors initialized. Entering main loop...");

    while (true) {
        // Step A: Read temperature and humidity
        if (aht21.read(temperature, humidity) == ESP_OK) {
            ESP_LOGI(TAG, "AHT21  | Temp: %.2f C | Hum: %.2f %%", temperature, humidity);
            
            // Step B: Feed the ENS160 the fresh environmental data
            ens160.set_environment(temperature, humidity);
        } else {
            ESP_LOGW(TAG, "Failed to read AHT21");
        }

        // Step C: Read the compensated gas data
        esp_err_t ens_err = ens160.read_data(gas_data);
        if (ens_err == ESP_OK) {
            ESP_LOGI(TAG, "ENS160 | AQI: %d | TVOC: %d ppb | eCO2: %d ppm", 
                     gas_data.aqi, gas_data.tvoc, gas_data.eco2);
        } else if (ens_err == ESP_ERR_INVALID_STATE) {
            ESP_LOGD(TAG, "ENS160 data not ready yet, skipping this cycle.");
        } else {
            ESP_LOGW(TAG, "I2C error reading ENS160");
        }

        printf("--------------------------------------------------\n");
        vTaskDelay(pdMS_TO_TICKS(2000)); 
    }
}