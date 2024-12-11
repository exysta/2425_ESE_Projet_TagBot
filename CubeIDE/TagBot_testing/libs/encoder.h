/*
 * encoder.h
 *
 *  Created on: Dec 11, 2024
 *      Author: charlotte
 */

#ifndef ENCODER_H_
#define ENCODER_H_



#endif /* ENCODER_H_ */


// 224.4 pulses per revolution of the motor
#define ENCODER_PPR  224.4f

#define MAX_PULSE 6799
#define NEUTRAL_PULSE 3399 // pulse auquel le moteur ne tourne pas
#define RAMP_TIME 6000 //temps en milisecondes pour changer de pulse

/**
 * @brief Structure for one motor
 */
typedef struct DCMOTOR_encoder_handle_t{
	uint8_t direction;
	float measured_rpm;
	uint8_t measured_direction;
	TIM_HandleTypeDef  *encoder_tim;
	volatile uint32_t  encoder_counter;	//store the encoder count


} DCMOTOR_motor_handle_t;
