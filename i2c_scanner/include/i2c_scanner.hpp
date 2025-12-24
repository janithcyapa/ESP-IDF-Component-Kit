#pragma once

#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"

namespace i2c_scanner {

/**
 * @brief Configuration for I2C master bus
 */
struct Config {
    gpio_num_t sda_io_num = GPIO_NUM_21;
    gpio_num_t scl_io_num = GPIO_NUM_22;
    i2c_port_t i2c_port = I2C_NUM_0;
    uint32_t clk_speed = 100000;  // 100 kHz
};

/**
 * @brief I2C Scanner class (RAII-style: auto init/deinit)
 */
class I2cScanner {
public:
    /**
     * @brief Construct and initialize I2C master
     * @param config Configuration struct
     */
    explicit I2cScanner(const Config& config);

    /**
     * @brief Destructor - automatically deinitializes I2C
     */
    ~I2cScanner();

    /**
     * @brief Perform I2C scan and print found devices
     */
    void scan();

private:
    i2c_port_t port_;
    static constexpr const char* TAG = "I2cScanner";
};

} // namespace i2c_scanner