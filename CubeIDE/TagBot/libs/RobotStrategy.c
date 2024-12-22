/*
 * RobotStrategy.c
 *
 *  Created on: Dec 18, 2024
 *      Author: exysta
 */


#include "RobotStrategy.h"
#include "DCMotor_driver.h"
#include "X4LIDAR_driver.h"

#include "main.h"
#include "usart.h"
#include "math.h"

__STRATEGY_HandleTypeDef strategies[3];

TaskHandle_t RobotStategy_task_handle;               // Handle for the FreeRTOS task
StaticTask_t RobotStategy_task_tcb;                  // Static Task Control Block
StackType_t RobotStategy_task_stack[STATEGY_STACK_SIZE]; // Static Stack for the task

extern X4LIDAR_handle_t X4LIDAR_handle;
extern DualDrive_handle_t DualDrive_handle;
extern __TARGET_HandleTypeDef Target_Handle;


HAL_StatusTypeDef RobotStrategy_CreateTask()
{
		// Create the task with a static stack
	RobotStategy_task_handle = xTaskCreateStatic(RobotStrategy_Task, // Task function
				"RobotStrategy_Task",                           // Task name
				STATEGY_STACK_SIZE,                    // Stack size
				NULL,                     // Parameters to task
				STRATEGY_TASK_PRIORITY,                       // Task priority
				RobotStategy_task_stack,          // Stack buffer
				&RobotStategy_task_tcb            // TCB buffer
		);

		// Check if task creation was successful
		if (RobotStategy_task_handle == NULL)
		{
			return HAL_ERROR; // Task creation failed
		}

		return HAL_OK; // Task created successfully
}

HAL_StatusTypeDef RobotStrategy__Init()
{
	return HAL_OK;
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
	memcpy(distance_buffer_copy,X4LIDAR_handle->scan_data.distances
			, MAX_ANGLE * sizeof(float));
	uint16_t angle_with_min_distance;
    for (uint16_t angle = 0; angle < MAX_ANGLE; angle++)
    {
        if (distance_buffer_copy[angle] > MIN_OBJECT_DIST && distance_buffer_copy[angle] < min_distance)
        {
            min_distance = distance_buffer_copy[angle];
            angle_with_min_distance = angle;
        }
    }
    // we determine the end angle
    uint16_t compare_angle = angle_with_min_distance == MAX_ANGLE - 1 ? 0 : angle_with_min_distance  + 1 ;
    while(abs((int)distance_buffer_copy[angle_with_min_distance] - (int)distance_buffer_copy[compare_angle]) < OBJECT_DIST_THRESHOLD )
    {
    	compare_angle = angle_with_min_distance == MAX_ANGLE - 1 ? 0 : angle_with_min_distance;
    	compare_angle++;
    }
    target->end_angle = compare_angle;
    // we determine the start angle
    compare_angle = angle_with_min_distance == 0 ? MAX_ANGLE : angle_with_min_distance  - 1 ;
    while(abs((int)distance_buffer_copy[angle_with_min_distance] - (int)distance_buffer_copy[compare_angle]) < OBJECT_DIST_THRESHOLD )
    {
    	compare_angle = angle_with_min_distance == 0 ? MAX_ANGLE : angle_with_min_distance;
    	compare_angle--;
    }
    target->start_angle = compare_angle;

    target->centroid_angle = RobotStrategy_CalculateCentroidAngle(target->start_angle,target->end_angle);
    return HAL_OK;
}

//function to asserv the robot to the point toward the target
//should be called after RobotStrategy_IdentifyClosestObject determined the target to track
static uint16_t RobotStrategy_CalculateAngleError(__TARGET_HandleTypeDef * target)
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
	target->angle_error = angle_error;
    return HAL_OK;
}

//calculate motors speed in differential driving to track target
HAL_StatusTypeDef RobotStrategy_CalculateMotorSpeed(__TARGET_HandleTypeDef * target,DualDrive_handle_t * DualDrive_handle)
{
	RobotStrategy_CalculateAngleError(target);
	uint16_t angle_error = target->angle_error;
	//base speed in %
	uint8_t base_speed = 60;
	// Proportional control for speed difference
	// with KP = 0.138, max(speed_difference = 24.84 = 25 with quantization round up of 0.5
	uint8_t speed_difference = (KP * angle_error) + 0.5;

	// Compute motor speeds
	uint8_t left_speed = base_speed - speed_difference;
	uint8_t right_speed = base_speed + speed_difference;

	DCMotor_SetSpeed(&DualDrive_handle ->motor_left, left_speed, POSITIVE_ROTATION);
	DCMotor_SetSpeed(&DualDrive_handle ->motor_right, right_speed * 0.85, POSITIVE_ROTATION);
	return HAL_OK;
}


void RobotStrategy_Task(void *argument)
{
	RobotStrategy_InitTarget(&Target_Handle);
	for (;;)
	{

		RobotStrategy_IdentifyClosestObject(&Target_Handle, &X4LIDAR_handle);
		RobotStrategy_CalculateMotorSpeed(&Target_Handle,&DualDrive_handle);
		vTaskDelay(pdMS_TO_TICKS(100));

	}
}
