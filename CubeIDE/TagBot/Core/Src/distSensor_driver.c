/*
 * distanceSensor.c
 *
 *  Created on: Sep 22, 2024
 *      Author: marie
 *  Code pour les capteurs de distance
 *
 *
 *
 */

/*
 * CODE POUR LES CAPTEURS DE DISTANCES
 *
 * ********ADC1**********
 * - Capteur West : west
 * - Capteur Nord : nord
 *
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



#define ADC1_CHANNEL_COUNT  2  // Nombre de canaux pour ADC1
#define ADC2_CHANNEL_COUNT  2  // Nombre de canaux pour ADC2


volatile uint8_t adc_ready_adc1_west = 0;
volatile uint8_t adc_ready_adc1_nord = 0;
volatile uint8_t adc_ready_adc2_sud = 0;
volatile uint8_t adc_ready_adc2_est = 0;

uint32_t adc_value;

uint32_t adc1_dma_buffer[ADC1_CHANNEL_COUNT]; // Buffer DMA pour ADC1
uint32_t adc2_dma_buffer[ADC2_CHANNEL_COUNT]; // Buffer DMA pour ADC2

uint32_t value_west;  				// Valeur de west (ADC1 Channel 5)
uint32_t value_nord; 				// Valeur de nord (ADC1 Channel 11)
uint32_t value_est; 				// Valeur de est (ADC2 Channel 15)
uint32_t value_sud; 				// Valeur de sud (ADC2 Channel 12)


// function to init ADC1 and ADC2 (DMA)
void distSensor_initADC_DMA(void)
{
	distSensor_TaskCreate(NULL);


	HAL_ADC_Start_DMA(&hadc1, adc1_dma_buffer, ADC1_CHANNEL_COUNT);
	HAL_ADC_Start_DMA(&hadc2, adc2_dma_buffer, ADC2_CHANNEL_COUNT);
	HAL_TIM_Base_Start(&htim6);


}

// Function to dread ADC (polling)
uint32_t distSensor_ReadADC(ADC_HandleTypeDef* hadc)
{
	HAL_ADC_Start(&hadc2);
	HAL_ADC_PollForConversion(&hadc2, HAL_MAX_DELAY);
	adc_value = HAL_ADC_GetValue(&hadc2);

	return adc_value;   // Return the ADC value
}

// Function to read value on ADC1 and ADC2 (DMA)
uint32_t distSensor_ReadADC_DMA(void)
{
	value_west = adc1_dma_buffer[0];    		// Valeur de west (ADC1 Channel 5)
	value_nord = adc1_dma_buffer[1];   			// Valeur de nord (ADC1 Channel 11)
	value_est = adc2_dma_buffer[0];   			// Valeur de est (ADC2 Channel 15)
	value_sud = adc2_dma_buffer[1];   			// Valeur de sud (ADC2 Channel 12)

	if (adc_ready_adc1_west == 1)
	{
		adc_ready_adc1_west = 0;
		return value_est;
	}

	else if (adc_ready_adc1_nord == 1)
	{
		adc_ready_adc1_nord = 0;
		return value_nord;
	}

	if (adc_ready_adc2_est == 1)
	{
		adc_ready_adc2_est = 0;
		return value_est;
	}

	if (adc_ready_adc2_sud == 1)
	{
		adc_ready_adc2_sud = 0;
		return value_sud;
	}

	else{
		return 1;
	}




}



//Call Back for ADC1 and ADC2
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if (hadc->Instance == ADC1)
	{

		if (value_west < 1000 )
		{
			adc_ready_adc1_west = 1;
		}
		else if (value_nord <1000)
		{
			adc_ready_adc1_nord = 1;
		}

	}

	if (hadc->Instance == ADC2)
	{
		if (value_sud < 1000 )
		{
			adc_ready_adc2_sud = 1;
		}
		else if (value_est <1000)
		{
			adc_ready_adc2_est = 1;
		}

	}
}



//
void distSensor_Task(void *unused){
	for (;;){
		uint32_t distance = distSensor_ReadADC_DMA();

		if (distance == 1){
			printf("error\r\n");
		}
		else {

			printf("Tache Capteur detect vide, %lu\r\n",distance );
		}
		vTaskDelay(100);


	}
}

void distSensor_TaskCreate(void*unused){
	xTaskCreate(distSensor_Task, "distSensor_task", 128, NULL, 23, NULL);
}






