/*
 * dc_motor_driver.c
 *
 *  Created on: Nov 30, 2024
 *      Author: exysta
 */

#include "tim.h"
#include "dc_motor_driver.h"

DCMOTOR_encoder_handle_t motor_left;
DCMOTOR_encoder_handle_t motor_right;
uint32_t timer_arr;

// encoder

HAL_StatusTypeDef encoder_init(DCMOTOR_encoder_handle_t *motor_handle ,TIM_HandleTypeDef * encoder_tim)
{
	motor_handle->encoder_tim = encoder_tim;
	motor_handle->measured_rpm = 0;
	motor_handle->speed = 0;
	motor_handle->encoder_counter = 0;
	//on récupère la valeur de l'arr pour le calcul de la vitesse plus tard
	timer_arr = __HAL_TIM_GET_AUTORELOAD(encoder_tim);
	//on set le counter à 0
	__HAL_TIM_SET_COUNTER(encoder_tim, 0);
	HAL_TIM_Encoder_Start_IT(encoder_tim, TIM_CHANNEL_ALL);
	return HAL_OK;

}

//l'ideé c'est d'appeler cette fonction toutes les secondes
//pour cela on peut faire un callback sur un timer qui compte toutes les secondes
//on calcul le nombre de pulse en 1 seconde. on obtient donc le nombre de pulse par seconde.
//on multiplie ce nombre par 60 pour avoir le nombre de pulse par minute
//on a que pulse par minute/ENCODER_PPR = nombre de tour du moteur par minute

HAL_StatusTypeDef DCMOTOR_calculate_motor_speed(DCMOTOR_encoder_handle_t *motor_handle)
{
	uint32_t current_counter = __HAL_TIM_GET_COUNTER(motor_handle->encoder_tim);
	uint32_t previous_counter  =	motor_handle->encoder_counter;

	int32_t delta_count;

	// Handle counter overflow/underflow
	if (current_counter < previous_counter) {
		delta_count = (timer_arr + current_counter) - previous_counter;
	} else {
		delta_count = current_counter - previous_counter;
	}

	float pulses_per_second = delta_count;
	float motor_rpm = pulses_per_second * 60.0/ENCODER_PPR;
	motor_handle->measured_rpm = motor_rpm;

	// Update last count
	motor_handle->encoder_counter = current_counter;
	motor_handle->measured_direction = __HAL_TIM_IS_TIM_COUNTING_DOWN(motor_handle->encoder_tim);
	// Calculate speed (RPM)
	return HAL_OK;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim == henc1.htim)
	{
		henc1.value = __HAL_TIM_GET_COUNTER(htim);
		henc1.it = 1;
	}
}

// PWM pour faire tourner les moteurs


PWM_HandleTypeDef pwm_handle;
uint32_t speed; // between 0-100 for motor speed. 0 is max speed in reverse, 50 is neutral and 100 is max speed in the other direction.

int control_motore

//int PWM_Set_Pulse(uint32_t speed)
//{
//	//on disable l'interuption pour éviter la modification des variables global dans l'interrupt du timer
//	__disable_irq();  // Disable interrupts
//	pwm_handle.pulse1 = MAX_PULSE * speed / 100;
//	pwm_handle.pulse2 = MAX_PULSE - pwm_handle.pulse1;
//	pwm_handle.previous_pulse1 = htim1.Instance->CCR1;
//	pwm_handle.previous_pulse2 = htim1.Instance->CCR2;
//	__enable_irq();   // Enable interrupts
//
//	pwm_handle.interrupt_counter = 0;
//	return 0;
//}
//
//int PWM_Speed_Control(h_shell_t *h_shell, char **argv, int argc)
//{
//	uint32_t new_speed = atoi(argv[1]);   //speed in expected in % of max speed
//
//	if (argc != 2)
//	{
//		uint8_t error_message[] =
//				"Error : speed function expect exactly 1 parameter \r\n";
//		HAL_UART_Transmit(&huart2, error_message, sizeof(error_message),
//				HAL_MAX_DELAY);
//
//		return 1;
//	}
//
//	else if (new_speed > 90 || new_speed < 10) //on vérifie qu'on met pas la vitesse ne soit pas au dessus de 95% de la max par sécurité
//	{
//		uint8_t error_message[] =
//				"speed function must not exceed 90% of max value  \r\n";
//		HAL_UART_Transmit(&huart2, error_message, sizeof(error_message),
//				HAL_MAX_DELAY);
//		return 1;
//
//	}
//	else if ((speed < 50 && new_speed > 50) || (speed > 50 && new_speed < 50))
//	{
//		uint8_t error_message[] =
//				"speed function must not change the direction of rotation, please go to neutral by entering 50 before\r\n";
//		HAL_UART_Transmit(&huart2, error_message, sizeof(error_message),
//				HAL_MAX_DELAY);
//		return 1;
//	}
//	speed = new_speed;
//	PWM_Set_Pulse(speed);
//	int uartTxStringLength = snprintf((char*) h_shell->uartTxBuffer,
//			UART_TX_BUFFER_SIZE, "speed set to %lu of max value \r\n",
//			(unsigned long) speed);
//	HAL_UART_Transmit(&huart2, h_shell->uartTxBuffer, uartTxStringLength,
//			HAL_MAX_DELAY);
//	return 0;
//
//}

int PWM_Start(h_shell_t *h_shell, char **argv, int argc)
{
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);


	speed = 50; //speed à 50 % du max
	pwm_handle.interrupt_counter = 0;

	pwm_handle.previous_pulse1 = htim1.Instance->CCR1;
	pwm_handle.previous_pulse2 = htim1.Instance->CCR2;
	pwm_handle.previous_pulse3 = htim1.Instance->CCR3;
	pwm_handle.previous_pulse4 = htim1.Instance->CCR4;
	pwm_handle.pulse1 = htim1.Instance->CCR1;
	pwm_handle.pulse2 = htim1.Instance->CCR2;
	pwm_handle.pulse3 = htim1.Instance->CCR3;
	pwm_handle.pulse4 = htim1.Instance->CCR4;


	HAL_TIM_Base_Start_IT(&htim7);
	return 0;

}

int PWM_Stop(h_shell_t *h_shell, char **argv, int argc)
{
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);
	HAL_TIM_Base_Stop(&htim7);
	return 0;

}
