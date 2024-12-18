/*
 DCMotor_driver.c
 *
 *  Created on: Dec 13, 2024
 *      Author: exysta
 */

#include "FreeRTOS.h"
#include "task.h"
#include "tim.h"
#include "timers.h"

#include "DCMotor_driver.h"

/** @brief Stack for the shell task. */
static StackType_t DCMotor_task_stack[TASK_MOTOR_STACK_DEPTH];

/** @brief TCB (Task Control Block) for the shell task. */
static StaticTask_t DCMotor_task_tcb;

extern DualDrive_handle_t DualDrive_handle;

//single motor init
void DCMotor_MotorInit(Motor_t *motor, TIM_HandleTypeDef motor_tim,
		uint32_t FWD_Channel, uint32_t REV_Channel)
{
	motor->timer = motor_tim;
	motor->FWD_Channel = FWD_Channel;
	motor->REV_Channel = REV_Channel;
	motor->FWD_current_pulse = 0;
	motor->FWD_target_pulse = 0;
	motor->REV_current_pulse = 0;
	motor->REV_target_pulse = 0;
}

void DCMotor_StartPWM(Motor_t *motor)
{
	// Set the PWM values using the timer
	__HAL_TIM_SET_COMPARE(&motor->timer, motor->FWD_Channel,
			motor->FWD_current_pulse);
	__HAL_TIM_SET_COMPARE(&motor->timer, motor->REV_Channel,
			motor->REV_current_pulse);

	HAL_TIM_PWM_Start(&motor->timer, motor->FWD_Channel);
	HAL_TIM_PWM_Start(&motor->timer, motor->REV_Channel);
}

//speed should be bewteen 0-100 for percentage of max speed
//rotation sign is either POSITIVE_ROTATION or NEGATIVE_ROTATION
HAL_StatusTypeDef DCMotor_SetSpeed(Motor_t *motor, uint8_t speed,
		uint8_t rotation_sign)
{
	if (((rotation_sign != POSITIVE_ROTATION)
			&& (rotation_sign != NEGATIVE_ROTATION)) || (speed > 100))
	{
		return HAL_ERROR;
	}

	motor->set_rpm = speed;
	motor->set_rotation_sign = rotation_sign;

	uint16_t pulse = MAX_PULSE * speed / 100;

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
	motor->FWD_current_pulse = 0;
	motor->REV_current_pulse = 0;

	DualDrive_handle.current_motor = motor;

//Start the ramp timer
	if (xTimerIsTimerActive(DualDrive_handle.dual_drive_timer) == pdFALSE)
	{
		xTimerStart(DualDrive_handle.dual_drive_timer, 0); // Start the ramp timer
	}

	return HAL_OK;
}

//brake the motor
HAL_StatusTypeDef DCMotor_Brake(Motor_t *motor)
{
	motor->FWD_target_pulse = MAX_PULSE;
	motor->REV_target_pulse = MAX_PULSE;
}

void DCMotor_InitPWMRampTimer()
{
	// Create a timer that will call vRampTimerCallback every 10ms (for smooth ramping)
	DualDrive_handle.dual_drive_timer = xTimerCreate("DCMotor_RampTimer",
			pdMS_TO_TICKS(10), pdTRUE, NULL, vRampTimerCallback);
	if (DualDrive_handle.dual_drive_timer == NULL)
	{
		// Handle error if timer creation fails
	}
}

void DCMotor_Init(DualDrive_handle_t *DualDrive_handle)
{
	DCMotor_MotorInit(&DualDrive_handle->motor_right, htim1, TIM_CHANNEL_1,
	TIM_CHANNEL_2);
	DCMotor_MotorInit(&DualDrive_handle->motor_left, htim1, TIM_CHANNEL_3,
	TIM_CHANNEL_4);
	DualDrive_handle->current_motor = NULL;
	DCMotor_StartPWM(&DualDrive_handle->motor_right);
	DCMotor_StartPWM(&DualDrive_handle->motor_left);
	DCMotor_InitPWMRampTimer();

}

int DCMotor_Forward(DualDrive_handle_t *DualDrive_handle,uint8_t speed)
{
	DCMotor_SetSpeed(&DualDrive_handle->motor_right, 60, POSITIVE_ROTATION);
	vTaskDelay(200);

	DCMotor_SetSpeed(&DualDrive_handle->motor_left, 60, POSITIVE_ROTATION);

}


void DCMotor_Task(void *argument)
{
	DualDrive_handle_t *DualDrive_handle = (DualDrive_handle_t*) argument;
	DCMotor_Init(DualDrive_handle);
	DCMotor_Forward(DualDrive_handle,60);
	for (;;)
	{
		vTaskDelay(10);
	}
}

int DCMotor_CreateTask(DualDrive_handle_t *DualDrive_handle)
{
	// Initialize the TCB to zero
	DualDrive_handle->h_task = NULL;

	// Create the shell task statically
	DualDrive_handle->h_task = xTaskCreateStatic(
	    DCMotor_Task,                // Task function
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
void vRampTimerCallback(TimerHandle_t xTimer)
{
	// Loop through each motor in the array

	Motor_t *motor = DualDrive_handle.current_motor;

	// Ramp the forward pulse
	if (motor->FWD_current_pulse < motor->FWD_target_pulse)
	{
		motor->FWD_current_pulse += 250; // Increment by 250 (adjust the step size as needed)
		if (motor->FWD_current_pulse > motor->FWD_target_pulse)
		{
			motor->FWD_current_pulse = motor->FWD_target_pulse; // Clamp to target
		}
	}
	else if (motor->FWD_current_pulse > motor->FWD_target_pulse)
	{
		motor->FWD_current_pulse -= 250;  // Decrement by 250
		if (motor->FWD_current_pulse < motor->FWD_target_pulse)
		{
			motor->FWD_current_pulse = motor->FWD_target_pulse; // Clamp to target
		}
	}

	// Ramp the reverse pulse
	if (motor->REV_current_pulse < motor->REV_target_pulse)
	{
		motor->REV_current_pulse += 250;
		if (motor->REV_current_pulse > motor->REV_target_pulse)
		{
			motor->REV_current_pulse = motor->REV_target_pulse;
		}
	}
	else if (motor->REV_current_pulse > motor->REV_target_pulse)
	{
		motor->REV_current_pulse -= 250;
		if (motor->REV_current_pulse < motor->REV_target_pulse)
		{
			motor->REV_current_pulse = motor->REV_target_pulse;
		}
	}

	// Set the PWM values using the timer
	__HAL_TIM_SET_COMPARE(&motor->timer, motor->FWD_Channel,
			motor->FWD_current_pulse);
	__HAL_TIM_SET_COMPARE(&motor->timer, motor->REV_Channel,
			motor->REV_current_pulse);

	// If the pulses are equal to the target, stop the timer (optional)
	if (motor->FWD_current_pulse == motor->FWD_target_pulse
			&& motor->REV_current_pulse == motor->REV_target_pulse)
	{
		xTimerStop(xTimer, 0);
	}
}

