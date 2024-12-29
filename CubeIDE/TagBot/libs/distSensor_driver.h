/*
 * distanceSensor.h
 *
 *  Created on: Sep 22, 2024
 *      Author: marie & charlotte
 */

#ifndef INC_DISTSENSOR_DRIVER_H_
#define INC_DISTSENSOR_DRIVER_H_


#include <stddef.h>
#include <stdint.h>

#define distSensor_Task_Priority 10

typedef enum {
	DistSensor_DETECT_FLOOR = 0,

	DistSensor_DETECT_VOID = 1,

} DistSensor_Detection_Status_t;

typedef enum {
	DistSensor_ADC_STATUS_NO_DATA = 0,

    DistSensor_ADC_STATUS_READY = 1,

} DistSensor_ADC_Status_t;

typedef enum {
	DistSensor_POSITION_NORD = 0,
	DistSensor_POSITION_EAST = 1,
	DistSensor_POSITION_SUD = 2,
	DistSensor_POSITION_WEST = 3,

} DistSensor_Position_t;

typedef struct {
	DistSensor_Detection_Status_t sensor_detection_status;
    uint16_t sensor_value;
    DistSensor_Position_t sensor_position;
    volatile DistSensor_ADC_Status_t adc_ready;
} DistSensor_handle_t;

#define VOID_TRESHOLD 1000 //adc value threshold to detect void

void distSensor_initADC_DMA(void);
uint32_t distSensor_ReadADC(ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef distSensor_ReadADC_DMA(void);

void distSensor_TaskCreate(void*unused);
void distSensor_Task(void *unused);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc);

#endif /* INC_DISTSENSOR_DRIVER_H_ */
