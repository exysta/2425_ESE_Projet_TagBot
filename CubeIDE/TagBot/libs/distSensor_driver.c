
/**
 * @file distanceSensor.c
 * @brief Task and creation function for managing distance sensor readings.
 * @Author: marie & charlotte & lucas
 * This section of the code contains the `distSensor_Task` that continuously monitors
 * the status of the distance sensors. Based on sensor readings, it checks if the
 * sensors detect void (no surface detected), in which case it activates the robot braking
 * and prints a message to the console. The task also includes the `distSensor_TaskCreate`
 * function to create and start the task.
 *
 * @date Sep 22, 2024
 */

/*
 * CODE POUR LES CAPTEURS DE DISTANCES
 *
 * ********ADC1**********
 * - Capteur West : west
 * - Capteur Nord : nord
 *
 * ********ADC2**********
 * - Capteur Sud : sud
 * - Capteur Est : est
 *
 * */

#include <adc.h>
#include <distSensor_driver.h>
#include "tim.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "DCMotor_driver.h"

// Number of channels in each ADC
#define ADC1_CHANNEL_COUNT  2  /**< Number of channels for ADC1 */
#define ADC2_CHANNEL_COUNT  2  /**< Number of channels for ADC2 */

// Flags to indicate ADC readiness for each sensor
volatile uint8_t adc_ready_adc1_west = 0; /**< Flag for ADC1 West sensor */
volatile uint8_t adc_ready_adc1_nord = 0; /**< Flag for ADC1 North sensor */
volatile uint8_t adc_ready_adc2_sud = 0;  /**< Flag for ADC2 South sensor */
volatile uint8_t adc_ready_adc2_est = 0;  /**< Flag for ADC2 East sensor */

// DistSensor handles for the four sensors
DistSensor_handle_t Sens_N; /**< Handle for North sensor */
DistSensor_handle_t Sens_S; /**< Handle for South sensor */
DistSensor_handle_t Sens_W; /**< Handle for West sensor */
DistSensor_handle_t Sens_E; /**< Handle for East sensor */

// Global flag to indicate if the robot is braking to avoid conflicts with speed setting
volatile uint8_t is_robot_braking; /**< Flag to indicate if the robot is braking */

// ADC value and DMA buffers for the sensors
uint32_t adc_value; /**< Current ADC value */
uint32_t adc1_dma_buffer[ADC1_CHANNEL_COUNT]; /**< DMA buffer for ADC1 */
uint32_t adc2_dma_buffer[ADC2_CHANNEL_COUNT]; /**< DMA buffer for ADC2 */

// Values for each sensor position
uint32_t value_west; /**< ADC value for West sensor (ADC1 Channel 5) */
uint32_t value_nord; /**< ADC value for North sensor (ADC1 Channel 11) */
uint32_t value_est;  /**< ADC value for East sensor (ADC2 Channel 15) */
uint32_t value_sud;  /**< ADC value for South sensor (ADC2 Channel 12) */

/**
 * @brief Initializes ADC1 and ADC2 in DMA mode.
 *
 * This function initializes the ADCs for the four distance sensors (West, North,
 * East, South) using DMA to continuously sample the sensor data. It also starts
 * a timer to trigger periodic updates.
 */
void distSensor_initADC_DMA(void)
{
    // Start ADC1 and ADC2 in DMA mode
    HAL_ADC_Start_DMA(&hadc1, adc1_dma_buffer, ADC1_CHANNEL_COUNT);
    HAL_ADC_Start_DMA(&hadc2, adc2_dma_buffer, ADC2_CHANNEL_COUNT);

    // Start a timer to trigger periodic updates
    HAL_TIM_Base_Start(&htim6);
}

/**
 * @brief Initializes a distance sensor handle.
 *
 * This function initializes the state of a distance sensor by setting its
 * detection status to "floor", its ADC value to zero, and its status to "no data".
 *
 * @param sensor Pointer to the DistSensor_handle_t structure for the sensor.
 * @param sensor_position The position of the sensor (North, South, West, East).
 */
