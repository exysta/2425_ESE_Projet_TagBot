/**
 * @file distanceSensor.h
 * @brief Header file for distance sensor driver.
 *
 * This file contains definitions, structures, and function prototypes for
 * interfacing with the distance sensor, including ADC initialization, reading,
 * and task management.
 *
 * @author marie & charlotte & lucas
 * @date Sep 22, 2024
 */

#ifndef INC_DISTSENSOR_DRIVER_H_
#define INC_DISTSENSOR_DRIVER_H_

#include <stddef.h>
#include <stdint.h>

/** @brief FreeRTOS task priority for distance sensor task. */
#define distSensor_Task_Priority 10

/**
 * @brief Enum representing the detection status of the distance sensor.
 *
 * The sensor can detect either the floor or a void (empty space).
 */
typedef enum {
    DistSensor_DETECT_FLOOR = 0, /**< Floor detected. */
    DistSensor_DETECT_VOID = 1,  /**< Void (empty space) detected. */
} DistSensor_Detection_Status_t;

/**
 * @brief Enum representing the ADC status for the distance sensor.
 *
 * Indicates whether the ADC has data ready or not.
 */
typedef enum {
    DistSensor_ADC_STATUS_NO_DATA = 0, /**< No data available. */
    DistSensor_ADC_STATUS_READY = 1,   /**< ADC data is ready. */
} DistSensor_ADC_Status_t;

/**
 * @brief Enum representing the position of the distance sensor.
 *
 * The sensor's position is defined in terms of cardinal directions.
 */
typedef enum {
    DistSensor_POSITION_NORD = 0, /**< Sensor positioned to the north. */
    DistSensor_POSITION_EAST = 1, /**< Sensor positioned to the east. */
    DistSensor_POSITION_SUD = 2,  /**< Sensor positioned to the south. */
    DistSensor_POSITION_WEST = 3, /**< Sensor positioned to the west. */
} DistSensor_Position_t;

/**
 * @brief Structure representing the distance sensor's state.
 *
 * This structure contains the current detection status, the sensor value,
 * the position of the sensor, and the ADC status.
 */
typedef struct {
    DistSensor_Detection_Status_t sensor_detection_status; /**< Current detection status of the sensor. */
    uint16_t sensor_value; /**< The sensor's raw ADC value. */
    DistSensor_Position_t sensor_position; /**< The position of the sensor (cardinal direction). */
    volatile DistSensor_ADC_Status_t adc_ready; /**< ADC readiness status. */
} DistSensor_handle_t;

/** @brief ADC threshold value to detect void. */
#define VOID_TRESHOLD 1000 /**< ADC value threshold to detect void. */

/**
 * @brief Initializes the ADC for the distance sensor using DMA.
 *
 * This function configures the ADC to operate in DMA mode for continuous
 * sampling of the distance sensor's data.
 */
void distSensor_initADC_DMA(void);

/**
 * @brief Reads the ADC value of the distance sensor.
 *
 * This function reads the ADC value of the distance sensor and returns
 * the converted value.
 *
 * @param hadc Pointer to the ADC handle.
 * @return The ADC conversion result.
 */
uint32_t distSensor_ReadADC(ADC_HandleTypeDef* hadc);

/**
 * @brief Reads the ADC value of the distance sensor using DMA.
 *
 * This function triggers an ADC read operation using DMA and returns
 * the status of the operation.
 *
 * @return HAL status indicating success or failure.
 */
HAL_StatusTypeDef distSensor_ReadADC_DMA(void);

/**
 * @brief Creates the FreeRTOS task for the distance sensor.
 *
 * This function initializes and creates the FreeRTOS task responsible
 * for managing the distance sensor operations.
 *
 * @param unused Unused task argument.
 */
void distSensor_TaskCreate(void* unused);

/**
 * @brief FreeRTOS task function for the distance sensor.
 *
 * This task continuously monitors the distance sensor, processes its
 * readings, and updates the sensor's status.
 *
 * @param unused Unused task argument.
 */
void distSensor_Task(void *unused);

/**
 * @brief Callback function triggered when ADC conversion is complete.
 *
 * This function is called automatically by the HAL when an ADC conversion
 * is complete, and is used to process the results.
 *
 * @param hadc Pointer to the ADC handle.
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc);

#endif /* INC_DISTSENSOR_DRIVER_H_ */
