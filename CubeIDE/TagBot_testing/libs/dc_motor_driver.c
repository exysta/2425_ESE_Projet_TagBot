/*
 * dc_motor_driver.c
 *
 *  Created on: Nov 30, 2024
 *      Author: exysta
 */

#include "tim.h"
#include "dc_motor_driver.h"
#include "usart.h"



// PWM_HandleTypeDef pwm_handle;

uint32_t speed; // between 0-100 for motor speed. 0 is max speed in reverse, 50 is neutral and 100 is max speed in the other direction.

int control_motor;
Motors_handle_t motors_handle;


int PWM_set_pulse(uint32_t speed, Motors_typedef motor)
{
	if(motor.name == MOTOR_LEFT)
	{
		//on disable l'interuption pour éviter la modification des variables global dans l'interrupt du timer
		__disable_irq();  // Disable interrupts
		motor.pulse1 = MAX_PULSE * speed / 100;
		motor.pulse2 = MAX_PULSE - motor.pulse1;
		motor.previous_pulse1 = htim1.Instance->CCR4; // PWM_REV
		motor.previous_pulse2 = htim1.Instance->CCR3; // PWM_FWD
		__enable_irq();   // Enable interrupts

		motor.interrupt_counter = 0;
	}
	else if(motor.name == MOTOR_RIGHT)
	{
		//on disable l'interuption pour éviter la modification des variables global dans l'interrupt du timer
		__disable_irq();  // Disable interrupts
		motor.pulse1 = MAX_PULSE * speed / 100;
		motor.pulse2 = MAX_PULSE - motor.pulse1;
		motor.previous_pulse1 = htim1.Instance->CCR2; // PWM_REV
		motor.previous_pulse2 = htim1.Instance->CCR1; // PWM_FWD
		__enable_irq();   // Enable interrupts

		motor.interrupt_counter = 0;
	}
	else
	{
		//ne rien faire, erreur car on ne devrait jamais arrivé ici
		printf("Error : unknown motor \r\n");
		Error_Handler();
	}

	return 0;
}


int PWM_speed_control(h_shell_t *h_shell, char **argv, int argc)
{
	uint32_t new_speed = atoi(argv[1]);   //speed in expected in % of max speed
	Motors_typedef motor;

	if (argc != 2)
	{
		uint8_t error_message[] =
				"Error : speed function expect exactly 1 parameter \r\n";
		HAL_UART_Transmit(&huart2, error_message, sizeof(error_message),
				HAL_MAX_DELAY);

		return 1;
	}

	else if (new_speed > 90 || new_speed < 10) //on vérifie qu'on met pas la vitesse ne soit pas au dessus de 95% de la max par sécurité
	{
		uint8_t error_message[] =
				"speed function must not exceed 90% of max value  \r\n";
		HAL_UART_Transmit(&huart2, error_message, sizeof(error_message),
				HAL_MAX_DELAY);
		return 1;

	}
	else if ((speed < 50 && new_speed > 50) || (speed > 50 && new_speed < 50))
	{
		uint8_t error_message[] =
				"speed function must not change the direction of rotation, please go to neutral by entering 50 before\r\n";
		HAL_UART_Transmit(&huart2, error_message, sizeof(error_message),
				HAL_MAX_DELAY);
		return 1;
	}
	speed = new_speed;
	PWM_set_pulse(speed, motor);
	int uartTxStringLength = snprintf((char*) h_shell->print_buffer,
			SHELL_BUFFER_SIZE, "speed set to %lu of max value \r\n",
			(unsigned long) speed);
	HAL_UART_Transmit(&huart2, h_shell->print_buffer, uartTxStringLength,
			HAL_MAX_DELAY);
	return 0;

}

int PWM_start(h_shell_t *h_shell, char **argv, int argc)
{

	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);

	speed = 50; //speed à 50 % du max

	// moteur droit
	motors_handle.motor_right.interrupt_counter = 0;

	motors_handle.motor_right.previous_pulse1 = htim1.Instance->CCR2;
	motors_handle.motor_right.previous_pulse2 = htim1.Instance->CCR1;

	motors_handle.motor_right.pulse1 = htim1.Instance->CCR2;
	motors_handle.motor_right.pulse2 = htim1.Instance->CCR1;

	// moteur gauche
	motors_handle.motor_left.interrupt_counter = 0;

	motors_handle.motor_left.previous_pulse1 = htim1.Instance->CCR4;
	motors_handle.motor_left.previous_pulse2 = htim1.Instance->CCR3;

	motors_handle.motor_left.pulse1 = htim1.Instance->CCR4;
	motors_handle.motor_left.pulse2 = htim1.Instance->CCR3;

	HAL_TIM_Base_Start_IT(&htim7);
	return 0;

}
int motor_init(Motors_handle_t *motors_driver)
{

	motors_driver->motor_left.name = MOTOR_LEFT;
	motors_driver->motor_right.name = MOTOR_RIGHT;

}
int PWM_stop(h_shell_t *h_shell, char **argv, int argc)
{
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);
	HAL_TIM_Base_Stop(&htim7);
	return 0;

}
