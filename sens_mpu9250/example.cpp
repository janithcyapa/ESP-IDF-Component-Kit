// #include <stdio.h>

// #include "esp_log.h"
// #include "mpu9250.h"
// #include "driver_mpu9250.h"
// #include "mpu9250_tests.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/i2c_master.h"
// #include "esp_log.h"

// static mpu9250_handle_t mpu9250_hdl;

// extern "C" void app_main() { 
//     static const char* TAG = "MIAN";

//     ESP_LOGI(TAG, "Starting Yathra");
    
//     i2c_master_bus_config_t i2c_master_conf = {}; // Initialize to zero
//     i2c_master_conf.clk_source = I2C_CLK_SRC_DEFAULT;
//     i2c_master_conf.i2c_port = I2C_NUM_0;
//     i2c_master_conf.sda_io_num = GPIO_NUM_21;
//     i2c_master_conf.scl_io_num = GPIO_NUM_22;
//     i2c_master_conf.flags.enable_internal_pullup = true; 
//     i2c_master_conf.glitch_ignore_cnt = 9;

//     i2c_device_config_t i2c_dev_conf = {};
//     i2c_dev_conf.dev_addr_length = I2C_ADDR_BIT_LEN_7;
//     i2c_dev_conf.device_address = 0x68;
//     // i2c_dev_conf.scl_speed_hz = 400000;
//     i2c_dev_conf.scl_speed_hz = 100000;


//     ESP_LOGI(TAG, "init i2c...");
//     ESP_ERROR_CHECK(mpu9250_init_i2c(&mpu9250_hdl, &i2c_master_conf, &i2c_dev_conf));
//     ESP_LOGI(TAG, "run mpu9250_basic_read_test 10 times");
//     ESP_ERROR_CHECK(mpu9250_basic_read_test(&mpu9250_hdl, 10));


//     while (true) {
//        // ESP_LOGI(TAG,"waiting...");
//         vTaskDelay(pdMS_TO_TICKS(100));
//     }
// }