/*
 * DCMotor_driver.h
 *
 *  Created on: Dec 13, 2024
 *      Author: exysta
 */

#ifndef DCMOTOR_DRIVER_H_
#define DCMOTOR_DRIVER_H_

#define MOTOR_LEFT 0
#define MOTOR_RIGHT 1
#define MAX_PULSE 6799

#define POSITIVE_ROTATION 0
#define NEGATIVE_ROTATION 1

#define MOTOR_PPR 224.4f

#define TASK_MOTOR_STACK_DEPTH 64
#define TASK_MOTOR_PRIORITY 3
#define TASK_TIMER_PRIORITY 2 // the freertos timer priority

#include "tim.h"
#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

typedef struct Encoder_typedef
{
	uint32_t measured_rpm;
	int measured_rotation_sign;
    uint16_t previous_pulse1;
    uint16_t previous_pulse2;
    uint16_t intermediate_pulse1;
    uint16_t intermediate_pulse2;
    uint16_t interrupt_counter;
}Encoder_t;


typedef struct Motor_typedef
{
	uint32_t set_rpm;
	int set_rotation_sign;
	TIM_HandleTypeDef timer;
	uint32_t FWD_Channel;
	uint32_t REV_Channel;
    uint16_t FWD_target_pulse;
    uint16_t REV_target_pulse;
    uint16_t FWD_current_pulse;
    uint16_t REV_current_pulse;
    Encoder_t encoder;
    TimerHandle_t xTimer;
}Motor_t;

//contains both motors
typedef struct DualDrive_handle_typed
{
	Motor_t motor_left;
	Motor_t motor_right;
	//a pointer set for the ramp timer
	Motor_t * current_motor;
	//to ramp up and down the motors pwm
	TimerHandle_t dual_drive_timer;
    TaskHandle_t h_task;

}DualDrive_handle_t;

void DCMotor_MotorInit(Motor_t *motor, TIM_HandleTypeDef motor_tim,
		uint32_t FWD_Channel, uint32_t REV_Channel);
void DCMotor_StartPWM(Motor_t *motor);

HAL_StatusTypeDef DCMotor_SetSpeed(Motor_t *motor, uint8_t speed,
		uint8_t rotation_sign);

HAL_StatusTypeDef DCMotor_Brake(Motor_t *motor);
void DCMotor_InitPWMRampTimer();
void DCMotor_Init(DualDrive_handle_t *DualDrive_handle);
void vRampTimerCallback(TimerHandle_t xTimer);
int DCMotor_CreateTask(DualDrive_handle_t *DualDrive_handle);

#endif /* DCMOTOR_DRIVER_H_ */
