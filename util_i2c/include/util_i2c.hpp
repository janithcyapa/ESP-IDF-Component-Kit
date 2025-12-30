#pragma once

#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"

// Configuration Macros
#define I2C_MASTER_SDA_IO GPIO_NUM_21
#define I2C_MASTER_SCL_IO GPIO_NUM_22
#define I2C_MASTER_NUM    I2C_NUM_0
#define I2C_MASTER_FREQ   100000

namespace i2c_util {

/**
 * @brief Initialize the I2C driver using the default macros.
 * Call this once at startup.
 */
void i2c_init();

/**
 * @brief Scan the I2C bus and print detected addresses to the console.
 */
void i2c_scan();

} // namespace i2c_util