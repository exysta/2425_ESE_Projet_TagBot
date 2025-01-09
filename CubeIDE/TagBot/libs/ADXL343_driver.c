/**
 * @file ADXL343_driver.c
 * @brief Driver implementation for the ADXL343 accelerometer.
 *
 * This file implements the functions for initializing, configuring, and reading data
 * from the ADXL343 accelerometer using SPI communication. It also includes the
 * functionality to detect tap events and calibrate the accelerometer offsets.
 * The task management is handled using FreeRTOS.
 *
 * @date Sep 16, 2024
 * @author marie
 * @note Code for the ADXL343 accelerometer
 */

#include <ADXL343_driver.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <spi.h>
#include <stddef.h>
#include "FreeRTOS.h"
#include "task.h"

// Constants for accelerometer configuration
#define TAP_THRESHOLD 0x20   /**< Tapping threshold: 2g (0x20), 6g (0x60), 8g (0x80), 16g (0xFF) */
#define TAP_DURATION 0x10    /**< Tapping duration: 10ms */
#define TAP_LATENT 0xC8      /**< Tapping latency: 250ms */

/**
 * @brief Initializes the ADXL343 accelerometer.
 *
 * This function reads the device ID and verifies it to ensure the ADXL343 is connected.
 * Then, it configures the accelerometer's power mode, data format, and measurement settings.
 *
 * @return int 0 on success, non-zero error code on failure.
 */
int ADXL343_Init(void) {
    uint8_t id = 0;

    // Read the device ID to verify the connection
    ADXL343_ReadRegister(ADXL343_REG_DEVID, &id, 1);

    if (id != 0xE5) {
        printf("ADXL343 not detected! ID: %02X\r\n", id);
        return 1;
    }

    /* Configuration of registers */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    ADXL343_WriteRegister(ADXL343_REG_POWER_CTL, 0x04);  // Sleep mode
    ADXL343_WriteRegister(ADXL343_REG_DATA_FORMAT, 0x08); // Full resolution, ±2g
    ADXL343_WriteRegister(ADXL343_REG_BW_RATE, 0X0B);     // Data rate
    ADXL343_WriteRegister(ADXL343_REG_POWER_CTL, 0x08);   // Measurement mode (active)

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);    // Disable NSS

    printf("ADXL343 Initialization done\r\n");
    return 0;
}

/**
 * @brief Configures the ADXL343 accelerometer for tap detection.
 *
 * This function configures the tap detection thresholds, duration, latency, and axes
 * to monitor for tap events. It also enables interrupt for single tap detection.
 */
void ADXL343_Configure(void) {
    ADXL343_WriteRegister(ADXL343_REG_THRESH_TAP, TAP_THRESHOLD);  // Set tap threshold (e.g., 2g)
    ADXL343_WriteRegister(ADXL343_REG_DUR, TAP_DURATION);         // Set tap duration (10ms)
    ADXL343_WriteRegister(ADXL343_REG_LATENT, TAP_LATENT);       // Set tap latency (250ms)
    ADXL343_WriteRegister(ADXL343_REG_TAP_AXES, 0x07);           // Enable tap detection on all axes (X, Y, Z)
    ADXL343_WriteRegister(ADXL343_REG_INT_ENABLE, 0x40);         // Enable single tap interrupt
    ADXL343_WriteRegister(ADXL343_REG_INT_MAP, 0x40);            // Map interrupt to INT1 pin

    printf("ADXL343 Configuration done\r\n");
}

/**
 * @brief Reads data from a register of the ADXL343.
 *
 * This function reads data from a specified register using SPI communication.
 *
 * @param reg The register address to read from.
 * @param rx_data Pointer to a buffer where the read data will be stored.
 * @param length The number of bytes to read from the register.
 */
void ADXL343_ReadRegister(uint8_t reg, int8_t* rx_data, size_t length) {
    uint8_t tx_data = reg | 0x80; // Set MSB to 1 for reading

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);  // Activate NSS
    HAL_SPI_Transmit(&hspi1, &tx_data, 1, 200);            // Send register address
    HAL_SPI_Receive(&hspi1, (uint8_t*)rx_data, 1, 200);    // Receive data
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);    // Deactivate NSS
}

/**
 * @brief Writes data to a register of the ADXL343.
 *
 * This function writes data to a specified register using SPI communication.
 *
 * @param reg The register address to write to.
 * @param data The data to write to the register.
 */
void ADXL343_WriteRegister(uint8_t reg, uint8_t data) {
    uint8_t buffer[2];
    buffer[0] = reg;  // Register address
    buffer[1] = data; // Data to write

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);  // Enable NSS
    HAL_SPI_Transmit(&hspi1, buffer, 2, 200);              // Transmit data
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);    // Disable NSS
}

/**
 * @brief Detects tap events from the ADXL343.
 *
 * This function reads the interrupt source register to check if a tap event has been detected.
 * If a tap is detected, it toggles a GPIO pin and prints a message.
 */
