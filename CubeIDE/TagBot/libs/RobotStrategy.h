/*
 * RobotStrategy.h
 *
 *  Created on: Dec 18, 2024
 *      Author: exysta
 */

#ifndef ROBOTSTRATEGY_H_
#define ROBOTSTRATEGY_H_

#include "main.h"
#include "X4LIDAR_driver.h"

#define MIN_OBJECT_DIST 120 //according to the datasheet the lidar has a min dist of 0.12 m = 120 mm
#define MAX_OBJECT_DIST 10000 //max dist of 10 meters

typedef struct __TARGET_HandleTypeDef{
	uint8_t start_angle;
	uint8_t centroid_angle;
	uint8_t end_angle;
	float min_distance;
	float max_distance;
	float avg_distance;
}__TARGET_HandleTypeDef;


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
	uint8_t action_id;
	uint8_t action_nb;
	__ACTION_HandleTypeDef action[10];
} __STRATEGY_HandleTypeDef;

extern __STRATEGY_HandleTypeDef actual_strategy;

void STRATEGY_Init(void);
void STRATEGY_Update(void);
void STRATEGY_Refresh(uint32_t time, uint8_t object_detected, uint8_t is_started);
#endif /* ROBOTSTRATEGY_H_ */
