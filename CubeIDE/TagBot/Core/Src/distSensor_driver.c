/*
 * distanceSensor.c
 *
 *  Created on: Sep 22, 2024
 *      Author: exysta
 */

#include <distSensor_driver.h>

ADC_HandleTypeDef hadc1;  ///< ADC handle for the first ADC instance
ADC_HandleTypeDef hadc2;  ///< ADC handle for the second ADC instance

/**
 * @brief Reads the ADC value from a specified channel.
 *
 * This function configures the specified ADC channel, starts the conversion,
 * waits for it to complete, retrieves the converted value, and then stops the ADC.
 *
 * @param hadc Pointer to the ADC handle structure.
 * @param channel The ADC channel to read from.
 * @return The ADC converted value.
 */
uint16_t distSensor_ReadADCChannel(ADC_HandleTypeDef* hadc, uint32_t channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};  ///< Structure for ADC channel configuration

    // Configure the ADC channel
    sConfig.Channel = channel;                        ///< Select the desired channel
    sConfig.Rank = ADC_REGULAR_RANK_1;               ///< Set as the first rank in regular conversion
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;  ///< Adjust sampling time as needed

    if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK)
    {
        // Configuration Error
        Error_Handler();
    }

    // Start the ADC conversion
    HAL_ADC_Start(hadc);

    // Poll for conversion complete
    HAL_ADC_PollForConversion(hadc, HAL_MAX_DELAY);

    // Get the ADC converted value
    uint16_t adcValue = HAL_ADC_GetValue(hadc);

    // Stop the ADC
    HAL_ADC_Stop(hadc);

    return adcValue;  ///< Return the ADC value
}

/**
 * @brief Converts the sensor value (voltage) to distance.
 *
 * This function takes a sensor value and applies a polynomial fit
 * to convert it into a distance value. It also applies a moving average
 * filter to smooth out the result.
 *
 * @param sensVal The raw sensor value (voltage).
 * @return The calculated distance after conversion and filtering.
 */
uint16_t distSensor_ConvertVoltToDistance(uint32_t sensVal)
{
    // Constrain sensor values to remain within set min-max range
    sensVal = constrain(sensVal);

    uint16_t dist = 0;  ///< Variable to store calculated distance

    // Calculate distance from polynomial fit function
    dist += coeffs[5] * pow(sensVal, 5);
    dist += coeffs[4] * pow(sensVal, 4);
    dist += coeffs[3] * pow(sensVal, 3);
    dist += coeffs[2] * pow(sensVal, 2);
    dist += coeffs[1] * sensVal;
    dist += coeffs[0];

    // Apply the moving average filter
    float filtered_dist = moving_average((float)dist);

    return filtered_dist;  ///< Return the filtered distance
}

/**
 * @brief Constrains the sensor value within the specified limits.
 *
 * This function ensures that the sensor value does not exceed
 * the maximum or minimum threshold values.
 *
 * @param sensVal Pointer to the sensor value to be constrained.
 */
void constrain(uint16_t * sensVal)
{
    if (*sensVal > Vmax)
    {
        *sensVal = Vmax;  ///< Set to maximum if exceeded
    }
    if (*sensVal < Vmin)
    {
        *sensVal = Vmin;  ///< Set to minimum if below threshold
    }
}

/**
 * @brief Calculates the moving average of sensor readings.
 *
 * This function updates a circular buffer with the new reading,
 * calculates the sum of the readings, and returns the average.
 *
 * @param new_reading The new sensor reading to add to the average.
 * @return The calculated moving average of the readings.
 */
float moving_average(float new_reading)
{
    // Update the buffer with the new reading
    readings[index] = new_reading;
    index = (index + 1) % WINDOW_SIZE;  ///< Move to the next index in a circular manner

    // Increment the count of readings
    if (count < WINDOW_SIZE) {
        count++;  ///< Increase count until it reaches the window size
    }

    // Calculate the sum of the current readings in the buffer
    float sum = 0;
    for (int i = 0; i < count; i++) {
        sum += readings[i];  ///< Accumulate the readings
    }

    // Return the moving average
    return sum / count;  ///< Return the average
}
