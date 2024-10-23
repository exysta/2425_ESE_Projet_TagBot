/*
 * distanceSensor.c
 *
 *  Created on: Sep 22, 2024
 *      Author: exysta
 *  Code pour les capteurs de distance
 *
 */

//ATTENTION CODE FAUX POUR LE MOMENT

#include <adc.h>
#include <distSensor_driver.h>

ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;


uint16_t distSensor_ReadADCChannel(ADC_HandleTypeDef* hadc, uint32_t channel)
{
	ADC_ChannelConfTypeDef sConfig = {0};

	// Configure the ADC channel
	sConfig.Channel = channel;             // Select the desired channel
	sConfig.Rank = ADC_REGULAR_RANK_1;     // Set as the first rank in regular conversion
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;  // Adjust sampling time as needed

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

	return adcValue;   // Return the ADC value
}

// Return the measured distance
uint16_t distSensor_ConvertVoltToDistance(uint32_t sensVal)
{
	// Constrain sensor values to remain within set min-max range
	sensVal = constrain(&sensVal);

	uint16_t dist = 0;

	// Calculate distance from polynomial fit function
	dist += coeffs[5] * pow(sensVal, 5);
	dist += coeffs[4] * pow(sensVal, 4);
	dist += coeffs[3] * pow(sensVal, 3);
	dist += coeffs[2] * pow(sensVal, 2);
	dist += coeffs[1] * sensVal;
	dist += coeffs[0];

	// Apply the moving average filter
	float filtered_dist = moving_average((float)dist);


	return filtered_dist;
}

void constrain(uint16_t * sensVal) {
    if (*sensVal > Vmax) {
        *sensVal = Vmax;
    }
    if (*sensVal < Vmin) {
        *sensVal = Vmin;
    }
}

// Function to calculate the moving average
float moving_average(float new_reading)
{
	// Update the buffer with the new reading
	readings[index] = new_reading;
	index = (index + 1) % WINDOW_SIZE; // Move to the next index in a circular manner

	// Increment the count of readings
	if (count < WINDOW_SIZE) {
		count++;
	}

	// Calculate the sum of the current readings in the buffer
	float sum = 0;
	for (int i = 0; i < count; i++) {
		sum += readings[i];
	}

	// Return the moving average
	return sum / count;
}

