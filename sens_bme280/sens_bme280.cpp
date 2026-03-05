#include "sens_bme280.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace sens_bme280 {

static const char* TAG = "BME280";

esp_err_t BME280::init(i2c_master_bus_handle_t bus_handle) {
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = sensor_addr_,
        .scl_speed_hz = 100000,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle_));

    // 1. Read ALL calibration data (Registers 0x88-0xA1 and 0xE1-0xE7)
    read_calibration();

    // 2. Set Configuration
    // Humidity oversampling x1 (0xF2)
    uint8_t reg_hum[] = {0xF2, 0x01};
    i2c_master_transmit(dev_handle_, reg_hum, 2, -1);

    // Temp x1, Pres x1, Normal Mode (0xF4)
    uint8_t reg_meas[] = {0xF4, 0x27};
    i2c_master_transmit(dev_handle_, reg_meas, 2, -1);

    ESP_LOGI(TAG, "BME280 initialized at 0x%02x", sensor_addr_);
    return ESP_OK;
}

esp_err_t BME280::read_calibration() {
    uint8_t cal1[24], cal2[1], cal3[7];
    uint8_t reg;

    // Dig_T1 to Dig_P9
    reg = 0x88;
    i2c_master_transmit_receive(dev_handle_, &reg, 1, cal1, 24, -1);
    calib_.dig_T1 = (cal1[1] << 8) | cal1[0];
    calib_.dig_T2 = (int16_t)((cal1[3] << 8) | cal1[2]);
    calib_.dig_T3 = (int16_t)((cal1[5] << 8) | cal1[4]);
    calib_.dig_P1 = (cal1[7] << 8) | cal1[6];
    calib_.dig_P2 = (int16_t)((cal1[9] << 8) | cal1[8]);
    calib_.dig_P3 = (int16_t)((cal1[11] << 8) | cal1[10]);
    calib_.dig_P4 = (int16_t)((cal1[13] << 8) | cal1[12]);
    calib_.dig_P5 = (int16_t)((cal1[15] << 8) | cal1[14]);
    calib_.dig_P6 = (int16_t)((cal1[17] << 8) | cal1[16]);
    calib_.dig_P7 = (int16_t)((cal1[19] << 8) | cal1[18]);
    calib_.dig_P8 = (int16_t)((cal1[21] << 8) | cal1[20]);
    calib_.dig_P9 = (int16_t)((cal1[23] << 8) | cal1[22]);

    // Dig_H1
    reg = 0xA1;
    i2c_master_transmit_receive(dev_handle_, &reg, 1, cal2, 1, -1);
    calib_.dig_H1 = cal2[0];

    // Dig_H2 to Dig_H6
    reg = 0xE1;
    i2c_master_transmit_receive(dev_handle_, &reg, 1, cal3, 7, -1);
    calib_.dig_H2 = (int16_t)((cal3[1] << 8) | cal3[0]);
    calib_.dig_H3 = cal3[2];
    calib_.dig_H4 = (int16_t)((cal3[3] << 4) | (cal3[4] & 0x0F));
    calib_.dig_H5 = (int16_t)((cal3[5] << 4) | (cal3[4] >> 4));
    calib_.dig_H6 = (int8_t)cal3[6];

    return ESP_OK;
}

esp_err_t BME280::read_data(Bme280Data &data) {
    uint8_t reg = 0xF7;
    uint8_t raw[8];
    i2c_master_transmit_receive(dev_handle_, &reg, 1, raw, 8, -1);

    int32_t adc_P = (raw[0] << 12) | (raw[1] << 4) | (raw[2] >> 4);
    int32_t adc_T = (raw[3] << 12) | (raw[4] << 4) | (raw[5] >> 4);
    int32_t adc_H = (raw[6] << 8) | raw[7];

    // Temperature Compensation
    double var1t = (((double)adc_T) / 16384.0 - ((double)calib_.dig_T1) / 1024.0) * ((double)calib_.dig_T2);
    double var2t = ((((double)adc_T) / 131072.0 - ((double)calib_.dig_T1) / 8192.0) *
                   (((double)adc_T) / 131072.0 - ((double)calib_.dig_T1) / 8192.0)) * ((double)calib_.dig_T3);
    double t_fine = var1t + var2t;
    data.temperature = (float)(t_fine / 5120.0);

    // Pressure Compensation
    double var1p = (t_fine / 2.0) - 64000.0;
    double var2p = var1p * var1p * ((double)calib_.dig_P6) / 32768.0;
    var2p = var2p + var1p * ((double)calib_.dig_P5) * 2.0;
    var2p = (var2p / 4.0) + (((double)calib_.dig_P4) * 65536.0);
    var1p = (((double)calib_.dig_P3) * var1p * var1p / 524288.0 + ((double)calib_.dig_P2) * var1p) / 524288.0;
    var1p = (1.0 + var1p / 32768.0) * ((double)calib_.dig_P1);
    
    if (var1p == 0.0) {
        data.pressure = 0; 
    } else {
        double p = 1048576.0 - (double)adc_P;
        p = (p - (var2p / 4096.0)) * 6250.0 / var1p;
        var1p = ((double)calib_.dig_P9) * p * p / 2147483648.0;
        var2p = p * ((double)calib_.dig_P8) / 32768.0;
        p = p + (var1p + var2p + ((double)calib_.dig_P7)) / 16.0;
        data.pressure = (float)(p / 100.0); // Convert to hPa
    }

    // Humidity Compensation
    double h = (t_fine - 76800.0);
    h = (adc_H - (((double)calib_.dig_H4) * 64.0 + ((double)calib_.dig_H5) / 16384.0 * h)) *
        (((double)calib_.dig_H2) / 65536.0 * (1.0 + ((double)calib_.dig_H6) / 67108864.0 * h *
        (1.0 + ((double)calib_.dig_H3) / 67108864.0 * h)));
    h = h * (1.0 - ((double)calib_.dig_H1) * h / 524288.0);
    if (h > 100.0) h = 100.0;
    else if (h < 0.0) h = 0.0;
    data.humidity = (float)h;

    return ESP_OK;
}

} // namespace sens_bme280