///*
// * distanceSensor.c
// *
// *  Created on: Sep 22, 2024
// *      Author: exysta
// *  Code pour les capteurs de distance
// *  Based on https://github.com/DrGFreeman/SharpDistSensor
// *
// *
// */


#include <adc.h>
#include <distSensor_driver.h>
#include "tim.h"



uint32_t adc_value = 0;
//uint32_t falling_dist = 0;
volatile uint8_t adc_ready = 0; // Flag pour indiquer que la valeur est prête à être afficher



void distSensor_initADC_DMA(ADC_HandleTypeDef* hadc, uint32_t channel)
{
//    ADC_ChannelConfTypeDef sConfig = {0}; // Configuration de canal ADC
//
//    sConfig.Channel = channel;
//    sConfig.Rank = ADC_REGULAR_RANK_1; // Premier rang pour ce canal
//    sConfig.SamplingTime = ADC_SAMPLETIME_47CYCLES_5; // Exemple de temps d'échantillonnage, ajustable
//
//    // Configurer le canal sélectionné
//    if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK)
//    {
//        return;
//    }

	HAL_ADC_Start_DMA(&hadc2, &adc_value, 1);
	HAL_TIM_Base_Start(&htim6);
}


uint32_t distSensor_ReadADC(ADC_HandleTypeDef* hadc)
{
	HAL_ADC_Start(&hadc2);
	HAL_ADC_PollForConversion(&hadc2, HAL_MAX_DELAY);
	adc_value = HAL_ADC_GetValue(&hadc2);

	return adc_value;   // Return the ADC value
}

uint32_t distSensor_ReadADC_DMA(ADC_HandleTypeDef* hadc)
{
    if (adc_ready == 1) // Vérifie si une nouvelle valeur est prête
    {
        //distance = falling_dist; // Donne la dernière valeur valide
    	return adc_value;
        adc_ready = 0;            // Réinitialise le flag
        //return 1;
    }
    return 1;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if (ADC2 == hadc->Instance)
	{
		//adc_ready = 1;
		// Si la valeur ADC est supérieure à 1000 (la distance avec le capteur est élevée, mettre le flag à 1

        //falling_dist = adc_value; // Copie de la valeur brute
        if (adc_value < 1000)    // ne garde que les valeurs < 1000
        {
            adc_ready = 1;        // Active le flag pour indiquer que la valeur est prête
        }
    }
}






