/*
 * encoder.c
 *
 *  Created on: Dec 11, 2024
 *      Author: charotte
 */

#include "tim.h"
#include "encoder.h"
#include "usart.h"

//DCMOTOR_encoder_handle_t motor_left;
//DCMOTOR_encoder_handle_t motor_right;
//uint32_t timer_arr;
//
//// encoder
//
//HAL_StatusTypeDef encoder_init(DCMOTOR_encoder_handle_t *motor_handle ,TIM_HandleTypeDef * encoder_tim)
//{
//	motor_handle->encoder_tim = encoder_tim;
//	motor_handle->measured_rpm = 0;
//	motor_handle->speed = 0;
//	motor_handle->encoder_counter = 0;
//	//on récupère la valeur de l'arr pour le calcul de la vitesse plus tard
//	timer_arr = __HAL_TIM_GET_AUTORELOAD(encoder_tim);
//	//on set le counter à 0
//	__HAL_TIM_SET_COUNTER(encoder_tim, 0);
//	HAL_TIM_Encoder_Start_IT(encoder_tim, TIM_CHANNEL_ALL);
//	return HAL_OK;
//
//}
//
////l'idée c'est d'appeler cette fonction toutes les secondes
////pour cela on peut faire un callback sur un timer qui compte toutes les secondes
////on calcul le nombre de pulse en 1 seconde. on obtient donc le nombre de pulse par seconde.
////on multiplie ce nombre par 60 pour avoir le nombre de pulse par minute
////on a que pulse par minute/ENCODER_PPR = nombre de tour du moteur par minute
//
//HAL_StatusTypeDef DCMOTOR_calculate_motor_speed(DCMOTOR_encoder_handle_t *motor_handle)
//{
//	uint32_t current_counter = __HAL_TIM_GET_COUNTER(motor_handle->encoder_tim);
//	uint32_t previous_counter  =	motor_handle->encoder_counter;
//
//	int32_t delta_count;
//
//	// Handle counter overflow/underflow
//	if (current_counter < previous_counter) {
//		delta_count = (timer_arr + current_counter) - previous_counter;
//	} else {
//		delta_count = current_counter - previous_counter;
//	}
//
//	float pulses_per_second = delta_count;
//	float motor_rpm = pulses_per_second * 60.0/ENCODER_PPR;
//	motor_handle->measured_rpm = motor_rpm;
//
//	// Update last count
//	motor_handle->encoder_counter = current_counter;
//	motor_handle->measured_direction = __HAL_TIM_IS_TIM_COUNTING_DOWN(motor_handle->encoder_tim);
//	// Calculate speed (RPM)
//	return HAL_OK;
//}
//
//void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
//{
//	if(htim == henc1.htim)
//	{
//		henc1.value = __HAL_TIM_GET_COUNTER(htim);
//		henc1.it = 1;
//	}
//}
