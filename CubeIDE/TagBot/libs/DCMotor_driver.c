/*
 DCMotor_driver.c
 *
 *  Created on: Dec 13, 2024
 *      Author: exysta
 */

#include "FreeRTOS.h"
#include "task.h"
#include "tim.h"
#include "semphr.h"

#include "DCMotor_driver.h"

//#define PRINT_DEBUG
/** @brief Stack for the shell task. */
static StackType_t DCMotor_task_stack[TASK_MOTOR_STACK_DEPTH];

/** @brief TCB (Task Control Block) for the shell task. */
static StaticTask_t DCMotor_task_tcb;

//single motor init
void DCMotor_MotorInit(Motor_t *motor, TIM_HandleTypeDef motor_tim,
		uint32_t FWD_Channel, uint32_t REV_Channel,uint16_t max_speed)
{
	motor->motor_timer = motor_tim;
	motor->FWD_Channel = FWD_Channel;
	motor->REV_Channel = REV_Channel;
	motor->FWD_current_pulse = 0;
	motor->FWD_target_pulse = 0;
	motor->REV_current_pulse = 0;
	motor->REV_target_pulse = 0;
	motor->max_speed = max_speed;

}

void DCMotor_EncoderInit(Motor_t *motor, TIM_HandleTypeDef motor_tim)
{
	motor->encoder.previous_count = 0;
	motor->encoder.current_count = 0;
	motor->encoder.measured_rpm = 0;
	motor->encoder.measured_rotation_sign = POSITIVE_ROTATION;
	motor->encoder.encoder_timer = motor_tim;
	__HAL_TIM_SET_COUNTER(&motor_tim, 0);
	HAL_TIM_Encoder_Start(&motor_tim, TIM_CHANNEL_ALL);

}


void DCMotor_StartPWM(Motor_t *motor)
{
	// Set the PWM values using the motor_timer
	__HAL_TIM_SET_COMPARE(&motor->motor_timer, motor->FWD_Channel,
			motor->FWD_current_pulse);
	__HAL_TIM_SET_COMPARE(&motor->motor_timer, motor->REV_Channel,
			motor->REV_current_pulse);

	HAL_TIM_PWM_Start(&motor->motor_timer, motor->FWD_Channel);
	HAL_TIM_PWM_Start(&motor->motor_timer, motor->REV_Channel);
}

void DCMotor_Init(DualDrive_handle_t *DualDrive_handle, TIM_HandleTypeDef *tim_enocder_synch)
{
	DCMotor_MotorInit(&DualDrive_handle->motor_right, htim1, TIM_CHANNEL_2,
	TIM_CHANNEL_1,MAX_SPEED_RIGHT);
	DCMotor_MotorInit(&DualDrive_handle->motor_left, htim1, TIM_CHANNEL_4,
	TIM_CHANNEL_3,MAX_SPEED_LEFT);
	DCMotor_EncoderInit(&DualDrive_handle->motor_right, htim3);
	DCMotor_EncoderInit(&DualDrive_handle->motor_left, htim4);

	DualDrive_handle->current_motor = NULL;
	DCMotor_StartPWM(&DualDrive_handle->motor_right);
	DCMotor_StartPWM(&DualDrive_handle->motor_left);

	DualDrive_handle->tim_enocder_synch = tim_enocder_synch;
	HAL_TIM_Base_Start_IT(tim_enocder_synch);

}

