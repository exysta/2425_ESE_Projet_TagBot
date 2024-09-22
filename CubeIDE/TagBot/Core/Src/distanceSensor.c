/*
 * distanceSensor.c
 *
 *  Created on: Sep 22, 2024
 *      Author: exysta
 */
#include "distanceSensor.h"

ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;



uint32_t distanceSensor_ReadADCChannel(ADC_HandleTypeDef* hadc, uint32_t channel)
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
    uint32_t adcValue = HAL_ADC_GetValue(hadc);

    // Stop the ADC
    HAL_ADC_Stop(hadc);

    return adcValue;   // Return the ADC value
}

void distanceSensor_ConvertVoltToDistance
