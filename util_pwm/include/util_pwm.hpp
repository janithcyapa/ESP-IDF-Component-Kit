#pragma once

#include "driver/ledc.h"
#include "esp_err.h"
#include <algorithm>

class PwmUtil {
private:
    ledc_channel_t _channel;
    int _gpio;
    uint32_t _freq;
    ledc_timer_t _timer;

    void update_pulse(uint32_t us) {
        // 13-bit resolution at 50Hz (20,000us period) -> 8191 max duty
        uint32_t duty = (us * 8191) / 20000;
        ledc_set_duty(LEDC_LOW_SPEED_MODE, _channel, duty);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, _channel);
    }

public:
    // Constructor stores the specific hardware info
    PwmUtil(int gpio, ledc_channel_t channel, ledc_timer_t timer = LEDC_TIMER_0) 
        : _channel(channel), _gpio(gpio), _freq(50), _timer(timer) {}

    esp_err_t init(uint32_t freq_hz = 50) {
        _freq = freq_hz;
        
        ledc_timer_config_t timer_conf = {
            .speed_mode      = LEDC_LOW_SPEED_MODE,
            .duty_resolution = LEDC_TIMER_13_BIT,
            .timer_num       = _timer,
            .freq_hz         = _freq,
            .clk_cfg         = LEDC_AUTO_CLK
        };
        ledc_timer_config(&timer_conf);

        ledc_channel_config_t chan_conf = {
            .gpio_num   = _gpio,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel    = _channel,
            .intr_type  = LEDC_INTR_DISABLE,
            .timer_sel  = _timer,
            .duty       = 0,
            .hpoint     = 0
        };
        return ledc_channel_config(&chan_conf);
    }

    void set_angle_180(int angle) {
        angle = std::clamp(angle, 0, 180);
        update_pulse(500 + (angle * 2000 / 180));
    }

    void set_angle_270(int angle) {
        angle = std::clamp(angle, 0, 270);
        update_pulse(500 + (angle * 2000 / 270));
    }

    void set_speed_360(int speed) {
        speed = std::clamp(speed, -100, 100);
        update_pulse(1500 + (speed * 10)); // -100=500us, 0=1500us, 100=2500us
    }

    void set_raw_duty(uint32_t duty) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, _channel, duty);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, _channel);
    }
};