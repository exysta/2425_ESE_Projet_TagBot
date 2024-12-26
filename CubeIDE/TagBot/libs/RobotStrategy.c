/*
 * RobotStrategy.c
 *
 *  Created on: Dec 18, 2024
 *      Author: exysta
 */


#include "RobotStrategy.h"
#include "DCMotor_driver.h"
#include "X4LIDAR_driver.h"
#include "semphr.h"

#include "main.h"
#include "usart.h"
#include <stdlib.h>

#define PRINT_DEBUG

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

	target->old_centroid_angle = 0;
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
HAL_StatusTypeDef RobotStrategy_IdentifyClosestObject(__TARGET_HandleTypeDef * target, X4LIDAR_handle_t *X4LIDAR_handle)
{
    // Copy the distance buffer to avoid modifying the original buffer
    int distance_buffer_copy[MAX_ANGLE];
    float min_distance = MAX_OBJECT_DIST;

    RobotStrategy_MedianFilter(X4LIDAR_handle->scan_data.distances, distance_buffer_copy);
    uint16_t angle_with_min_distance = 0; // Initialize to avoid potential undefined behavior

    // Find the angle with the minimum distance
    for (uint16_t angle = 0; angle < MAX_ANGLE; angle++)
    {
        if (distance_buffer_copy[angle] > MIN_OBJECT_DIST && distance_buffer_copy[angle] < min_distance)
        {
            min_distance = distance_buffer_copy[angle];
            angle_with_min_distance = angle;
        }
    }
    target->min_distance = min_distance;
    // Determine the end angle
    uint16_t compare_angle = (angle_with_min_distance) % MAX_ANGLE; // Wraparound using modulo
    while (abs((int)distance_buffer_copy[angle_with_min_distance] - (int)distance_buffer_copy[compare_angle]) < OBJECT_DIST_THRESHOLD)
    {
        compare_angle = (compare_angle + 1) % MAX_ANGLE ; // Increment and wraparound
        if (compare_angle == angle_with_min_distance) // Prevent infinite loop
        {
            break;
        }
    }
    target->end_angle = compare_angle;

    // Determine the start angle
    compare_angle = (angle_with_min_distance == 0) ? (MAX_ANGLE - 1) : (angle_with_min_distance - 1); // Wraparound backward
    while (abs((int)distance_buffer_copy[angle_with_min_distance] - (int)distance_buffer_copy[compare_angle]) < OBJECT_DIST_THRESHOLD)
    {
        compare_angle = (compare_angle == 0) ? (MAX_ANGLE - 1) : (compare_angle - 1); // Decrement and wraparound
        if (compare_angle == angle_with_min_distance) // Prevent infinite loop
        {
            break;
        }
    }
    target->start_angle = compare_angle;
    target->min_distance = compare_angle;

    // Calculate the centroid angle
    target->centroid_angle = RobotStrategy_CalculateCentroidAngle(target->start_angle, target->end_angle);

    return HAL_OK;
}


