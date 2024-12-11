///*
// * distanceSensor.c
// *
// *  Created on: Sep 22, 2024
// *      Author: marie &charlotte
// *  Code pour les capteurs de distance
// *
// *
// *
// */


#include <adc.h>
#include <distSensor_driver.h>
#include "tim.h"

#define ADC1_CHANNEL_COUNT  2  // Nombre de canaux pour ADC1
#define ADC2_CHANNEL_COUNT  2  // Nombre de canaux pour ADC2

volatile uint8_t adc_ready_adc1 = 0; // Flag pour indiquer que la valeur est prête à être afficher
volatile uint8_t adc_ready_adc2 = 0;

uint32_t adc_value;

uint32_t adc1_dma_buffer[ADC1_CHANNEL_COUNT]; // Buffer DMA pour ADC1
uint32_t adc2_dma_buffer[ADC2_CHANNEL_COUNT]; // Buffer DMA pour ADC2

uint32_t value_pb5;  				// Valeur de PB5 (ADC1 Channel 5)
uint32_t value_pb11; 				// Valeur de PB11 (ADC1 Channel 11)
uint32_t value_pb15; 				// Valeur de PB15 (ADC2 Channel 15)
uint32_t value_pb12; 				// Valeur de PB12 (ADC2 Channel 12)



void distSensor_initADC_DMA(void)
{


    HAL_ADC_Start_DMA(&hadc1, adc1_dma_buffer, ADC1_CHANNEL_COUNT);
    HAL_ADC_Start_DMA(&hadc2, adc2_dma_buffer, ADC2_CHANNEL_COUNT);
	HAL_TIM_Base_Start(&htim6);



}


uint32_t distSensor_ReadADC(ADC_HandleTypeDef* hadc)
{
	HAL_ADC_Start(&hadc2);
	HAL_ADC_PollForConversion(&hadc2, HAL_MAX_DELAY);
	adc_value = HAL_ADC_GetValue(&hadc2);

	return adc_value;   // Return the ADC value
}


uint32_t distSensor_ReadADC_DMA(uint32_t* buffer)
{
	value_pb5 = adc1_dma_buffer[0];    // Valeur de PB5 (ADC1 Channel 5)
	value_pb11 = adc1_dma_buffer[1];   // Valeur de PB11 (ADC1 Channel 11)
	value_pb15 = adc2_dma_buffer[0];   // Valeur de PB15 (ADC2 Channel 15)
	value_pb12 = adc2_dma_buffer[1];   // Valeur de PB12 (ADC2 Channel 12)

    if (adc_ready_adc1 == 1) // Vérifie si une nouvelle valeur est prête
    {

        adc_ready_adc1 = 0;            // Réinitialise le flag

        buffer[0] = adc1_dma_buffer[0];
        buffer[1] = adc1_dma_buffer[1];

    }

    else if (adc_ready_adc2 == 1) // Vérifie si une nouvelle valeur est prête
    {

        adc_ready_adc2 = 0;            // Réinitialise le flag

        buffer[2] = adc2_dma_buffer[0];
        buffer[3] = adc2_dma_buffer[1];


    }

    else {
        buffer[0] = 777; // Valeur par défaut si les données ne sont pas prêtes
        buffer[1] = 777;
        buffer[2] = 777;
        buffer[3] = 777;

    }




}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if (hadc->Instance == ADC1)
	{

//        if (value_pb5 < 1000 && value_pb11<1000 )    // ne garde que les valeurs < 1000
//        {
//            adc_ready_adc1 = 1;        // Active le flag pour indiquer que la valeur est prête
//        }

		adc_ready_adc1 =1;

    }

	if (hadc->Instance == ADC2)
	{
//		if ((value_pb15 < 1000)&& (value_pb12<1000))    // ne garde que les valeurs < 1000
//		{
//			adc_ready_adc2 = 1;        // Active le flag pour indiquer que la valeur est prête
//		}

		adc_ready_adc2 =1;

	}
}






