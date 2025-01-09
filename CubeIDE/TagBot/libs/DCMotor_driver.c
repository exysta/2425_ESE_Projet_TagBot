/**
 * @file DCMotor_driver.c
 * @brief Implementation of DC motor control functions for a dual motor system.
 * @date Dec 13, 2024
 * @author exysta
 */

#include "FreeRTOS.h"
#include "task.h"
#include "tim.h"
#include "semphr.h"
#include "DCMotor_driver.h"

//#define PRINT_DEBUG

/**
 * @brief Stack for the motor control task.
 */
static StackType_t DCMotor_task_stack[TASK_MOTOR_STACK_DEPTH];

/**
 * @brief Task Control Block (TCB) for the motor control task.
 */
static StaticTask_t DCMotor_task_tcb;

/**
 * @brief Initialize a single motor structure with timer and PWM configuration.
 *
 * @param motor Pointer to the motor structure to initialize.
 * @param motor_tim Timer handle for the motor PWM control.
 * @param FWD_Channel Forward PWM channel.
 * @param REV_Channel Reverse PWM channel.
 * @param max_speed Maximum speed (RPM) for the motor.
 */
void DCMotor_MotorInit(Motor_t *motor, TIM_HandleTypeDef motor_tim,
                       uint32_t FWD_Channel, uint32_t REV_Channel, uint16_t max_speed)
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

/**
 * @brief Initialize the encoder for a motor to measure speed and direction.
 *
 * @param motor Pointer to the motor structure containing the encoder.
 * @param motor_tim Timer handle for the encoder.
 */
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

/**
 * @brief Start the PWM signals for a motor.
 *
 * @param motor Pointer to the motor structure.
 */
void DCMotor_StartPWM(Motor_t *motor)
{
    __HAL_TIM_SET_COMPARE(&motor->motor_timer, motor->FWD_Channel, motor->FWD_current_pulse);
    __HAL_TIM_SET_COMPARE(&motor->motor_timer, motor->REV_Channel, motor->REV_current_pulse);

    HAL_TIM_PWM_Start(&motor->motor_timer, motor->FWD_Channel);
    HAL_TIM_PWM_Start(&motor->motor_timer, motor->REV_Channel);
}

/**
 * @brief Initialize the dual motor system including encoders and timers.
 *
 * @param DualDrive_handle Pointer to the dual motor system handle.
 * @param tim_enocder_synch Timer handle for encoder synchronization.
 */
void DCMotor_Init(DualDrive_handle_t *DualDrive_handle, TIM_HandleTypeDef *tim_enocder_synch)
{
    DCMotor_MotorInit(&DualDrive_handle->motor_right, htim1, TIM_CHANNEL_2, TIM_CHANNEL_1, MAX_SPEED_RIGHT);
    DCMotor_MotorInit(&DualDrive_handle->motor_left, htim1, TIM_CHANNEL_4, TIM_CHANNEL_3, MAX_SPEED_LEFT);
    DCMotor_EncoderInit(&DualDrive_handle->motor_right, htim3);
    DCMotor_EncoderInit(&DualDrive_handle->motor_left, htim4);

    DualDrive_handle->current_motor = NULL;
    DCMotor_StartPWM(&DualDrive_handle->motor_right);
    DCMotor_StartPWM(&DualDrive_handle->motor_left);

    DualDrive_handle->tim_enocder_synch = tim_enocder_synch;
    HAL_TIM_Base_Start_IT(tim_enocder_synch);
}

/**
 * @brief Set the speed and direction of a motor.
 *
 * @param motor Pointer to the motor structure.
 * @param speed Desired speed as a percentage (0-100) of maximum speed.
 * @param rotation_sign Direction of rotation (POSITIVE_ROTATION or NEGATIVE_ROTATION).
 * @return HAL_OK if successful, HAL_ERROR otherwise.
 */
HAL_StatusTypeDef DCMotor_SetSpeed(Motor_t *motor, uint8_t speed, uint8_t rotation_sign)
{
    if ((rotation_sign != POSITIVE_ROTATION) && (rotation_sign != NEGATIVE_ROTATION))
    {
        return HAL_ERROR;
    }

    uint16_t pulse;
    uint16_t normalized_speed = speed;

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

    motor->set_rpm = normalized_speed * motor->max_speed / 100;
    return HAL_OK;
}

/**
 * @brief Apply an emergency brake to a motor by setting maximum PWM pulses.
 *
 * @param motor Pointer to the motor structure.
 */
void DCMotor_Brake(Motor_t *motor)
{
    __HAL_TIM_SET_COMPARE(&motor->motor_timer, motor->FWD_Channel, MAX_PULSE);
    __HAL_TIM_SET_COMPARE(&motor->motor_timer, motor->REV_Channel, MAX_PULSE);
}

/**
 * @brief Move both motors forward at a given speed.
 *
 * @param DualDrive_handle Pointer to the dual motor system handle.
 * @param speed Speed percentage (0-100) for the motors.
 */
