# VL53L0X ESP32 ESP-IDF Driver

A lightweight C driver for the STMicroelectronics VL53L0X Time-of-Flight (ToF) distance sensor. This implementation is designed specifically for the **ESP-IDF** (v3.x/v4.x/v5.x) framework on ESP32.



## Features
- **Single Shot & Continuous Mode**: Measure distances up to 2 meters.
- **Timing Budget Control**: Balance between high speed (20ms) and high accuracy (200ms).
- **Multiple Sensor Support**: Hardware control via `XSHUT` and I2C address reassignment.
- **ESP32 Optimized**: Uses the native `driver/i2c.h` master driver.

## Hardware Connection

| VL53L0X Pin | ESP32 Pin (Example) | Description |
| :--- | :--- | :--- |
| **VCC** | 3.3V | Power Supply |
| **GND** | GND | Ground |
| **SCL** | GPIO 22 | I2C Clock |
| **SDA** | GPIO 21 | I2C Data |
| **XSHUT** | GPIO 19 | Shutdown (Optional, for Reset/Multi-sensor) |

## Usage

### 1. Initialization
```c
// Configure the sensor
vl53l0x_t *sensor = vl53l0x_config(I2C_NUM_0, 22, 21, 19, 0x29, 1);

if (sensor == NULL) {
    printf("Failed to configure I2C\n");
}

// Initialize internal tuning
const char *err = vl53l0x_init(sensor);
if (err) {
    printf("Init Error: %s\n", err);
}
```

### 2. Single Distance Measurement
```c
uint16_t distance = vl53l0x_readRangeSingleMillimeters(sensor);
printf("Distance: %d mm\n", distance);
```

### 3. High Accuracy Mode

To increase accuracy, increase the timing budget (e.g., to 200ms):
```c
vl53l0x_setMeasurementTimingBudget(sensor, 200000);
```

### 4. Multiple Sensors

If using multiple sensors, connect each XSHUT pin to a different GPIO. Pull all XSHUT low initially, then wake them one by one and use vl53l0x_setAddress() to assign unique addresses.

## Error Handling

The driver provides two status checks:

- vl53l0x_timeoutOccurred(sensor): Returns true if the sensor failed to respond within the allotted time.

- vl53l0x_i2cFail(sensor): Returns true if an I2C communication error occurred.


## Funstions

### 1. Lifecycle and Configuration

These functions handle the memory and initial hardware setup.

- `vl53l0x_config(...)`: Initializes the ESP32 I2C peripheral. It sets which GPIOs to use for SDA/SCL, handles the xshut (reset) pin, and allocates memory for the sensor's state. It returns a handle used for all other functions.

- `vl53l0x_init(v)`: The most important setup step. It wakes the sensor from hardware standby, loads a large table of "tuning settings" (factory-recommended register values), and performs internal calibrations (VHV and Phase) required for accurate readings.

- `vl53l0x_end(v)`: Safely shuts down the I2C driver and frees the allocated memory.


### 2. I2C Address Management

Useful if you have multiple sensors on the same bus.

- `vl53l0x_setAddress(v, new_addr)`: Changes the sensor's I2C address. Since all VL53L0X sensors start with the same address (0x29), you use this to give each one a unique ID.

- `vl53l0x_getAddress(v)`: Returns the current I2C address stored in the local driver structure.

### 3. Register Access (Low-Level)

These are helper functions for communicating directly with the sensor's internal memory.

- `vl53l0x_writeReg(8/16/32)Bit`: Writes a 1, 2, or 4-byte value to a specific sensor register.

- `vl53l0x_readReg(8/16/32)Bit`: Reads a 1, 2, or 4-byte value from a specific sensor register.

- `vl53l0x_writeMulti / vl53l0x_readMulti`: Used to transfer a block of multiple bytes at once, which is more efficient for things like reading the SPAD (Single Photon Avalanche Diode) maps.

### 4. Performance Tuning

These allow you to trade off speed, range, and accuracy.

- `vl53l0x_setSignalRateLimit(v, limit_Mcps)`: Sets the minimum "return signal" strength required to validate a reading.
    - Lower limit: Allows for longer range but more noise.
    - Higher limit: More reliable readings but shorter range.

- `vl53l0x_setMeasurementTimingBudget(v, budget_us)`: Defines how much time the sensor has to calculate a distance.

    - 20ms: High speed, lower accuracy.

    - 200ms: High accuracy, slow update rate.

- `vl53l0x_setVcselPulsePeriod(v, type, period_pclks)`: Adjusts the laser pulse duration. Longer pulses (higher PCLK values) are generally required for "Long Range" mode configurations.

### 5. Ranging Operations

The primary functions for getting distance data.

- `vl53l0x_startContinuous(v, period_ms)`: Puts the sensor in a loop where it takes measurements automatically. If period_ms is 0, it runs as fast as the timing budget allows.

- `vl53l0x_stopContinuous(v)`: Tells the sensor to stop its automatic looping and go into an idle state.

- `vl53l0x_readRangeContinuousMillimeters(v)`: Used when in continuous mode. It waits for the "Data Ready" interrupt and returns the distance in mm.

- `vl53l0x_readRangeSingleMillimeters(v)`: A "one-shot" function. It wakes the sensor, takes one measurement, and returns the result. This is simpler but slower if you need frequent updates.

### 6. Health and Timeout Monitoring

Because I2C can hang if a wire is loose, these keep your code from crashing.

- `vl53l0x_setTimeout(v, timeout)`: Sets how long the driver should wait for the sensor to respond before giving up.

- `vl53l0x_getTimeout(v)`: Returns the current timeout setting.

- `vl53l0x_timeoutOccurred(v)`: Returns 1 if the last operation failed due to a timeout. It resets the flag to 0 after being called.

- `vl53l0x_i2cFail(v)`: Returns 1 if the ESP32 I2C driver reported a hardware communication error.

### Summary Table for ToF Concepts

- PCLK  | Pixel Clock
    - Timing unit for the laser pulses.
- MCPS	| Million Counts Per Second	        
    - Measurement of the signal strength returning from the target.
- SPAD	| Single Photon Avalanche Diode	
    - The individual light-sensing "pixels" on the sensor.
- XSHUT	| Shutdown Pin	
    - A hardware pin used to reset the sensor or put it to sleep.