//speed should be bewteen 0-100 for percentage of max speed
//rotation sign is either POSITIVE_ROTATION or NEGATIVE_ROTATION
HAL_StatusTypeDef DCMotor_SetSpeed(Motor_t *motor, uint8_t speed,
		uint8_t rotation_sign)
{
	if (((rotation_sign != POSITIVE_ROTATION)
			&& (rotation_sign != NEGATIVE_ROTATION)))
	{
		return HAL_ERROR;
	}
	uint16_t pulse;
	uint16_t normalized_speed = speed;
	//uint16_t normalized_speed = speed * MAX_SPEED_LEFT/motor->max_speed;

	if (normalized_speed <= MAX_SPEED)
	{
		pulse = MAX_PULSE * normalized_speed / 100;
	}
	else
	{
		pulse = MAX_PULSE * MAX_SPEED / 100;

	}

	motor->set_rotation_sign = rotation_sign;

	if (motor->set_rotation_sign == POSITIVE_ROTATION)
	{
		motor->FWD_target_pulse = pulse;
		motor->REV_target_pulse = 0;
	}
	else if (motor->set_rotation_sign == NEGATIVE_ROTATION)
	{
		motor->FWD_target_pulse = 0;
		motor->REV_target_pulse = pulse;
	}

	motor->set_rpm = normalized_speed * motor->max_speed /100;
	return HAL_OK;
}

//brake the motor
//emergency brake so no ramp
void DCMotor_Brake(Motor_t *motor)
{
	__HAL_TIM_SET_COMPARE(&motor->motor_timer, motor->FWD_Channel,
			MAX_PULSE);
	__HAL_TIM_SET_COMPARE(&motor->motor_timer, motor->REV_Channel,
			MAX_PULSE);
}

void DCMotor_Forward(DualDrive_handle_t *DualDrive_handle, uint8_t speed)
{
	//right is faster than left
	DCMotor_SetSpeed(&DualDrive_handle->motor_right, speed, POSITIVE_ROTATION);

	DCMotor_SetSpeed(&DualDrive_handle->motor_left, speed * MAGIC_MOTOR_RATIO,
			POSITIVE_ROTATION);

}

void DCMotor_MeasureMotorSpeed(Motor_t *motor)
{
	// Retrieve current encoder counts
	motor->encoder.current_count = __HAL_TIM_GET_COUNTER(
			&motor->encoder.encoder_timer);
	uint16_t current_count = motor->encoder.current_count;

	uint16_t previous_count = motor->encoder.previous_count;
	uint16_t delta_count;

	// Calculate delta count for left motor
	if (current_count == previous_count)
	{
		delta_count = 0;
	}
	else if (current_count > previous_count)
	{

		delta_count = current_count - previous_count;

	}
	else
	{
		if(motor->set_rotation_sign == POSITIVE_ROTATION)
		{
			delta_count = current_count + ENCODER_TIMER_MAX_COUNT - previous_count;

		}
		else
		{
			delta_count = previous_count - current_count;

		}
	}

	// Calculate pulses per second
	uint16_t motor_pulses_per_second = delta_count
			* ENCODER_CALLBACK_FREQUENCY;

	// Calculate speed (RPM)
	motor->encoder.measured_rpm = (motor_pulses_per_second / (4 * MOTOR_PPR)) * 60;

	// ------------------ Update Previous Counts ------------------
	motor->encoder.previous_count = current_count;
}

void DCMotor_MeasureBothSpeed(DualDrive_handle_t *DualDrive_handle)
{
	//measure max_left speed = 1470
	DCMotor_MeasureMotorSpeed(&DualDrive_handle->motor_left);
	//measure max_right speed = 2372
	DCMotor_MeasureMotorSpeed(&DualDrive_handle->motor_right);
}

