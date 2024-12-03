/*
 * dc_motor_driver.h
 *
 *  Created on: Nov 30, 2024
 *      Author: exysta
 */

#ifndef DC_MOTOR_DRIVER_H_
#define DC_MOTOR_DRIVER_H_

// 224.4 pulses per revolution of the motor
#define ENCODER_PPR  224.4f

/**
 * @brief Structure for one motor
 */
typedef struct DCMOTOR_motor_handle_t{
	uint8_t direction;
	float measured_rpm;
	uint8_t measured_direction;
	int8_t speed; // speed in % from -100% to +100%
	TIM_HandleTypeDef  *encoder_tim;
	volatile uint32_t  encoder_counter;	//store the encoder count


} DCMOTOR_motor_handle_t;

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
