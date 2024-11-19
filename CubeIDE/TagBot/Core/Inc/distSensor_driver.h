/*
 * distanceSensor.h
 *
 *  Created on: Sep 22, 2024
 *      Author: exysta
 */

#ifndef INC_DISTSENSOR_DRIVER_H_
#define INC_DISTSENSOR_DRIVER_H_


#include <stddef.h>
#include <stdint.h>

void distSensor_initADC_DMA(ADC_HandleTypeDef* hadc, uint32_t channel);
uint32_t distSensor_ReadADC(ADC_HandleTypeDef* hadc);
uint32_t distSensor_ReadADC_DMA(ADC_HandleTypeDef* hadc);
//void distSensor_ReadADC_DMA(ADC_HandleTypeDef* hadc, uint32_t *distance);

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc);

#endif /* INC_DISTSENSOR_DRIVER_H_ */
