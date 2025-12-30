void func(void);
#ifndef QMC5883L_H
#define QMC5883L_H

#include "driver/i2c.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the QMC5883L magnetometer.
 * * @note This function assumes i2c_driver_install() has already been called
 * for the specified port (e.g. by your MPU9250 driver).
 * * @param port The I2C port number (e.g., I2C_NUM_0)
 * @return ESP_OK on success
 */
esp_err_t qmc5883l_init(i2c_port_t port);

/**
 * @brief Read raw X, Y, Z magnetic field values.
 * * @param x Pointer to store X axis value
 * @param y Pointer to store Y axis value
 * @param z Pointer to store Z axis value
 * @return ESP_OK on success
 */
esp_err_t qmc5883l_read_mag(int16_t *x, int16_t *y, int16_t *z);

typedef struct {
    float x;
    float y;
    float z;
} qmc_vector_t;

esp_err_t qmc5883l_read_mag_float(qmc_vector_t *vec);

#ifdef __cplusplus
}
#endif

#endif // QMC5883L_H