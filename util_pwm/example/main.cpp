#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "util_PWM.hpp"

// 1. Create instances with descriptive names
// Parameters: (GPIO, CHANNEL, TIMER)
PwmInstance shoulder_servo(18, LEDC_CHANNEL_0, LEDC_TIMER_0);
PwmInstance elbow_servo(19, LEDC_CHANNEL_1, LEDC_TIMER_0);
PwmInstance wheels_motor(21, LEDC_CHANNEL_2, LEDC_TIMER_0);
PwmInstance status_led(2, LEDC_CHANNEL_3, LEDC_TIMER_1); // Use Timer 1 for different frequency

extern "C" void app_main(void) {
    // 2. Initialize them
    shoulder_servo.init(50); 
    elbow_servo.init(50);
    wheels_motor.init(50);
    status_led.init(5000); // LED at 5kHz

    while (1) {
        // 3. Use them naturally by name
        shoulder_servo.set_angle_180(90);
        elbow_servo.set_angle_270(135);
        
        // Drive forward at 50% speed
        wheels_motor.set_speed_360(50);
        
        vTaskDelay(pdMS_TO_TICKS(2000));

        shoulder_servo.set_angle_180(0);
        elbow_servo.set_angle_270(0);
        
        // Stop wheels
        wheels_motor.set_speed_360(0);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}