#include "vl53l0x.hpp"
#include "esp_timer.h"

static const char* TAG = "VL53L0X";

// Default tuning settings from ST/Pololu
static const uint8_t default_settings[] = {
    0xFF, 0x01, 0x00, 0x00, 0xFF, 0x00, 0x09, 0x00,
    0x10, 0x00, 0x11, 0x00, 0x24, 0x01, 0x25, 0xFF,
    0x75, 0x00, 0xFF, 0x01, 0x4E, 0x00, 0x4F, 0x64,
    0xFF, 0x00, 0xFF, 0x01, 0x00, 0x00
};

VL53L0X::VL53L0X(const Config& config) : port_(config.port), address_(config.address) {
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = config.sda,
        .scl_io_num = config.scl,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {.clk_speed = config.clock_speed},
        .clk_flags = 0  // Fixed: missing initializer
    };

    ESP_ERROR_CHECK(i2c_param_config(port_, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(port_, I2C_MODE_MASTER, 0, 0, 0));
    ESP_LOGI(TAG, "I2C initialized for VL53L0X");
}

VL53L0X::~VL53L0X() {
    i2c_driver_delete(port_);
}

bool VL53L0X::write_reg(uint8_t reg, uint8_t value) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address_ << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, value, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(port_, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return ret == ESP_OK;
}

bool VL53L0X::write_reg16(uint8_t reg, uint16_t value) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address_ << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, value >> 8, true);
    i2c_master_write_byte(cmd, value & 0xFF, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(port_, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return ret == ESP_OK;
}

uint8_t VL53L0X::read_reg(uint8_t reg) {
    uint8_t value = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address_ << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address_ << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &value, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(port_, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return value;
}

uint16_t VL53L0X::read_reg16(uint8_t reg) {
    uint8_t data[2];
    if (read_multi(reg, data, 2)) {
        return (uint16_t)data[0] << 8 | data[1];
    }
    return 0;
}

bool VL53L0X::read_multi(uint8_t reg, uint8_t* dst, uint8_t count) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address_ << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address_ << 1) | I2C_MASTER_READ, true);
    for (uint8_t i = 0; i < count - 1; ++i) {
        i2c_master_read_byte(cmd, dst + i, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, dst + count - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(port_, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return ret == ESP_OK;
}

bool VL53L0X::load_default_tuning_settings() {
    for (size_t i = 0; i < sizeof(default_settings); i += 2) {
        if (!write_reg(default_settings[i], default_settings[i + 1])) {
            return false;
        }
    }
    return true;
}

bool VL53L0X::perform_single_ref_calibration(uint8_t vhv_init_byte) {
    write_reg(VL53L0X_REG_SYSRANGE_START, 0x01 | vhv_init_byte);
    uint64_t start = esp_timer_get_time();
    while ((read_reg(VL53L0X_REG_RESULT_INTERRUPT_STATUS) & 0x07) == 0) {
        if (esp_timer_get_time() - start > timeout_ms_ * 1000) return false;
    }
    write_reg(VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR, 0x01);
    write_reg(VL53L0X_REG_SYSRANGE_START, 0x00);
    return true;
}

bool VL53L0X::init() {
    if (read_reg(VL53L0X_REG_IDENTIFICATION_MODEL_ID) != 0xEE) {
        ESP_LOGE(TAG, "Not a VL53L0X! Model ID: 0x%02x", read_reg(VL53L0X_REG_IDENTIFICATION_MODEL_ID));
        return false;
    }

    // Standard init sequence
    write_reg(0x88, 0x00);
    write_reg(0x80, 0x01);
    write_reg(0xFF, 0x01);
    write_reg(0x00, 0x00);
    stop_variable_ = read_reg(0x91);
    write_reg(0x00, 0x01);
    write_reg(0xFF, 0x00);
    write_reg(0x80, 0x00);

    if (!load_default_tuning_settings()) return false;

    // Reference calibration
    if (!perform_single_ref_calibration(0x40)) return false; // VHV
    if (!perform_single_ref_calibration(0x00)) return false; // Phase

    set_measurement_timing_budget(66000);  // Good default

    ESP_LOGI(TAG, "VL53L0X initialized successfully");
    return true;
}

bool VL53L0X::set_measurement_timing_budget(uint32_t budget_us) {
    if (budget_us < 20000) budget_us = 20000;
    measurement_timing_budget_us_ = budget_us;
    // Simplified - in full driver this adjusts VCSEL periods
    return true;
}

std::optional<uint16_t> VL53L0X::read_range_single_mm() {
    write_reg(VL53L0X_REG_SYSRANGE_START, 0x01);

    uint64_t start = esp_timer_get_time();
    while ((read_reg(VL53L0X_REG_RESULT_RANGE_STATUS) & 0x01) == 0) {
        if (esp_timer_get_time() - start > timeout_ms_ * 1000ULL) {
            return std::nullopt;
        }
    }

    uint16_t range = read_reg16(VL53L0X_REG_RESULT_RANGE_VAL);
    write_reg(VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR, 0x01);

    return (range < 8191) ? std::optional<uint16_t>(range) : std::nullopt;
}