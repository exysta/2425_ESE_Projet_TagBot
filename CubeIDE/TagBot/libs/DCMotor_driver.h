/*
 * DCMotor_driver.h
 *
 *  Created on: Dec 13, 2024
 *      Author: exysta
 */

#ifndef DCMOTOR_DRIVER_H_
#define DCMOTOR_DRIVER_H_

#include "tim.h"
#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"

/* Motor Configuration Macros */
#define MOTOR_LEFT 0                   // Identifier for the left motor
#define MOTOR_RIGHT 1                  // Identifier for the right motor
#define MAX_PULSE 6799                 // Maximum pulse width for PWM

#define MAX_SPEED 100                  // Maximum speed as a percentage

#define POSITIVE_ROTATION 0            // Forward rotation direction
#define NEGATIVE_ROTATION 1            // Reverse rotation direction

/* FreeRTOS Task Configuration */
#define TASK_MOTOR_STACK_DEPTH 256     // Stack size for motor tasks
#define TASK_MOTOR_PRIORITY 11         // Priority for motor tasks
#define TASK_TIMER_PRIORITY 2          // Priority for encoder timer

/* Encoder Configuration */
#define ENCODER_TIMER_MAX_COUNT 65535  // Maximum count value for encoder timer
#define ENCODER_CALLBACK_FREQUENCY 2   // Callback frequency in Hz

/* Motor Tuning Parameters */
#define MAGIC_MOTOR_RATIO 0.8f         // Ratio to balance motor speeds
#define MOTOR_PPR 224.4f               // Pulses per revolution for motor encoder

/* PID Tuning Constants */
#define PID_Kp 0.904f                  // Proportional gain
#define PID_Ki 326.0f                  // Integral gain
#define PID_Kd 0.000585f               // Derivative gain

/* Measured Maximum Speeds (RPM) */
#define MAX_SPEED_LEFT 355             // Maximum RPM for left motor
#define MAX_SPEED_RIGHT 447            // Maximum RPM for right motor

/* Encoder Structure */
typedef struct Encoder_typedef
{
    uint32_t measured_rpm;             // Measured revolutions per minute
    int measured_rotation_sign;        // Direction of rotation (-1 or 1)
    uint16_t previous_count;           // Previous encoder count
    uint16_t current_count;            // Current encoder count
    TIM_HandleTypeDef encoder_timer;   // Timer handle for encoder
} Encoder_t;

/* Motor Structure */
typedef struct Motor_typedef
{
    uint32_t set_rpm;                  // Target RPM for the motor
    int set_rotation_sign;             // Target rotation direction
    TIM_HandleTypeDef motor_timer;     // Timer handle for motor PWM
    uint32_t FWD_Channel;              // PWM forward channel
    uint32_t REV_Channel;              // PWM reverse channel
    uint16_t FWD_target_pulse;         // Target pulse for forward PWM
    uint16_t REV_target_pulse;         // Target pulse for reverse PWM
    uint16_t FWD_current_pulse;        // Current pulse for forward PWM
    uint16_t REV_current_pulse;        // Current pulse for reverse PWM
    Encoder_t encoder;                 // Associated encoder for motor
    uint16_t max_speed;                // Maximum RPM for motor
} Motor_t;

/* Dual Drive Structure */
typedef struct DualDrive_handle_typed
{
    Motor_t motor_left;                // Left motor configuration
    Motor_t motor_right;               // Right motor configuration
    Motor_t *current_motor;            // Pointer to the motor currently ramping
    TaskHandle_t h_task;               // Handle for FreeRTOS task
    TIM_HandleTypeDef *tim_encoder_sync;  // Timer for encoder synchronization
} DualDrive_handle_t;

/* Global Handle */
extern DualDrive_handle_t DualDrive_handle;

/* Function Prototypes */

/**
 * @brief Initialize a single motor.
 *
 * @param motor Pointer to Motor_t structure.
 * @param motor_tim Timer handle for PWM.
 * @param FWD_Channel Forward PWM channel.
 * @param REV_Channel Reverse PWM channel.
 * @param max_speed Maximum RPM for motor.
 */
void DCMotor_MotorInit(Motor_t *motor, TIM_HandleTypeDef motor_tim,
                       uint32_t FWD_Channel, uint32_t REV_Channel, uint16_t max_speed);

/**
 * @brief Start PWM for the motor.
 *
 * @param motor Pointer to Motor_t structure.
 */
void DCMotor_StartPWM(Motor_t *motor);

/**
 * @brief Set speed and direction for a motor.
 *
 * @param motor Pointer to Motor_t structure.
 * @param speed Speed percentage (0-100).
 * @param rotation_sign Direction of rotation (0 for forward, 1 for reverse).
 * @return HAL_StatusTypeDef Status of the operation.
 */
HAL_StatusTypeDef DCMotor_SetSpeed(Motor_t *motor, uint8_t speed, uint8_t rotation_sign);

/**
 * @brief Move both motors forward at a specified speed.
 *
 * @param DualDrive_handle Pointer to DualDrive_handle_t structure.
 * @param speed Speed percentage (0-100).
 */
void DCMotor_Forward(DualDrive_handle_t *DualDrive_handle, uint8_t speed);

/**
 * @brief Apply a brake to the motor.
 *
 * @param motor Pointer to Motor_t structure.
 */
void DCMotor_Brake(Motor_t *motor);

/**
 * @brief Initialize the dual drive system.
 *
 * @param DualDrive_handle Pointer to DualDrive_handle_t structure.
 * @param tim_encoder_sync Timer handle for encoder synchronization.
 */
void DCMotor_Init(DualDrive_handle_t *DualDrive_handle, TIM_HandleTypeDef *tim_encoder_sync);

/**
 * @brief Create a task for motor control.
 *
 * @param DualDrive_handle Pointer to DualDrive_handle_t structure.
 * @return int Status of task creation.
 */
int DCMotor_CreateTask(DualDrive_handle_t *DualDrive_handle);

/**
 * @brief Ramp the PWM pulse for a motor.
 *
 * @param motor Pointer to Motor_t structure.
 */
void DCMotor_PulseRamp(Motor_t *motor);

/**
 * @brief Encoder callback to calculate RPM and direction.
 *
 * @param tim Timer handle triggering the callback.
 * @param DualDrive_handle Pointer to DualDrive_handle_t structure.
 */
void DCMotor_EncoderCallback(TIM_HandleTypeDef *tim, DualDrive_handle_t *DualDrive_handle);

#endif /* DCMOTOR_DRIVER_H_ */
