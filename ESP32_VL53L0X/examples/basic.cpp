#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

extern "C" {
    #include "driver/i2c.h"
    #include "vl53l0x.h"
}
// Your configuration
#define I2C_PORT        I2C_NUM_0
#define I2C_SDA_GPIO    GPIO_NUM_21
#define I2C_SCL_GPIO    GPIO_NUM_22
#define XSHUT_GPIO      -1
#define VL53L0X_ADDRESS 0x29

static const char *TAG = "Scorpion";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Starting Scorpion");
    // 1. Configure the sensor
    vl53l0x_t *sensor = vl53l0x_config(
        I2C_PORT,
        I2C_SCL_GPIO,
        I2C_SDA_GPIO,
        XSHUT_GPIO,
        VL53L0X_ADDRESS,
        1
    );
    
    if (!sensor) {
        ESP_LOGE(TAG, "Failed to configure VL53L0X!");
        vTaskDelay(pdMS_TO_TICKS(1000));
        return;
    }
    
    // 2. Initialize the sensor
    const char *error = vl53l0x_init(sensor);
    if (error) {
        ESP_LOGE(TAG, "VL53L0X initialization failed: %s", error);
        vl53l0x_end(sensor);
        return;
    }
    
    ESP_LOGI(TAG, "VL53L0X initialized successfully!");
    
    // 3. Configure timing budget for better performance
    error = vl53l0x_setMeasurementTimingBudget(sensor, 200000); // 200ms timing budget
    if (error) {
        ESP_LOGW(TAG, "Failed to set timing budget: %s", error);
    }
    
    // 4. Set timeout for measurements
    vl53l0x_setTimeout(sensor, 500); // 500ms timeout
    
    // 5. Start continuous measurements
    vl53l0x_startContinuous(sensor, 0); // 0 = back-to-back mode (fastest)
    
    ESP_LOGI(TAG, "Starting distance measurements...");
    
    // 6. Main measurement loop
    while (1) {
        // Read distance in millimeters
        uint16_t distance = vl53l0x_readRangeContinuousMillimeters(sensor);
        
        if (vl53l0x_timeoutOccurred(sensor)) {
            ESP_LOGW(TAG, "Measurement timeout occurred");
        } else if (distance == 65535) {
            ESP_LOGW(TAG, "Measurement out of range");
        } else {
            // Valid measurement
            if (distance < 1200) {
                ESP_LOGI(TAG, "Distance: %d", distance);
            } else {
                ESP_LOGI(TAG, "Distance: > 1200 mm (out of optimal range)");
            }
        }
        
        // Check for I2C communication errors
        if (vl53l0x_i2cFail(sensor)) {
            ESP_LOGE(TAG, "I2C communication error detected!");
        }
        
        // Wait before next measurement
        vTaskDelay(pdMS_TO_TICKS(10)); // 100ms delay
    }
    
    // 7. Cleanup (this won't be reached in this example)
    vl53l0x_stopContinuous(sensor);
    vl53l0x_end(sensor);
}