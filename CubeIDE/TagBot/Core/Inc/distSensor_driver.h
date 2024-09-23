/*
 * distanceSensor.h
 *
 *  Created on: Sep 22, 2024
 *      Author: exysta
 */

#ifndef INC_DISTSENSOR_DRIVER_H_
#define INC_DISTSENSOR_DRIVER_H_

#define WINDOW_SIZE 5 // Size of the moving average window

//coeff used are from using a 7 degree polynomial fitting of the datasheet sensor values
float coeffs[] = { -23.67560604  , 299.24647087 ,-1567.5164766  , 4408.38686151,
					 -7211.95874456  ,6959.22725692 ,-3844.96742449 , 1114.78386014};



float Vmin = 61;
float Vmax = 614;

// Global variables for the moving average filter
float readings[WINDOW_SIZE] = {0}; // Circular buffer
int index = 0; // Current index in the buffer
int count = 0; // Number of readings received

uint16_t distSensor_ReadADCChannel(ADC_HandleTypeDef* hadc, uint32_t channel);
uint16_t distSensor_ConvertVoltToDistance(uint32_t sensVal);
void constrain(uint16_t * sensVal);
float moving_average(float new_reading);

#endif /* INC_DISTSENSOR_DRIVER_H_ */
