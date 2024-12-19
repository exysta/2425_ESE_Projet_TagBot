/*
 * RobotStrategy.c
 *
 *  Created on: Dec 18, 2024
 *      Author: exysta
 */


#include "RobotStrategy.h"
#include "main.h"
#include "usart.h"
#include "math.h"

__STRATEGY_HandleTypeDef strategies[3];

HAL_StatusTypeDef RobotStrategy_CreateTask(__STRATEGY_HandleTypeDef strategy)
{
	//	// Check for null handle
	//	if (X4LIDAR_handle == NULL)
	//	{
	//		return HAL_ERROR;
	//	}
	//
	//	// Create the task with a static stack
	//	X4LIDAR_handle->task_handle = xTaskCreateStatic(X4LIDAR_task, // Task function
	//			"X4LIDAR_task",                           // Task name
	//			LIDAR_STACK_SIZE,                    // Stack size
	//			(void*) X4LIDAR_handle,                     // Parameters to task
	//			LIDAR_TASK_PRIORITY,                       // Task priority
	//			X4LIDAR_handle->task_stack,          // Stack buffer
	//			&X4LIDAR_handle->task_tcb            // TCB buffer
	//	);
	//
	//	// Check if task creation was successful
	//	if (X4LIDAR_handle->task_handle == NULL)
	//	{
	//		return HAL_ERROR; // Task creation failed
	//	}
	//
	//	return HAL_OK; // Task created successfully
}

void RobotStrategy_InitTarget(__TARGET_HandleTypeDef * target)
{
	target->min_distance = 0;
	target->avg_distance = 0;
	target->max_distance = 0;

	target->start_angle = 0;
	target->centroid_angle = 0;
	target->end_angle = 0;
}

//reserved to be called inside RobotStrategy_IdentifyClosestObject
static uint16_t RobotStrategy_CalculateCentroidAngle(uint16_t start_angle, uint16_t end_angle)
{
    //we determine the centroid angle
    //we have to handle the case where the target is around the 0 angle since the 0 angle is facing forward it will be our goal for asserv tracking toward target
    // Handle wraparound
    if (end_angle < start_angle)
    {
        end_angle += 360;
    }

    // Calculate midpoint
    uint16_t centroid_angle = (start_angle + end_angle) / 2;

    // Wrap back to [0, 360) if necessary
    if (centroid_angle >= 360) {
        centroid_angle -= 360;
    }

    return centroid_angle;
}

//function to detect closest object
//pinpoint the tracking target
//the lidar motor has a revolution speed of 7Hz, calling this function once every 0.1 sec should be enough for asserv ?
HAL_StatusTypeDef RobotStrategy_IdentifyClosestObject(__TARGET_HandleTypeDef * target,X4LIDAR_handle_t *X4LIDAR_handle)
{
	//we copy the buffer to avoid any modifications to the distance buffer
	float distance_buffer_copy[MAX_ANGLE];
	float min_distance = MAX_OBJECT_DIST;
	memcpy(distance_buffer_copy,X4LIDAR_handle->scan_data->distances
			, MAX_ANGLE * sizeof(float));
	uint16_t angle_with_min_distance;
    for (uint16_t angle = 0; angle < MAX_ANGLE; angle++)
    {
        if (distance_buffer_copy[angle] > MIN_OBJECT_DIST && distance_buffer_copy[angle] < min_distance)
        {
            min_distance = distances[angle];
            angle_with_min_distance = angle;
        }
    }
    // we determine the end angle
    uint16_t compare_angle = angle_with_min_distance == MAX_ANGLE - 1 ? 0 : angle_with_min_distance  + 1 ;
    while(abs((int)distances[angle_with_min_distance] - (int)distances[compare_angle]) < OBJECT_DIST_THRESHOLD )
    {
    	compare_angle = angle_with_min_distance == MAX_ANGLE - 1 ? 0 : angle_with_min_distance;
    	compare_angle++;
    }
    target->end_angle = compare_angle;
    // we determine the start angle
    compare_angle = angle_with_min_distance == 0 ? MAX_ANGLE : angle_with_min_distance  - 1 ;
    while(abs((int)distances[angle_with_min_distance] - (int)distances[compare_angle]) < OBJECT_DIST_THRESHOLD )
    {
    	compare_angle = angle_with_min_distance == 0 ? MAX_ANGLE : angle_with_min_distance1;
    	compare_angle--;
    }
    target->start_angle = compare_angle;

    target->centroid_angle = RobotStrategy_CalculateCentroidAngle(target->start_angle,target->end_angle);
    return HAL_OK;
}

//function to asserv the robot to the point toward the target
//should be called after RobotStrategy_IdentifyClosestObject determined the target to track
static uint16_t RobotStrategy_CalculateAngleError(__TARGET_HandleTypeDef * target,X4LIDAR_handle_t *X4LIDAR_handle)
{
	uint16_t angle_error;
	// target is to the left of the robot if condition is met
	if(target->centroid_angle/2 >= MAX_ANGLE)
	{
		angle_error = MAX_ANGLE - target->centroid_angle;
	}
	// target is to the right of the robot
	else
	{
		angle_error = target->centroid_angle;
	}
    return HAL_OK;
}

//calculate motors speed in differential driving to track target
HAL_StatusTypeDef RobotStrategy_CalculateMotorSpeed(__TARGET_HandleTypeDef * target,X4LIDAR_handle_t *X4LIDAR_handle)
{
	uint16_t angle_error = RobotStrategy_CalculateAngleError(target,X4LIDAR_handle);

	// Proportional control for speed difference
	float speed_difference = KP * angle_error;

	// Compute motor speeds
	*left_motor_speed = base_speed - speed_difference;
	*right_motor_speed = base_speed + speed_difference;

	// Clamp speeds to 0-100%
	if (*left_motor_speed < 0) *left_motor_speed = 0;
	if (*left_motor_speed > MAX_SPEED) *left_motor_speed = MAX_SPEED;
	if (*right_motor_speed < 0) *right_motor_speed = 0;
	if (*right_motor_speed > MAX_SPEED) *right_motor_speed = MAX_SPEED;
}