void distSensor_Init_Sensor(DistSensor_handle_t * sensor, DistSensor_Position_t sensor_position)
{
    sensor->adc_ready = DistSensor_ADC_STATUS_NO_DATA; /**< Set ADC status to no data */
    sensor->sensor_position = sensor_position; /**< Set the sensor's position */
    sensor->sensor_detection_status = DistSensor_DETECT_FLOOR; /**< Set default detection status */
    sensor->sensor_value = 0; /**< Set the sensor value to 0 */
}

/**
 * @brief Reads the ADC value of the distance sensor using polling.
 *
 * This function triggers the ADC conversion and waits for the conversion
 * to complete, then returns the converted ADC value.
 *
 * @param hadc Pointer to the ADC handle.
 * @return The ADC value after conversion.
 */
uint32_t distSensor_ReadADC(ADC_HandleTypeDef* hadc)
{
    HAL_ADC_Start(&hadc2); /**< Start ADC conversion */
    HAL_ADC_PollForConversion(&hadc2, HAL_MAX_DELAY); /**< Wait for conversion to complete */
    adc_value = HAL_ADC_GetValue(&hadc2); /**< Retrieve the ADC conversion result */

    return adc_value; /**< Return the ADC value */
}
/**
 * @brief Reads values from ADC1 and ADC2 using DMA and updates sensor status.
 *
 * This function checks the `adc_ready` status for each sensor and reads the
 * corresponding ADC values from the DMA buffers. It then determines whether
 * the sensor is detecting the floor or void based on the threshold value.
 * After processing the ADC values, the sensor's `adc_ready` status is reset.
 *
 * @return HAL_OK if the function executes successfully.
 */
HAL_StatusTypeDef distSensor_ReadADC_DMA(void)
{
    // Processing the West sensor
    if (Sens_W.adc_ready == DistSensor_ADC_STATUS_READY)
    {
        Sens_W.sensor_value = adc1_dma_buffer[0]; // ADC1 Channel 5: West sensor value
        // Check if the sensor is detecting the floor or void
        if(Sens_W.sensor_value > VOID_TRESHOLD)
        {
            Sens_W.sensor_detection_status = DistSensor_DETECT_FLOOR;
        }
        else if(Sens_W.sensor_value != 0)
        {
            Sens_W.sensor_detection_status = DistSensor_DETECT_VOID;
        }
        Sens_W.adc_ready = DistSensor_ADC_STATUS_NO_DATA; // Reset ADC ready status
    }

    // Processing the North sensor
    if (Sens_N.adc_ready == DistSensor_ADC_STATUS_READY)
    {
        Sens_N.sensor_value = adc1_dma_buffer[1]; // ADC1 Channel 11: North sensor value
        // Check if the sensor is detecting the floor or void
        if(Sens_N.sensor_value > VOID_TRESHOLD)
        {
            Sens_N.sensor_detection_status = DistSensor_DETECT_FLOOR;
        }
        else if(Sens_N.sensor_value != 0)
        {
            Sens_N.sensor_detection_status = DistSensor_DETECT_VOID;
        }
        Sens_N.adc_ready = DistSensor_ADC_STATUS_NO_DATA; // Reset ADC ready status
    }

    // Processing the South sensor
    if (Sens_S.adc_ready == DistSensor_ADC_STATUS_READY)
    {
        Sens_S.sensor_value = adc2_dma_buffer[0]; // ADC2 Channel 12: South sensor value
        // Check if the sensor is detecting the floor or void
        if(Sens_S.sensor_value > VOID_TRESHOLD)
        {
            Sens_S.sensor_detection_status = DistSensor_DETECT_FLOOR;
        }
        else if(Sens_S.sensor_value != 0)
        {
            Sens_S.sensor_detection_status = DistSensor_DETECT_VOID;
        }
        Sens_S.adc_ready = DistSensor_ADC_STATUS_NO_DATA; // Reset ADC ready status
    }

    // Processing the East sensor
    if (Sens_E.adc_ready == DistSensor_ADC_STATUS_READY)
    {
        Sens_E.sensor_value = adc2_dma_buffer[1]; // ADC2 Channel 15: East sensor value
        // Check if the sensor is detecting the floor or void
        if(Sens_E.sensor_value > VOID_TRESHOLD)
        {
            Sens_E.sensor_detection_status = DistSensor_DETECT_FLOOR;
        }
        else if(Sens_E.sensor_value != 0)
        {
            Sens_E.sensor_detection_status = DistSensor_DETECT_VOID;
        }
        Sens_E.adc_ready = DistSensor_ADC_STATUS_NO_DATA; // Reset ADC ready status
    }

    return HAL_OK; // Return HAL_OK to indicate success
}

