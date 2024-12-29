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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


#define ADC1_CHANNEL_COUNT  2  // Nombre de canaux pour ADC1
#define ADC2_CHANNEL_COUNT  2  // Nombre de canaux pour ADC2


volatile uint8_t adc_ready_adc1_west = 0;
volatile uint8_t adc_ready_adc1_nord = 0;
volatile uint8_t adc_ready_adc2_sud = 0;
volatile uint8_t adc_ready_adc2_est = 0;

DistSensor_handle_t Sens_N;
DistSensor_handle_t Sens_S;
DistSensor_handle_t Sens_W;
DistSensor_handle_t Sens_E;

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

	HAL_ADC_Start_DMA(&hadc1, adc1_dma_buffer, ADC1_CHANNEL_COUNT);
	HAL_ADC_Start_DMA(&hadc2, adc2_dma_buffer, ADC2_CHANNEL_COUNT);
	HAL_TIM_Base_Start(&htim6);


}

void distSensor_Init_Sensor(DistSensor_handle_t * sensor,DistSensor_Position_t sensor_position)
{
	sensor->adc_ready = DistSensor_ADC_STATUS_NO_DATA;
	sensor->sensor_position = sensor_position;
	sensor->sensor_detection_status = DistSensor_DETECT_FLOOR;
	sensor->sensor_value = 0;
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
HAL_StatusTypeDef distSensor_ReadADC_DMA(void)
{

	if (Sens_W.adc_ready == DistSensor_ADC_STATUS_READY)
	{
		Sens_W.sensor_value = adc1_dma_buffer[0];    		// Valeur de west (ADC1 Channel 5)
		if(Sens_W.sensor_value > VOID_TRESHOLD)
		{
			Sens_W.sensor_detection_status = DistSensor_DETECT_FLOOR;
		}
		else if(Sens_W.sensor_value != 0)

		{
			Sens_W.sensor_detection_status = DistSensor_DETECT_VOID;
		}
		Sens_W.adc_ready = DistSensor_ADC_STATUS_NO_DATA;
	}

	if (Sens_N.adc_ready == DistSensor_ADC_STATUS_READY)
	{
		Sens_N.sensor_value = adc1_dma_buffer[1];   			// Valeur de nord (ADC1 Channel 11)
		if(Sens_N.sensor_value > VOID_TRESHOLD)
		{
			Sens_N.sensor_detection_status = DistSensor_DETECT_FLOOR;
		}
		else if(Sens_N.sensor_value != 0)
		{
			Sens_N.sensor_detection_status = DistSensor_DETECT_VOID;
		}
		Sens_N.adc_ready = DistSensor_ADC_STATUS_NO_DATA;
	}



	if (Sens_S.adc_ready == DistSensor_ADC_STATUS_READY)
	{
		Sens_S.sensor_value = adc2_dma_buffer[0];   			// Valeur de sud (ADC2 Channel 12)
		if(Sens_S.sensor_value > VOID_TRESHOLD)
		{
			Sens_S.sensor_detection_status = DistSensor_DETECT_FLOOR;
		}
		else if(Sens_S.sensor_value != 0)

		{
			Sens_S.sensor_detection_status = DistSensor_DETECT_VOID;
		}
		Sens_S.adc_ready = DistSensor_ADC_STATUS_NO_DATA;
	}

	if (Sens_E.adc_ready == DistSensor_ADC_STATUS_READY)
	{
		Sens_E.sensor_value = adc2_dma_buffer[1];   			// Valeur de est (ADC2 Channel 15)
		if(Sens_E.sensor_value > VOID_TRESHOLD)
		{
			Sens_E.sensor_detection_status = DistSensor_DETECT_FLOOR;
		}
		else if(Sens_E.sensor_value != 0)

		{
			Sens_E.sensor_detection_status = DistSensor_DETECT_VOID;
		}
		Sens_E.adc_ready = DistSensor_ADC_STATUS_NO_DATA;
	}
	return HAL_OK;
}



//Call Back for ADC1 and ADC2
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if (hadc->Instance == ADC1)
	{
		Sens_W.adc_ready = DistSensor_ADC_STATUS_READY;
		Sens_N.adc_ready = DistSensor_ADC_STATUS_READY;
	}

	if (hadc->Instance == ADC2)
	{
		Sens_S.adc_ready = DistSensor_ADC_STATUS_READY;
		Sens_E.adc_ready = DistSensor_ADC_STATUS_READY;
	}
}



//
void distSensor_Task(void *unused)
{
//	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
//	HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
	distSensor_Init_Sensor(&Sens_N, DistSensor_POSITION_NORD);
	distSensor_Init_Sensor(&Sens_W, DistSensor_POSITION_WEST);
	distSensor_Init_Sensor(&Sens_S, DistSensor_POSITION_SUD);
	distSensor_Init_Sensor(&Sens_E, DistSensor_POSITION_EAST);
	distSensor_initADC_DMA();

	for (;;)
	{
//		while (Sens_W.adc_ready != DistSensor_ADC_STATUS_READY &&
//		    Sens_N.adc_ready != DistSensor_ADC_STATUS_READY &&
//		    Sens_S.adc_ready != DistSensor_ADC_STATUS_READY &&
//		    Sens_E.adc_ready != DistSensor_ADC_STATUS_READY) {
//		    vTaskDelay(20);  // Small delay to prevent hogging the CPU
//		}

		distSensor_ReadADC_DMA();

		if(Sens_N.sensor_detection_status == DistSensor_DETECT_VOID)
		{
			printf("North detect void\r\n");
		}
		if(Sens_W.sensor_detection_status == DistSensor_DETECT_VOID)
		{
			printf("West detect void\r\n");
		}
		if(Sens_S.sensor_detection_status == DistSensor_DETECT_VOID)
		{
			printf("Sud detect void\r\n");
		}
		if(Sens_E.sensor_detection_status == DistSensor_DETECT_VOID)
		{
			printf("East detect void\r\n");
		}

		vTaskDelay(100);


	}
}

void distSensor_TaskCreate(void*unused)
{
	xTaskCreate(distSensor_Task, "distSensor_task", 128, NULL,  distSensor_Task_Priority, NULL);
}






