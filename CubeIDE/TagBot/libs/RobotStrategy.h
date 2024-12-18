/*
 * RobotStrategy.h
 *
 *  Created on: Dec 18, 2024
 *      Author: exysta
 */

#ifndef ROBOTSTRATEGY_H_
#define ROBOTSTRATEGY_H_

#include "main.h"

typedef enum {
    NO_Strat = 0x00,
	CAT_Strat = 0x01,
	MOUSE_Strat = 0x02,
} STRATEGY_Enum;

typedef struct __ACTION_HandleTypeDef
{
	uint32_t start_Time;
	uint32_t end_Time;
	uint8_t enable_stop_lidar;
	uint32_t speed_Motor_Left;
	uint32_t speed_Motor_Right;
} __ACTION_HandleTypeDef;

typedef struct __STRATEGY_HandleTypeDef
{
	uint8_t id;
	uint8_t name[2];
	uint8_t action_id;
	uint8_t action_nb;
	__ACTION_HandleTypeDef action[10];
} __STRATEGY_HandleTypeDef;

extern __STRATEGY_HandleTypeDef actual_strategy;

void STRATEGY_Init(void);
void STRATEGY_Update(void);
void STRATEGY_Refresh(uint32_t time, uint8_t object_detected, uint8_t is_started);
#endif /* ROBOTSTRATEGY_H_ */
