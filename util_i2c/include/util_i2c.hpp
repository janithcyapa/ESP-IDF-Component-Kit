#pragma once

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/gpio.h"

// Configuration Macros
#define I2C_MASTER_SDA_IO GPIO_NUM_21
#define I2C_MASTER_SCL_IO GPIO_NUM_22
#define I2C_MASTER_NUM    0
#define I2C_MASTER_FREQ   100000

namespace i2c_util {

/**
 * @brief Initialize the I2C driver using the new master API.
 * Call this once at startup.
 */
void i2c_init();

/**
 * @brief Scan the I2C bus and print detected addresses to the console.
 */
void i2c_scan();

/**
 * @brief Get the I2C bus handle. 
 * Other sensor components will need this to attach to the bus.
 */
i2c_master_bus_handle_t get_bus_handle();

} // namespace i2c_util