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


int PWM_Set_Pulse(uint32_t speed);
int PWM_Speed_Control(h_shell_t *h_shell,char **argv,int argc);
int PWM_Start(h_shell_t *h_shell,char **argv,int argc);
int PWM_Stop(h_shell_t *h_shell,char **argv,int argc);

// pwm.h
// Define a struct to hold PWM-related variables
typedef struct {
    uint16_t pulse1;
    uint16_t pulse2;
    uint16_t pulse3;
    uint16_t pulse4;
    uint16_t previous_pulse1;
    uint16_t previous_pulse2;
    uint16_t previous_pulse3;
    uint16_t previous_pulse4;
    uint16_t intermediate_pulse1;
    uint16_t intermediate_pulse2;
    uint16_t intermediate_pulse3;
    uint16_t intermediate_pulse4;
    uint16_t interrupt_counter;
} PWM_HandleTypeDef;

typedef struct Motors_handle_t
{
	Motors_typedef motor_left;
	Motors_typedef motor_right;
}Motors_handle_t;

typedef struct Motors_typedef
{
	uint32_t speed;
	int rotation_sign;
	uint32_t PwM_FWD;
	uint32_t PwM_REV;
}Motors_typedef;




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
