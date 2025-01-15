/**
 * @file RobotStrategy.c
 * @brief Implementation of the robot's strategy logic, including task creation,
 * initialization, and target handling.
 *
 * This file contains the implementation of functions required for the robot's
 * strategy, including task management, target initialization, and centroid
 * angle calculations.
 *
 * @date Created on: Dec 18, 2024
 * @author exysta
 */

#include "RobotStrategy.h"
#include "DCMotor_driver.h"
#include "X4LIDAR_driver.h"
#include "semphr.h"
#include <distSensor_driver.h>
#include "main.h"
#include "usart.h"
#include <stdlib.h>

// Uncomment to enable debug print statements
//#define PRINT_DEBUG

/** @brief Array to hold multiple strategy definitions. */
__STRATEGY_HandleTypeDef strategies[3];

/** @brief FreeRTOS task handle for the robot strategy task. */
TaskHandle_t RobotStategy_task_handle;

/** @brief Static Task Control Block for the robot strategy task. */
StaticTask_t RobotStategy_task_tcb;

/** @brief Static stack for the robot strategy task. */
StackType_t RobotStategy_task_stack[STATEGY_STACK_SIZE];

/** @brief External handle for the LiDAR sensor. */
extern X4LIDAR_handle_t X4LIDAR_handle;

/** @brief External handle for the dual motor drive system. */
extern DualDrive_handle_t DualDrive_handle;

/** @brief External handle for the current detected target. */
extern __TARGET_HandleTypeDef Target_Handle;

/** @brief External handles for directional distance sensors. */
extern DistSensor_handle_t Sens_N; /**< North sensor handle. */
extern DistSensor_handle_t Sens_S; /**< South sensor handle. */
extern DistSensor_handle_t Sens_W; /**< West sensor handle. */
extern DistSensor_handle_t Sens_E; /**< East sensor handle. */

/**
 * @brief Flag indicating whether the robot is currently braking.
 *
 * This is used to prevent rapid switching between braking and speed set to zero.
 */
extern volatile uint8_t is_robot_braking;

/**
 * @brief Creates the robot strategy task using a static stack.
 *
 * This function creates the task responsible for managing the robot's strategy
 * using FreeRTOS's static task creation API.
 *
 * @return HAL_OK if task creation succeeds, HAL_ERROR otherwise.
 */
HAL_StatusTypeDef RobotStrategy_CreateTask()
{
    // Create the task with a static stack
    RobotStategy_task_handle = xTaskCreateStatic(
        RobotStrategy_Task,         // Task function
        "RobotStrategy_Task",       // Task name
        STATEGY_STACK_SIZE,         // Stack size
        NULL,                       // Parameters to task
        STRATEGY_TASK_PRIORITY,     // Task priority
        RobotStategy_task_stack,    // Stack buffer
        &RobotStategy_task_tcb      // TCB buffer
    );

    // Check if task creation was successful
    if (RobotStategy_task_handle == NULL)
    {
        return HAL_ERROR; // Task creation failed
    }

    return HAL_OK; // Task created successfully
}

/**
 * @brief Placeholder for strategy module initialization.
 *
 * This function can be extended to include initialization steps for the strategy module.
 *
 * @return HAL_OK to indicate successful initialization.
 */
HAL_StatusTypeDef RobotStrategy__Init()
{
    return HAL_OK;
}

/**
 * @brief Initializes a target handle structure.
 *
 * Sets all fields of the target handle to their default values, including distance
 * metrics, angles, and historical data.
 *
 * @param[out] target Pointer to the target handle structure to initialize.
 */
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

/**
 * @brief Calculates the centroid angle for a detected target.
 *
 * This function determines the centroid angle of a detected target based on its
 * start and end angles. It handles the special case where the target wraps
 * around the 0-degree angle (forward direction).
 *
 * @param[in] start_angle Start angle of the target in degrees.
 * @param[in] end_angle End angle of the target in degrees.
 * @return Centroid angle of the target in degrees, adjusted for wraparound.
 */
static uint16_t RobotStrategy_CalculateCentroidAngle(uint16_t start_angle, uint16_t end_angle)
{
    // Handle wraparound where the target spans across the 0-degree angle
    if (end_angle < start_angle)
    {
        end_angle += 360;
    }

    // Calculate the midpoint angle
    uint16_t centroid_angle = (start_angle + end_angle) / 2;

    // Wrap back to the range [0, 360) if necessary
    if (centroid_angle >= 360) {
        centroid_angle -= 360;
    }

    return centroid_angle;
}


