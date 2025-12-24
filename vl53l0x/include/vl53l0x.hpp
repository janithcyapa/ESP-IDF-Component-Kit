#pragma once

#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include <optional>

class VL53L0X {
public:
    struct Config {
        i2c_port_t port = I2C_NUM_0;
        gpio_num_t sda = GPIO_NUM_21;
        gpio_num_t scl = GPIO_NUM_22;
        uint32_t clock_speed = 400000;  // 400 kHz
        uint8_t address = 0x29;
    };

    explicit VL53L0X(const Config& config);
    ~VL53L0X();

    bool init();
    std::optional<uint16_t> read_range_single_mm();     // Single shot
    std::optional<uint16_t> read_range_continuous_mm(); // For continuous mode

    void set_timeout(uint16_t timeout_ms) { timeout_ms_ = timeout_ms; }
    bool set_measurement_timing_budget(uint32_t budget_us);

private:
    // I2C helpers
    bool write_reg(uint8_t reg, uint8_t value);
    bool write_reg16(uint8_t reg, uint16_t value);
    bool write_multi(uint8_t reg, const uint8_t* src, uint8_t count);
    uint8_t read_reg(uint8_t reg);
    uint16_t read_reg16(uint8_t reg);
    bool read_multi(uint8_t reg, uint8_t* dst, uint8_t count);

    // Initialization steps
    bool load_default_tuning_settings();
    bool perform_single_ref_calibration(uint8_t vhv_init_byte);

    i2c_port_t port_;
    uint8_t address_;
    uint16_t timeout_ms_ = 1000;
    uint8_t stop_variable_ = 0;
    uint32_t measurement_timing_budget_us_ = 33000;

    bool did_timeout_ = false;
};

// Key VL53L0X registers
#define VL53L0X_REG_IDENTIFICATION_MODEL_ID         0xC0
#define VL53L0X_REG_IDENTIFICATION_REVISION_ID      0xC2
#define VL53L0X_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD   0x50
#define VL53L0X_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD 0x70
#define VL53L0X_REG_SYSRANGE_START                  0x00
#define VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG          0x01
#define VL53L0X_REG_SYSTEM_INTERRUPT_CONFIG_GPIO    0x0A
#define VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR          0x0B
#define VL53L0X_REG_RESULT_INTERRUPT_STATUS         0x13
#define VL53L0X_REG_RESULT_RANGE_STATUS             0x14
#define VL53L0X_REG_RESULT_PEAK_SIGNAL_RATE_LIMIT   0x1C
#define VL53L0X_REG_VHV_CONFIG_PAD_SCL_SDA__TIMEOUT_CONTROL 0x89
#define VL53L0X_REG_MSRC_CONFIG_TIMEOUT_MACROP      0x46
#define VL53L0X_REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT 0x44
#define VL53L0X_REG_SYSTEM_THRESH_HIGH              0x0C
#define VL53L0X_REG_SYSTEM_THRESH_LOW               0x0E
#define VL53L0X_REG_RESULT_RANGE_VAL                0x1E  // Distance in mm
#define VL53L0X_REG_SOFT_RESET_GO2_SOFT_RESET_N     0xBF
#define VL53L0X_REG_GLOBAL_CONFIG_VCSEL_WIDTH       0x32
#define VL53L0X_REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_0 0xB0