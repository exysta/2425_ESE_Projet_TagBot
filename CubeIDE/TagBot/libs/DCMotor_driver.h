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

#define MAX_SPEED 100 //max speed in percentage for motors

#define POSITIVE_ROTATION 0
#define NEGATIVE_ROTATION 1


#define TASK_MOTOR_STACK_DEPTH 256
#define TASK_MOTOR_PRIORITY 11
#define TASK_TIMER_PRIORITY 2 // the freertos timer priority, to be changed in ioc or .h

#define ENCODER_TIMER_MAX_COUNT 65535

#define MOTOR_PPR 224.4f

#define ENCODER_CALLBACK_FREQUENCY 2 //freq in HZ for the callback of encoder

#define MAGIC_MOTOR_RATIO 0.8f // the right motor speed = 0.8 * left motor speed

#define PID_Kp 0.904f
#define PID_Ki 326.0f
#define PID_Kd 0.000585f

#define MAX_SPEED_LEFT 355 // measured max rpm
#define MAX_SPEED_RIGHT 447


#include "tim.h"
#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"

typedef struct Encoder_typedef
{
	uint32_t measured_rpm;
	int measured_rotation_sign;
    uint16_t previous_count;
    uint16_t current_count;
	TIM_HandleTypeDef encoder_timer;

}Encoder_t;



typedef struct Motor_typedef
{
	uint32_t set_rpm;
	int set_rotation_sign;
	TIM_HandleTypeDef motor_timer;
	uint32_t FWD_Channel;
	uint32_t REV_Channel;
    uint16_t FWD_target_pulse;
    uint16_t REV_target_pulse;
    uint16_t FWD_current_pulse;
    uint16_t REV_current_pulse;
    Encoder_t encoder;
    uint16_t max_speed;
}Motor_t;

//contains both motors
typedef struct DualDrive_handle_typed
{
	Motor_t motor_left;
	Motor_t motor_right;
	//a pointer set for the ramp timer
	Motor_t * current_motor;
	//to ramp up and down the motors pwm
    TaskHandle_t h_task;
    //callback timer  2 Hz to measure motor speed
    TIM_HandleTypeDef * tim_enocder_synch;

}DualDrive_handle_t;

extern DualDrive_handle_t DualDrive_handle;

void DCMotor_MotorInit(Motor_t *motor, TIM_HandleTypeDef motor_tim,
		uint32_t FWD_Channel, uint32_t REV_Channel,uint16_t max_speed);
void DCMotor_StartPWM(Motor_t *motor);

HAL_StatusTypeDef DCMotor_SetSpeed(Motor_t *motor, uint8_t speed,
		uint8_t rotation_sign);
int DCMotor_Forward(DualDrive_handle_t *DualDrive_handle, uint8_t speed);
void DCMotor_Brake(Motor_t *motor);
void DCMotor_Init(DualDrive_handle_t *DualDrive_handle,TIM_HandleTypeDef *tim_enocder_synch);
int DCMotor_CreateTask(DualDrive_handle_t *DualDrive_handle);
void DCMotor_PulseRamp(Motor_t * motor);
void DCMotor_EncoderCallback(TIM_HandleTypeDef * tim, DualDrive_handle_t *DualDrive_handle);
#endif /* DCMOTOR_DRIVER_H_ */
