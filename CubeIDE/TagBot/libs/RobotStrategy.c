/*
 * RobotStrategy.c
 *
 *  Created on: Dec 18, 2024
 *      Author: exysta
 */


#include "RobotStrategy.h"
#include "main.h"
#include "usart.h"

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


//function to detect closest object
//the lidar motor has a revolution speed of 7Hz, calling this function once every 0.1 sec should be enough for asserv ?
HAL_StatusTypeDef RobotStrategy_IdentifyClosestObject(__TARGET_HandleTypeDef * target,X4LIDAR_handle_t *X4LIDAR_handle)
{
	//we copy the buffer to avoid any modifications to the distance buffer
//	float distance_buffer_copy[MAX_ANGLE];
//	memcpy(distance_buffer_copy, 	distanceX4LIDAR_handle->scan_data->distances
//			, MAX_ANGLE * sizeof(float));
//    for (int angle = 0; angle < MAX_ANGLE; angle++)
//    {
//        if (distance_buffer_copy[angle] > MIN_OBJECT_DIST && distance_buffer_copy[angle] < min_distance)
//        {
//            min_distance = distances[angle];
//            angle_with_min_distance = angle;
//        }
//    }
}
