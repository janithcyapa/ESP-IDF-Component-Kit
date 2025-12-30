#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sens_qmc5883l.h"

static const char *TAG = "QMC5883L";

// --- Internal Configuration ---
#define QMC5883L_ADDR       0x0D

// --- Registers ---
#define QMC_REG_X_LSB       0x00
#define QMC_REG_STATUS      0x06
#define QMC_REG_CONTROL_1   0x09
#define QMC_REG_CONTROL_2   0x0A
#define QMC_REG_RESET       0x0B

// Store the I2C port internally
static i2c_port_t g_i2c_port = I2C_NUM_0;

// Internal helper: Write byte
static esp_err_t qmc5883l_write_reg(uint8_t reg_addr, uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (QMC5883L_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(g_i2c_port, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return ret;
}

// Internal helper: Read bytes
static esp_err_t qmc5883l_read_bytes(uint8_t reg_addr, uint8_t *data, size_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (QMC5883L_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (QMC5883L_ADDR << 1) | I2C_MASTER_READ, true);
    
    if (len > 1) {
        i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, &data[len - 1], I2C_MASTER_NACK);
    
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(g_i2c_port, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t qmc5883l_init(i2c_port_t port) {
    g_i2c_port = port;

    // 1. Soft Reset
    esp_err_t ret = qmc5883l_write_reg(QMC_REG_RESET, 0x01);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "QMC5883L not found or I2C error");
        return ret;
    }
    vTaskDelay(pdMS_TO_TICKS(10)); 

    // 2. Setup Control Register 1
    // Mode: Continuous (01), ODR: 200Hz (11), RNG: 8G (01), OSR: 512 (00) -> 0x1D
    ret = qmc5883l_write_reg(QMC_REG_CONTROL_1, 0x1D);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Initialized successfully on port %d", port);
    }
    return ret;
}

esp_err_t qmc5883l_read_mag(int16_t *x, int16_t *y, int16_t *z) {
    uint8_t buffer[6];
    esp_err_t ret = qmc5883l_read_bytes(QMC_REG_X_LSB, buffer, 6);
    
    if (ret == ESP_OK) {
        *x = (int16_t)((buffer[1] << 8) | buffer[0]);
        *y = (int16_t)((buffer[3] << 8) | buffer[2]);
        *z = (int16_t)((buffer[5] << 8) | buffer[4]);
    } else {
        ESP_LOGE(TAG, "Failed to read data");
    }
    return ret;
}

esp_err_t qmc5883l_read_mag_float(qmc_vector_t *vec) {
    int16_t raw_x, raw_y, raw_z;
    esp_err_t ret = qmc5883l_read_mag(&raw_x, &raw_y, &raw_z);

    // Convert to Gauss (for 8G Range setting)
    if (ret == ESP_OK) {
        vec->x = raw_x / 3000.0f;
        vec->y = raw_y / 3000.0f;
        vec->z = raw_z / 3000.0f;
    }
    return ret;
}