//function to asserv the robot to the point toward the target
//should be called after RobotStrategy_IdentifyClosestObject determined the target to track
static uint16_t RobotStrategy_CalculateAngleError(__TARGET_HandleTypeDef * target)
{
	uint16_t angle_error;
	// target is to the left of the robot if condition is met
	if(target->centroid_angle >= MAX_ANGLE/2)
	{
		angle_error = MAX_ANGLE - target->centroid_angle;
		target->direction = LEFT;
	}
	// target is to the right of the robot
	else
	{
		angle_error = target->centroid_angle;
		target->direction = RIGHT;

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
	uint8_t base_speed = 70;
	// Proportional control for speed difference
	// with KP = 0.138, max(speed_difference = 24.84 = 25 with quantization round up of 0.5
	//uint8_t speed_difference = (KP * angle_error) + 0.5;
	uint8_t speed_difference = (KP/3 * angle_error) + 0.5;

	uint8_t left_speed;
	uint8_t right_speed;

	//rignt wheel should be faster than left wheel to turn left
	if(target->direction == LEFT)
	{
		left_speed	 = base_speed - speed_difference;
		right_speed	 = base_speed + speed_difference;
	}
	else{
		left_speed	 = base_speed + speed_difference;
		right_speed	 = base_speed - speed_difference;
	}
	// Compute motor speeds

	if(	target->old_centroid_angle != target->centroid_angle)
	{


		DCMotor_SetSpeed(&DualDrive_handle ->motor_left, left_speed, POSITIVE_ROTATION);
		DCMotor_SetSpeed(&DualDrive_handle ->motor_right, right_speed, POSITIVE_ROTATION);
        xTaskNotifyGive(DualDrive_handle->h_task);

	}


#ifdef PRINT_DEBUG
	printf("start angle = %d , dist = %d\r\n\ ",target->start_angle,target->min_distance);
	printf("centroid angle = %d , dist = %d\r\n ",target->centroid_angle,target->avg_distance);
	printf("end angle = %d , dist = %d\r\n ",target->end_angle,target->max_distance);
	printf("left measured speed = %lu,left set speed = %lu \r\n",DualDrive_handle ->motor_left.encoder.measured_rpm,DualDrive_handle->motor_left.set_rpm);
	printf("right measured speed = %lu right set speed = %lu\r\n",DualDrive_handle ->motor_right.encoder.measured_rpm,DualDrive_handle->motor_right.set_rpm);
	printf("-----------------------\r\n");

#endif

	target->old_centroid_angle = target->centroid_angle;
	return HAL_OK;
}


void RobotStrategy_Task(void *argument)
{
	RobotStrategy_InitTarget(&Target_Handle);
	// to let other task initialize proprely before handling the values with the strategy
	vTaskDelay(pdMS_TO_TICKS(500));

	for (;;)
	{

		RobotStrategy_IdentifyClosestObject(&Target_Handle, &X4LIDAR_handle);
		RobotStrategy_CalculateMotorSpeed(&Target_Handle,&DualDrive_handle);

		vTaskDelay(pdMS_TO_TICKS(500));

	}
}


// Fonction pour calculer la médiane d'un tableau d'entiers
int RobotStrategy_CalculateMedian(int *values, int size)
{
    // Copier les valeurs dans un tableau temporaire
    int temp[size];
    memcpy(temp, values, size * sizeof(int));

    // Trier les valeurs
    for (int i = 0; i < size - 1; i++) {
        for (int j = i + 1; j < size; j++) {
            if (temp[i] > temp[j]) {
                int tmp = temp[i];
                temp[i] = temp[j];
                temp[j] = tmp;
            }
        }
    }

    // Retourner la médiane
    if (size % 2 == 0) {
        return (temp[size / 2 - 1] + temp[size / 2]) / 2;
    } else {
        return temp[size / 2];
    }
}

// Fonction pour appliquer le filtre médian
void RobotStrategy_MedianFilter(float unfiltered_buffer[MAX_ANGLE], int filtered_buffer[MAX_ANGLE])
{
    int size = MAX_ANGLE;

    int int_unfiltered_buffer[MAX_ANGLE];

    // Cast float buffer to int buffer
    for (int i = 0; i < MAX_ANGLE; i++) {
    	int_unfiltered_buffer[i] = (int)unfiltered_buffer[i]; // Truncate fractional parts
    }

    for (int i = 0; i < size; i++) {
        int window[MEDIAN_FILTER_WINDOW_SIZE];
        int window_count = 0;

        // Fill the sliding window
        for (int j = -MEDIAN_FILTER_WINDOW_SIZE / 2; j <= MEDIAN_FILTER_WINDOW_SIZE / 2; j++) {
            int index = i + j;

            // Handle out-of-bounds indices with wraparound
            if (index < 0) index += size;
            if (index >= size) index -= size;

            if (int_unfiltered_buffer[index] != 0) { // Ignore zeros
                window[window_count++] = int_unfiltered_buffer[index];
            }
        }

        // Calculate the median if the window is not empty
        if (window_count > 0) {
            int median = RobotStrategy_CalculateMedian(window, window_count);
            if (int_unfiltered_buffer[i] == 0) {
                filtered_buffer[i] = median; // Replace zero with median
            } else {
                filtered_buffer[i] = int_unfiltered_buffer[i]; // Retain original value
            }
        } else {
            filtered_buffer[i] = int_unfiltered_buffer[i]; // Retain original value
        }
    }
}
