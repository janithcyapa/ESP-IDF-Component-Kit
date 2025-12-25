#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

// Using the built-in enum name is the safest way in C++
#define BLINK_GPIO GPIO_NUM_2 

static const char *TAG = "BlinkExample";

extern "C" void app_main(void)
{
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    int led_state = 0;

    while (1) {
        led_state = !led_state;
        gpio_set_level(BLINK_GPIO, (uint32_t)led_state);

        ESP_LOGI(TAG, "LED state: %s", led_state ? "ON" : "OFF");

        vTaskDelay(pdMS_TO_TICKS(1000)); // More idiomatic way to handle delay
    }
}