/**
 * @brief Function to detect the closest object and pinpoint the tracking target.
 *
 * This function processes the LiDAR scan data, filters it, and identifies the object
 * with the closest distance within a defined range. It calculates the start and end
 * angles of the detected object and determines its centroid angle.
 *
 * The LiDAR motor operates at a revolution speed of 7Hz, and this function is
 * typically called every 0.1 seconds for proper object tracking.
 *
 * @param target Pointer to the target structure where the closest object details will be stored.
 * @param X4LIDAR_handle Pointer to the LiDAR handle containing the scan data.
 * @return HAL_OK on success, HAL_ERROR on failure.
 */
HAL_StatusTypeDef RobotStrategy_IdentifyClosestObject(__TARGET_HandleTypeDef * target, X4LIDAR_handle_t *X4LIDAR_handle)
{
    // Copy the distance buffer to avoid modifying the original buffer
    int distance_buffer_copy[MAX_ANGLE];
    float min_distance = MAX_OBJECT_DIST;

    // Apply median filter to the LiDAR scan data to reduce noise
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

    // Determine the end angle of the object
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

    // Determine the start angle of the object
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

    // Calculate the centroid angle of the detected object
    target->centroid_angle = RobotStrategy_CalculateCentroidAngle(target->start_angle, target->end_angle);

    return HAL_OK;
}

/**
 * @brief Calculate the angle error for the robot to track the target.
 *
 * This function calculates the angle error between the robot's forward direction
 * and the target's centroid angle. It also determines the direction (left or right)
 * for the robot to turn in order to track the target.
 *
 * @param target Pointer to the target structure containing the centroid angle.
 * @return HAL_OK on success.
 */
static uint16_t RobotStrategy_CalculateAngleError(__TARGET_HandleTypeDef * target)
{
    uint16_t angle_error;

    // If the target is to the left of the robot, calculate the angle error
    if(target->centroid_angle >= MAX_ANGLE/2)
    {
        angle_error = MAX_ANGLE - target->centroid_angle;
        target->direction = LEFT; // Set direction to left
    }
    // If the target is to the right of the robot
    else
    {
        angle_error = target->centroid_angle;
        target->direction = RIGHT; // Set direction to right
    }
    target->angle_error = angle_error;
    return HAL_OK;
}

/**
 * @brief Calculate motor speeds for differential driving to track the target.
 *
 * This function computes the required motor speeds for the left and right wheels based on the
 * angle error and the robot's current direction to properly track the identified target.
 * A proportional control approach is used to adjust the speed difference between the motors.
 *
 * @param target Pointer to the target structure containing angle and direction information.
 * @param DualDrive_handle Pointer to the dual drive system that controls the motors.
 * @return HAL_OK on success.
 */
HAL_StatusTypeDef RobotStrategy_CalculateMotorSpeed(__TARGET_HandleTypeDef * target, DualDrive_handle_t * DualDrive_handle)
{
    // Calculate the angle error between the robot and the target
    RobotStrategy_CalculateAngleError(target);
    uint16_t angle_error = target->angle_error;

    // Base speed in percentage
    uint8_t base_speed = 70;
    // Calculate the speed difference using proportional control
    uint8_t speed_difference = (KP/3 * angle_error) + 0.5;

    uint8_t left_speed;
    uint8_t right_speed;

    // If target is to the left, the right wheel should move faster to turn left
    if(target->direction == LEFT)
    {
        left_speed   = base_speed - speed_difference;
        right_speed  = base_speed + speed_difference;
    }
    // If target is to the right, the left wheel should move faster to turn right
    else{
        left_speed   = base_speed + speed_difference;
        right_speed  = base_speed - speed_difference;
    }

    // Update motor speeds if the centroid angle has changed
    if(target->old_centroid_angle != target->centroid_angle)
    {
        DCMotor_SetSpeed(&DualDrive_handle ->motor_left, left_speed, POSITIVE_ROTATION);
        DCMotor_SetSpeed(&DualDrive_handle ->motor_right, right_speed, POSITIVE_ROTATION);
        xTaskNotifyGive(DualDrive_handle->h_task);
    }

#ifdef PRINT_DEBUG
    // Debug print statements to display motor speeds and target angles
    printf("start angle = %d , dist = %d\r\n\ ", target->start_angle, target->min_distance);
    printf("centroid angle = %d , dist = %d\r\n ", target->centroid_angle, target->avg_distance);
    printf("end angle = %d , dist = %d\r\n ", target->end_angle, target->max_distance);
    printf("left measured speed = %lu,left set speed = %lu \r\n", DualDrive_handle ->motor_left.encoder.measured_rpm, DualDrive_handle->motor_left.set_rpm);
    printf("right measured speed = %lu right set speed = %lu\r\n", DualDrive_handle ->motor_right.encoder.measured_rpm, DualDrive_handle->motor_right.set_rpm);
    printf("-----------------------\r\n");
#endif

    // Update the old centroid angle for the next cycle
    target->old_centroid_angle = target->centroid_angle;
    return HAL_OK;
}



