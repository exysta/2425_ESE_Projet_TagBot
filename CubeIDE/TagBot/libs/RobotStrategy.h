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
#define MAX_OBJECT_DIST 10000 //max dist of 10 meter for object detection with the Lidar
#define OBJECT_DIST_THRESHOLD 100 //the distance difference  threshold which separates object into different segment

// Proportional control constant for speed difference in differential motor driving
// For now we have a max angle error of 180°, if we want our max differential speed to be 25% of max speed we have KP = 25/180 = 0.138
// to be adjusted experimentally
#define KP 0.138f

#define MEDIAN_FILTER_WINDOW_SIZE 3

#define STATEGY_STACK_SIZE 1024
#define STRATEGY_TASK_PRIORITY 10

typedef enum {
    LEFT = 0x00,
	RIGHT = 0x01,
	FORWARD = 0x03,
	BACKWARD = 0x04,
} DIRECTION_Enum;

typedef struct __TARGET_HandleTypeDef{
	uint16_t start_angle;
	uint16_t centroid_angle;
	uint16_t end_angle;
	// difference in ° between robot forward direction and the object centroid angle
	//ideally it should be 0
	uint16_t angle_error;
	uint16_t min_distance;
	uint16_t max_distance;
	uint16_t avg_distance;

	DIRECTION_Enum direction;

	uint16_t old_centroid_angle;

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

//void STRATEGY_Init(void);
//void STRATEGY_Update(void);
//void STRATEGY_Refresh(uint32_t time, uint8_t object_detected, uint8_t is_started);
HAL_StatusTypeDef RobotStrategy_CreateTask();
void RobotStrategy_Task(void *argument);
void RobotStrategy_MedianFilter(float unfiltered_buffer[MAX_ANGLE], int filtered_buffer[MAX_ANGLE]);
int RobotStrategy_CalculateMedian(int *values, int size);

#endif /* ROBOTSTRATEGY_H_ */