void DCMotor_Forward(DualDrive_handle_t *DualDrive_handle, uint8_t speed)
{
    // Right motor runs slightly faster to compensate for mechanical imbalance.
    DCMotor_SetSpeed(&DualDrive_handle->motor_right, speed, POSITIVE_ROTATION);
    DCMotor_SetSpeed(&DualDrive_handle->motor_left, speed * MAGIC_MOTOR_RATIO, POSITIVE_ROTATION);
}

/**
 * @brief Measure the speed of a motor using its encoder.
 *
 * @param motor Pointer to the motor structure containing encoder data.
 */
void DCMotor_MeasureMotorSpeed(Motor_t *motor)
{
    motor->encoder.current_count = __HAL_TIM_GET_COUNTER(&motor->encoder.encoder_timer);
    uint16_t current_count = motor->encoder.current_count;
    uint16_t previous_count = motor->encoder.previous_count;
    uint16_t delta_count;

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
        if (motor->set_rotation_sign == POSITIVE_ROTATION)
        {
            delta_count = current_count + ENCODER_TIMER_MAX_COUNT - previous_count;
        }
        else
        {
            delta_count = previous_count - current_count;
        }
    }

    uint16_t motor_pulses_per_second = delta_count * ENCODER_CALLBACK_FREQUENCY;
    motor->encoder.measured_rpm = (motor_pulses_per_second / (4 * MOTOR_PPR)) * 60;
    motor->encoder.previous_count = current_count;
}

/**
 * @brief Measure the speed of both motors in the dual drive system.
 *
 * @param DualDrive_handle Pointer to the dual motor system handle.
 */
void DCMotor_MeasureBothSpeed(DualDrive_handle_t *DualDrive_handle)
{
    DCMotor_MeasureMotorSpeed(&DualDrive_handle->motor_left);
    DCMotor_MeasureMotorSpeed(&DualDrive_handle->motor_right);
}

/**
 * @brief Task to control the DC motors, including initialization and speed ramping.
 *
 * @param argument Pointer to the dual drive system handle passed as an argument.
 */
void DCMotor_Task(void *argument)
{
    DualDrive_handle_t *DualDrive_handle = (DualDrive_handle_t*) argument;
    DCMotor_Init(DualDrive_handle, &htim8);

    DCMotor_SetSpeed(&DualDrive_handle->motor_left, 100, POSITIVE_ROTATION);
    DCMotor_SetSpeed(&DualDrive_handle->motor_right, 100, POSITIVE_ROTATION);

    for (;;)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

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
            // Uncomment for debug messages
            // printf("Left speed: %d RPM\r\n", DualDrive_handle->motor_left.encoder.measured_rpm);
            // printf("Right speed: %d RPM\r\n", DualDrive_handle->motor_right.encoder.measured_rpm);
            #endif
            vTaskDelay(20);
        }
    }
}

/**
 * @brief Callback function to measure both motor speeds during a timer interrupt.
 *
 * @param tim Pointer to the timer handle.
 * @param DualDrive_handle Pointer to the dual motor system handle.
 */
void DCMotor_EncoderCallback(TIM_HandleTypeDef *tim, DualDrive_handle_t *DualDrive_handle)
{
    if (tim->Instance == DualDrive_handle->tim_enocder_synch->Instance)
    {
        DCMotor_MeasureBothSpeed(DualDrive_handle);
    }
}

/**
 * @brief Create the motor control task.
 *
 * @param DualDrive_handle Pointer to the dual motor system handle.
 * @return 0 if successful, 1 if task creation failed.
 */
int DCMotor_CreateTask(DualDrive_handle_t *DualDrive_handle)
{
    DualDrive_handle->h_task = NULL;
    DualDrive_handle->h_task = xTaskCreateStatic(
        DCMotor_Task,              // Task function
        "DCMotor_Task",            // Task name
        TASK_MOTOR_STACK_DEPTH,    // Stack size
        (void*) DualDrive_handle,  // Task parameters
        TASK_MOTOR_PRIORITY,       // Task priority
        DCMotor_task_stack,        // Stack buffer
        &DCMotor_task_tcb          // Task control block
    );

    if (DualDrive_handle->h_task == NULL)
    {
        return 1;
    }
    return 0;
}

/**
 * @brief Gradually update motor PWM values to reach the target speed.
 *
 * @param motor Pointer to the motor structure.
 */
void DCMotor_PulseRamp(Motor_t *motor)
{
    if (motor->FWD_current_pulse < motor->FWD_target_pulse)
    {
        motor->FWD_current_pulse += PWM_TAMP_STEP;
        if (motor->FWD_current_pulse > motor->FWD_target_pulse)
        {
            motor->FWD_current_pulse = motor->FWD_target_pulse;
        }
    }
    else if (motor->FWD_current_pulse > motor->FWD_target_pulse)
    {
        motor->FWD_current_pulse -= PWM_TAMP_STEP;
        if (motor->FWD_current_pulse < motor->FWD_target_pulse)
        {
            motor->FWD_current_pulse = motor->FWD_target_pulse;
        }
    }

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

    __HAL_TIM_SET_COMPARE(&motor->motor_timer, motor->FWD_Channel, motor->FWD_current_pulse);
    __HAL_TIM_SET_COMPARE(&motor->motor_timer, motor->REV_Channel, motor->REV_current_pulse);
}