void DCMotor_Task(void *argument)
{
	DualDrive_handle_t *DualDrive_handle = (DualDrive_handle_t*) argument;
	DCMotor_Init(DualDrive_handle,&htim8);
	//DCMotor_SetSpeed(&DualDrive_handle->motor_right, 65, POSITIVE_ROTATION);

	DCMotor_SetSpeed(&DualDrive_handle ->motor_left, 100, POSITIVE_ROTATION);
	DCMotor_SetSpeed(&DualDrive_handle ->motor_right, 100, POSITIVE_ROTATION);
	for (;;)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		//immonde mais ca fonctionne
		while (
		    (DualDrive_handle->motor_left.FWD_current_pulse != DualDrive_handle->motor_left.FWD_target_pulse ||
		     DualDrive_handle->motor_right.FWD_current_pulse != DualDrive_handle->motor_right.FWD_target_pulse) ||
		    (DualDrive_handle->motor_left.REV_current_pulse != DualDrive_handle->motor_left.REV_target_pulse ||
		     DualDrive_handle->motor_right.REV_current_pulse != DualDrive_handle->motor_right.REV_target_pulse)
		)
		{
		DCMotor_PulseRamp(&DualDrive_handle->motor_left);
		DCMotor_PulseRamp(&DualDrive_handle->motor_right);
#ifdef PRINT_DEBUG
//		printf("left speed : %d\r\n",
//				DualDrive_handle->motor_left.encoder.measured_rpm);
//		printf("right speed : %d\r\n",
//				DualDrive_handle->motor_right.encoder.measured_rpm);
#endif
		vTaskDelay(20);
		}

	}

}

void DCMotor_EncoderCallback(    TIM_HandleTypeDef * tim,
DualDrive_handle_t *DualDrive_handle)
{
	if(tim->Instance == DualDrive_handle->tim_enocder_synch->Instance)
	{
		DCMotor_MeasureBothSpeed(DualDrive_handle);
	}
}

int DCMotor_CreateTask(DualDrive_handle_t *DualDrive_handle)
{
	// Initialize the TCB to zero
	DualDrive_handle->h_task = NULL;

	// Create the shell task statically
	DualDrive_handle->h_task = xTaskCreateStatic(DCMotor_Task,  // Task function
			"DCMotor_Task",              // Task name
			TASK_MOTOR_STACK_DEPTH,      // Stack size
			(void*) DualDrive_handle,    // Parameters to task
			TASK_MOTOR_PRIORITY,         // Task priority
			DCMotor_task_stack,          // Stack buffer
			&DCMotor_task_tcb            // Task control block
			);

	// Check if task creation was successful
	if (DualDrive_handle->h_task == NULL)
	{
		return 1;  // Task creation failed
	}
	return 0;
}

// Timer callback to update motor PWM values gradually
void DCMotor_PulseRamp(Motor_t *motor)
{
	// Ramp the forward pulse
	if (motor->FWD_current_pulse < motor->FWD_target_pulse)
	{
		motor->FWD_current_pulse += PWM_TAMP_STEP; // Increment by PWM_TAMP_STEP (adjust the step size as needed)
		if (motor->FWD_current_pulse > motor->FWD_target_pulse)
		{
			motor->FWD_current_pulse = motor->FWD_target_pulse; // Clamp to target
		}
	}
	else if (motor->FWD_current_pulse > motor->FWD_target_pulse)
	{
		motor->FWD_current_pulse -= PWM_TAMP_STEP;  // Decrement by PWM_TAMP_STEP
		if (motor->FWD_current_pulse < motor->FWD_target_pulse)
		{
			motor->FWD_current_pulse = motor->FWD_target_pulse; // Clamp to target
		}
	}

	// Ramp the reverse pulse
	if (motor->REV_current_pulse < motor->REV_target_pulse)
	{
		motor->REV_current_pulse += PWM_TAMP_STEP;
		if (motor->REV_current_pulse > motor->REV_target_pulse)
		{
			motor->REV_current_pulse = motor->REV_target_pulse;
		}
	}
	else if (motor->REV_current_pulse > motor->REV_target_pulse)
	{
		motor->REV_current_pulse -= PWM_TAMP_STEP;
		if (motor->REV_current_pulse < motor->REV_target_pulse)
		{
			motor->REV_current_pulse = motor->REV_target_pulse;
		}
	}

	// Set the PWM values using the motor_timer
	__HAL_TIM_SET_COMPARE(&motor->motor_timer, motor->FWD_Channel,
			motor->FWD_current_pulse);
	__HAL_TIM_SET_COMPARE(&motor->motor_timer, motor->REV_Channel,
			motor->REV_current_pulse);
}