void ADXL343_DetectTap(void) {
    int8_t tap_status;
    ADXL343_ReadRegister(ADXL343_REG_INT_SOURCE, &tap_status, 1); // Read interrupt source

    // Check for single tap event (bit 6)
    if (tap_status & (1 << 6)) {
        printf("Tap detected!\r\n");
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
        HAL_Delay(100);  // Short delay after detecting a tap
    }
}

/**
 * @brief Reads the X, Y, Z acceleration values from the ADXL343.
 *
 * This function reads the 16-bit values for the X, Y, and Z axes from the ADXL343's data registers.
 *
 * @param x Pointer to store the X-axis value.
 * @param y Pointer to store the Y-axis value.
 * @param z Pointer to store the Z-axis value.
 */
void ADXL343_Read_XYZ(int16_t* x, int16_t* y, int16_t* z) {
    int8_t x_buff[2] = {0, 0};
    int8_t y_buff[2] = {0, 0};
    int8_t z_buff[2] = {0, 0};

    // Read the X, Y, and Z data registers
    ADXL343_ReadRegister(ADXL343_REG_DATAX0, x_buff, 2);
    ADXL343_ReadRegister(ADXL343_REG_DATAY0, y_buff, 2);
    ADXL343_ReadRegister(ADXL343_REG_DATAZ0, z_buff, 2);

    // Combine the MSB and LSB bytes to form 16-bit values
    *x = (int16_t)((x_buff[1] << 8) | x_buff[0]);
    *y = (int16_t)((y_buff[1] << 8) | y_buff[0]);
    *z = (int16_t)((z_buff[1] << 8) | z_buff[0]);
}

/**
 * @brief Calibrates the offsets for the ADXL343 accelerometer.
 *
 * This function reads the current accelerometer values and calculates offsets for
 * the X, Y, and Z axes. These offsets are then written to the corresponding
 * offset registers to correct for any sensor bias.
 */
void calibrateOffsets(void) {
    int16_t x, y, z;
    int8_t offsetx, offsety, offsetz;

    ADXL343_Read_XYZ(&x, &y, &z);

    // Compute offsets based on current values (adjusting for sensor bias)
    offsetx = -((x + 5) / 4 * 100);
    offsety = -((y + 5) / 4 * 100);
    offsetz = -(((z - 256) + 5) / 4 * 100);

    // Write the offsets to the ADXL343 registers
    ADXL343_WriteRegister(ADXL343_REG_OFFSX, offsetx);
    ADXL343_WriteRegister(ADXL343_REG_OFFSY, offsety);
    ADXL343_WriteRegister(ADXL343_REG_OFFSZ, offsetz);

    printf("Calibration done: Offset X: %i, Y: %i, Z: %i\r\n", offsetx, offsety, offsetz);
}


/**
 * @brief FreeRTOS task to handle the ADXL343 accelerometer.
 *
 * This task initializes and configures the ADXL343 accelerometer. It continuously
 * monitors the interrupt source register for tap events. When a tap is detected,
 * it logs a message and can trigger additional actions.
 *
 * @param unused Unused parameter.
 */
void ADXL343_Task(void* unused) {
    int init_attempts = 0;
    const int max_init_attempts = 5;

    // Initialize the accelerometer with a retry mechanism
    while (1 == ADXL343_Init()) {
        init_attempts++;
        if (init_attempts >= max_init_attempts) {
            printf("Failed to initialize ADXL343 after %d attempts.\r\n", max_init_attempts);
            vTaskSuspend(NULL); // Suspend the task if initialization fails
            return;
        }
        vTaskDelay(500); // Wait before retrying
    }

    // Configure the accelerometer
    ADXL343_Configure();

    for (;;) {
        uint8_t tap_status = 0;

        // Read interrupt source register
        ADXL343_ReadRegister(ADXL343_REG_INT_SOURCE, &tap_status, 1);

        // Check for single-tap event (bit 6)
        if (tap_status & (1 << 6)) {
            printf("Tap detected by Task!\r\n");

            // Toggle LED or trigger another task/state change
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
            vTaskDelay(250);

            /******* Add functionality for state change here ********/
        }

        // Delay to allow other tasks to execute
        vTaskDelay(100);
    }
}

/**
 * @brief Creates the ADXL343 FreeRTOS task.
 *
 * This function initializes and starts a FreeRTOS task that monitors the
 * ADXL343 accelerometer.
 *
 * @param unused Unused parameter.
 */
void ADXL343_TaskCreate(void* unused) {
    // Create the task with the specified name, stack size, priority, and function
    if (xTaskCreate(ADXL343_Task, "ADXL343 Task", 128, NULL, ADXL343_Task_Priority, NULL) != pdPASS) {
        printf("Failed to create ADXL343 Task\r\n");
    }
}

