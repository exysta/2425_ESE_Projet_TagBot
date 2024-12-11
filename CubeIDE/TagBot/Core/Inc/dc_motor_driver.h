/*
 * dc_motor_driver.h
 *
 *  Created on: Nov 30, 2024
 *      Author: exysta
 */

#ifndef DC_MOTOR_DRIVER_H_
#define DC_MOTOR_DRIVER_H_

#define MOTOR_LEFT 0
#define MOTOR_RIGHT 1
#define MAX_PULSE 6799

#include "shell.h"


typedef struct Motors_typedef
{
	uint32_t speed;
	int rotation_sign;
	uint8_t name; // define if it is the left or right motor
    uint16_t pulse1;
    uint16_t pulse2;
    uint16_t previous_pulse1;
    uint16_t previous_pulse2;
    uint16_t intermediate_pulse1;
    uint16_t intermediate_pulse2;
    uint16_t interrupt_counter;
}Motors_typedef;

typedef struct Motors_handle_t
{
	Motors_typedef motor_left;
	Motors_typedef motor_right;
}Motors_handle_t;


int PWM_set_pulse(uint32_t speed, Motors_typedef motor);
int PWM_speed_control(h_shell_t *h_shell,char **argv,int argc);
int PWM_start(h_shell_t *h_shell,char **argv,int argc);
int PWM_stop(h_shell_t *h_shell,char **argv,int argc);
int motor_init(Motors_handle_t *motors_driver);

// pwm.h
// Define a struct to hold PWM-related variables
//typedef struct {
//    uint16_t pulse3;
//    uint16_t pulse4;
//    uint16_t previous_pulse3;
//    uint16_t previous_pulse4;
//    uint16_t intermediate_pulse3;
//    uint16_t intermediate_pulse4;
//    uint16_t interrupt_counter;
//} PWM_HandleTypeDef;




///**
// * @brief Structure for one motor
// */
//typedef struct DCMOTOR_encoder_handle_t{
//	DCMOTOR_handle_t motor_left;
//	DCMOTOR_handle_t motor_right;
//	TIM_HandleTypeDef  *encoder_tim;
//
//} DCMOTOR_encoder_handle_t;

#endif /* DC_MOTOR_DRIVER_H_ */
