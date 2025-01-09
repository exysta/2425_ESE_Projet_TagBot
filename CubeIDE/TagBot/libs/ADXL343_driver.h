/**
 * @file ADXL343_driver.h
 * @brief Driver for ADXL343 accelerometer.
 *
 * This header file contains the register definitions, function prototypes,
 * and task management for the ADXL343 accelerometer. The functions allow for
 * the initialization, configuration, reading, and writing to the ADXL343 registers.
 * Additionally, functions for detecting taps and reading accelerometer data
 * (X, Y, Z axes) are provided.
 *
 * @date Sep 16, 2024
 * @author marie
 */

#ifndef INC_ADXL343_DRIVER_H_
#define INC_ADXL343_DRIVER_H_

#include <stdint.h>
#include <stddef.h>

// Register Definitions for ADXL343 Accelerometer
#define ADXL343_REG_DEVID        0x00   /**< Device ID Register */
#define ADXL343_REG_THRESH_TAP   0x1D   /**< Tap Threshold Register */
#define ADXL343_REG_OFFSX        0x1E   /**< X-axis Offset Register */
#define ADXL343_REG_OFFSY        0x1F   /**< Y-axis Offset Register */
#define ADXL343_REG_OFFSZ        0x20   /**< Z-axis Offset Register */
#define ADXL343_REG_DUR          0x21   /**< Tap Duration Register */
#define ADXL343_REG_LATENT       0x22   /**< Latency Register */
#define ADXL343_REG_THRESH_ACT   0x24   /**< Activity Threshold Register */
#define ADXL343_REG_TAP_AXES     0x2A   /**< Tap Axes Register */
#define ADXL343_REG_BW_RATE      0x2C   /**< Bandwidth Rate Register */
#define ADXL343_REG_POWER_CTL    0x2D   /**< Power Control Register */
#define ADXL343_REG_INT_ENABLE   0x2E   /**< Interrupt Enable Register */
#define ADXL343_REG_INT_MAP      0x2F   /**< Interrupt Mapping Register */
#define ADXL343_REG_INT_SOURCE   0x30   /**< Interrupt Source Register */
#define ADXL343_REG_DATA_FORMAT  0x31   /**< Data Format Register */
#define ADXL343_REG_DATAX0       0x32   /**< X-axis Data 0 Register */
#define ADXL343_REG_DATAX1       0x33   /**< X-axis Data 1 Register */
#define ADXL343_REG_DATAY0       0x34   /**< Y-axis Data 0 Register */
#define ADXL343_REG_DATAY1       0x35   /**< Y-axis Data 1 Register */
#define ADXL343_REG_DATAZ0       0x36   /**< Z-axis Data 0 Register */
#define ADXL343_REG_DATAZ1       0x37   /**< Z-axis Data 1 Register */

// Task Priority for ADXL343 Task
#define ADXL343_Task_Priority    8

/**
 * @brief Initializes the ADXL343 accelerometer.
 *
 * This function initializes the ADXL343 accelerometer by configuring the
 * required registers to their default values. It prepares the device for
 * data reading and configuration.
 *
 * @return int 0 on success, non-zero error code on failure.
 */
int ADXL343_Init(void);

/**
 * @brief Configures the ADXL343 accelerometer settings.
 *
 * This function configures the ADXL343's various settings such as
 * bandwidth rate, data format, and other device-specific settings
 * according to application needs.
 */
void ADXL343_Configure(void);

/**
 * @brief Reads data from a specific register of the ADXL343.
 *
 * This function reads data from the specified register of the ADXL343 device.
 *
 * @param reg The register address to read from.
 * @param rx_data Pointer to a buffer where the read data will be stored.
 * @param length The number of bytes to read from the register.
 */
void ADXL343_ReadRegister(uint8_t reg, int8_t* rx_data, size_t length);

/**
 * @brief Writes data to a specific register of the ADXL343.
 *
 * This function writes data to the specified register of the ADXL343 device.
 *
 * @param reg The register address to write to.
 * @param data The data to write to the register.
 */
void ADXL343_WriteRegister(uint8_t reg, uint8_t data);

/**
 * @brief Detects tap events on the ADXL343.
 *
 * This function checks the ADXL343 for tap events. If a tap event is detected,
 * it can trigger an interrupt or other actions depending on the configuration.
 */
void ADXL343_DetectTap(void);

/**
 * @brief Reads the X, Y, Z axis data from the ADXL343.
 *
 * This function reads the accelerometer values from the X, Y, and Z axes of
 * the ADXL343 and stores the values in the provided variables.
 *
 * @param x Pointer to store the X-axis value.
 * @param y Pointer to store the Y-axis value.
 * @param z Pointer to store the Z-axis value.
 */
void ADXL343_Read_XYZ(int16_t* x, int16_t* y, int16_t* z);

/**
 * @brief Calibrates the accelerometer offsets.
 *
 * This function calibrates the offset values for the X, Y, and Z axes
 * of the accelerometer to correct for any bias in the sensor readings.
 */
void calibrateOffsets(void);

/**
 * @brief ADXL343 task for reading accelerometer data.
 *
 * The `ADXL343_Task` runs in a loop and continuously reads accelerometer data
 * from the ADXL343. It processes the data and performs any necessary actions,
 * such as detecting tap events or reading accelerometer values.
 *
 * @param unused Unused task parameter.
 */
void ADXL343_Task(void *unused);

/**
 * @brief Creates and starts the ADXL343 task.
 *
 * This function creates a FreeRTOS task to manage the ADXL343 accelerometer.
 * The task is created with a priority defined by `ADXL343_Task_Priority`.
 *
 * @param unused Unused task parameter.
 */
void ADXL343_TaskCreate(void *unused);

#endif /* INC_ADXL343_DRIVER_H_ */
