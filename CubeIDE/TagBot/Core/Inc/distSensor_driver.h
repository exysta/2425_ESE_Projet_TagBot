/**
 * @file distanceSensor.h
 * @brief Header file for distance sensor driver.
 *
 * This file contains definitions, constants, and function prototypes for
 * the distance sensor driver. It provides functionalities for reading
 * sensor data, converting voltage readings to distance, and applying a
 * moving average filter to smooth the readings.
 *
 * @date Created on: Sep 22, 2024
 * @author exysta
 */

#ifndef INC_DISTSENSOR_DRIVER_H_
#define INC_DISTSENSOR_DRIVER_H_

/**
 * @def WINDOW_SIZE
 * @brief Size of the moving average window.
 *
 * This constant defines the number of readings to consider for
 * calculating the moving average of the sensor data.
 */
#define WINDOW_SIZE 5

/**
 * @brief Coefficients for polynomial fitting of the sensor values.
 *
 * These coefficients are derived from a 7-degree polynomial fit based
 * on the sensor's datasheet values. They are used for converting
 * voltage readings to distance.
 */
float coeffs[] = { -23.67560604, 299.24647087, -1567.5164766, 4408.38686151,
                   -7211.95874456, 6959.22725692, -3844.96742449, 1114.78386014 };

/**
 * @brief Minimum and maximum voltage readings.
 *
 * These constants define the minimum and maximum voltage values
 * that the sensor can produce.
 */
float Vmin = 61; ///< Minimum voltage
float Vmax = 614; ///< Maximum voltage

// Global variables for the moving average filter
/**
 * @brief Circular buffer for storing sensor readings.
 *
 * This array stores the most recent readings from the sensor to
 * calculate the moving average.
 */
float readings[WINDOW_SIZE] = {0};

/**
 * @brief Current index in the circular buffer.
 *
 * This variable keeps track of the current position in the
 * circular buffer for new readings.
 */
int index = 0;

/**
 * @brief Number of readings received so far.
 *
 * This variable counts how many readings have been added to the
 * circular buffer.
 */
int count = 0;

/**
 * @brief Reads the ADC channel for the distance sensor.
 *
 * This function reads the ADC channel specified by the `channel`
 * parameter using the provided ADC handle.
 *
 * @param hadc Pointer to the ADC_HandleTypeDef structure.
 * @param channel The ADC channel to read.
 * @return The digital value read from the ADC channel.
 */
uint16_t distSensor_ReadADCChannel(ADC_HandleTypeDef* hadc, uint32_t channel);

/**
 * @brief Converts the sensor value from voltage to distance.
 *
 * This function takes a voltage reading from the sensor and
 * converts it to a distance value using the predefined polynomial
 * coefficients.
 *
 * @param sensVal The voltage value from the sensor.
 * @return The calculated distance corresponding to the voltage.
 */
uint16_t distSensor_ConvertVoltToDistance(uint32_t sensVal);

/**
 * @brief Constrains the sensor value within the minimum and maximum range.
 *
 * This function ensures that the sensor value does not exceed
 * predefined limits.
 *
 * @param sensVal Pointer to the sensor value to be constrained.
 */
void constrain(uint16_t * sensVal);

/**
 * @brief Calculates the moving average of sensor readings.
 *
 * This function takes a new reading and updates the moving average
 * using the defined circular buffer.
 *
 * @param new_reading The latest sensor reading to include in the average.
 * @return The updated moving average value.
 */
float moving_average(float new_reading);

#endif /* INC_DISTSENSOR_DRIVER_H_ */