/**
 * @brief FreeRTOS task function that manages the robot's strategy.
 *
 * This function is responsible for identifying the closest object, calculating the motor speeds
 * to track the target, and controlling the robot's movement. If the robot is braking, it will
 * delay and stop the motors. Otherwise, it will adjust the motor speeds based on the target's position.
 *
 * The task runs indefinitely with a delay of 100ms between iterations to periodically update the strategy.
 *
 * @param argument Unused task argument.
 */
void RobotStrategy_Task(void *argument)
{
    // Initialize the target structure for object detection
    RobotStrategy_InitTarget(&Target_Handle);

    // Allow other tasks to initialize before handling values
    vTaskDelay(pdMS_TO_TICKS(500));

    for (;;)
    {
        // Identify the closest object to the robot
        RobotStrategy_IdentifyClosestObject(&Target_Handle, &X4LIDAR_handle);

        // If the robot is not braking, calculate motor speeds to track the target
        if(!is_robot_braking)
        {
            RobotStrategy_CalculateMotorSpeed(&Target_Handle, &DualDrive_handle);
        }
        else
        {
            // Delay for 3 seconds when braking, then stop motors
            vTaskDelay(pdMS_TO_TICKS(3000));

            is_robot_braking = 0;
            DCMotor_SetSpeed(&DualDrive_handle.motor_left, 0, POSITIVE_ROTATION);
            DCMotor_SetSpeed(&DualDrive_handle.motor_right, 0, POSITIVE_ROTATION);
        }

			is_robot_braking = 0;
			DCMotor_SetSpeed(&DualDrive_handle.motor_left, 0, POSITIVE_ROTATION);
			DCMotor_SetSpeed(&DualDrive_handle.motor_right, 0, POSITIVE_ROTATION);


		}

		vTaskDelay(pdMS_TO_TICKS(100));

	}
}

/**
 * @brief Function to calculate the median of an array of integers.
 *
 * This function copies the input array, sorts it, and returns the median value.
 * If the size is even, it returns the average of the two middle values.
 *
 * @param values Input array of integers.
 * @param size The size of the array.
 * @return The median value of the array.
 */
int RobotStrategy_CalculateMedian(int *values, int size)
{
    // Copy the values into a temporary array
    int temp[size];
    memcpy(temp, values, size * sizeof(int));

    // Sort the array
    for (int i = 0; i < size - 1; i++) {
        for (int j = i + 1; j < size; j++) {
            if (temp[i] > temp[j]) {
                int tmp = temp[i];
                temp[i] = temp[j];
                temp[j] = tmp;
            }
        }
    }

    // Return the median value
    if (size % 2 == 0) {
        return (temp[size / 2 - 1] + temp[size / 2]) / 2;
    } else {
        return temp[size / 2];
    }
}

/**
 * @brief Apply a median filter to the LiDAR scan data to reduce noise.
 *
 * This function applies a sliding window median filter to the LiDAR scan data,
 * replacing noisy data points with the median value from neighboring points.
 * The filter uses a window of a predefined size and handles wraparound of array indices.
 *
 * @param unfiltered_buffer Input buffer containing the raw LiDAR data.
 * @param filtered_buffer Output buffer where the filtered data will be stored.
 */
void RobotStrategy_MedianFilter(float unfiltered_buffer[MAX_ANGLE], int filtered_buffer[MAX_ANGLE])
{
    int size = MAX_ANGLE;

    int int_unfiltered_buffer[MAX_ANGLE];

    // Convert the float buffer to an integer buffer
    for (int i = 0; i < MAX_ANGLE; i++) {
        int_unfiltered_buffer[i] = (int)unfiltered_buffer[i]; // Truncate fractional parts
    }

    // Apply the median filter to each data point
    for (int i = 0; i < size; i++) {
        int window[MEDIAN_FILTER_WINDOW_SIZE];
        int window_count = 0;

        // Fill the sliding window with neighboring data points
        for (int j = -MEDIAN_FILTER_WINDOW_SIZE / 2; j <= MEDIAN_FILTER_WINDOW_SIZE / 2; j++) {
            int index = i + j;

            // Handle out-of-bounds indices using wraparound
            if (index < 0) index += size;
            if (index >= size) index -= size;

            if (int_unfiltered_buffer[index] != 0) { // Ignore zeros
                window[window_count++] = int_unfiltered_buffer[index];
            }
        }

        // Calculate the median if the window contains valid data
        if (window_count > 0) {
            int median = RobotStrategy_CalculateMedian(window, window_count);
            if (int_unfiltered_buffer[i] == 0) {
                filtered_buffer[i] = median; // Replace zero with median value
            } else {
                filtered_buffer[i] = int_unfiltered_buffer[i]; // Retain original value
            }
        } else {
            filtered_buffer[i] = int_unfiltered_buffer[i]; // Retain original value
        }
    }
}