/**
 * @brief Callback function for ADC1 and ADC2 conversion completion.
 *
 * This callback function is triggered when an ADC conversion is completed.
 * It sets the `adc_ready` status for each sensor to indicate that new sensor
 * data is available for processing.
 *
 * @param hadc Pointer to the ADC handle that triggered the callback.
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    // If the callback was triggered by ADC1
    if (hadc->Instance == ADC1)
    {
        Sens_W.adc_ready = DistSensor_ADC_STATUS_READY; // Set West sensor to ready
        Sens_N.adc_ready = DistSensor_ADC_STATUS_READY; // Set North sensor to ready
    }

    // If the callback was triggered by ADC2
    if (hadc->Instance == ADC2)
    {
        Sens_S.adc_ready = DistSensor_ADC_STATUS_READY; // Set South sensor to ready
        Sens_E.adc_ready = DistSensor_ADC_STATUS_READY; // Set East sensor to ready
    }
}



/**
 * @brief Main task for handling distance sensor data and robot braking logic.
 *
 * The `distSensor_Task` function continuously reads the ADC values from the distance sensors
 * using DMA. It checks the sensor status for each of the four directions (North, South, East, West).
 * If any sensor detects void (no surface detected), the robot's motors are braked.
 * The task also prints messages to the console indicating which direction has detected void.
 *
 * The task runs in an infinite loop and contains a small delay to prevent excessive CPU usage.
 */
void distSensor_Task(void *unused)
{
    // Initialize the distance sensors
    distSensor_Init_Sensor(&Sens_N, DistSensor_POSITION_NORD);
    distSensor_Init_Sensor(&Sens_W, DistSensor_POSITION_WEST);
    distSensor_Init_Sensor(&Sens_S, DistSensor_POSITION_SUD);
    distSensor_Init_Sensor(&Sens_E, DistSensor_POSITION_EAST);
    distSensor_initADC_DMA(); // Initialize ADC and DMA for sensor data acquisition

    for (;;)
    {
        // Read the ADC values via DMA for all sensors
        distSensor_ReadADC_DMA();

        // Check if any of the sensors detect void and apply braking if necessary
        if (Sens_N.sensor_detection_status == DistSensor_DETECT_VOID ||
            Sens_E.sensor_detection_status == DistSensor_DETECT_VOID ||
            Sens_W.sensor_detection_status == DistSensor_DETECT_VOID ||
            Sens_S.sensor_detection_status == DistSensor_DETECT_VOID)
        {
            // If the robot is not already braking, initiate braking
            if (!is_robot_braking)
            {
                DCMotor_Brake(&DualDrive_handle.motor_left);   // Brake left motor
                DCMotor_Brake(&DualDrive_handle.motor_right);  // Brake right motor
                printf("Braking !! Resetting speed to 0 after 5 sec\r\n");
                is_robot_braking = 1;  // Set the braking flag
            }
        }

        // Print messages to indicate which sensor detects void
        if (Sens_N.sensor_detection_status == DistSensor_DETECT_VOID)
        {
            printf("North detects void\r\n");
        }
        if (Sens_W.sensor_detection_status == DistSensor_DETECT_VOID)
        {
            printf("West detects void\r\n");
        }
        if (Sens_S.sensor_detection_status == DistSensor_DETECT_VOID)
        {
            printf("South detects void\r\n");
        }
        if (Sens_E.sensor_detection_status == DistSensor_DETECT_VOID)
        {
            printf("East detects void\r\n");
        }

        // Add a small delay to prevent excessive CPU usage
        vTaskDelay(50);
    }
}

/**
 * @brief Creates and starts the distance sensor task.
 *
 * This function creates the `distSensor_Task` with a priority defined by
 * `distSensor_Task_Priority` and a stack size of 128 bytes.
 */
void distSensor_TaskCreate(void* unused)
{
    // Create the task to manage the distance sensors
    xTaskCreate(distSensor_Task, "distSensor_task", 128, NULL, distSensor_Task_Priority, NULL);
}






