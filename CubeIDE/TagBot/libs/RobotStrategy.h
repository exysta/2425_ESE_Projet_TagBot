/**
 * @file RobotStrategy.h
 * @brief Header file for the robot's strategy module.
 *
 * This file defines constants, types, and function prototypes used for managing
 * the robot's strategy, including motion, object detection, and task handling.
 *
 * @date Created on: Dec 18, 2024
 * @author exysta
 */

#ifndef ROBOTSTRATEGY_H_
#define ROBOTSTRATEGY_H_

#include "main.h"
#include "X4LIDAR_driver.h"

/** @brief Minimum detectable distance by the LiDAR sensor in millimeters. */
#define MIN_OBJECT_DIST 120 // 0.12 m as per the datasheet.

/** @brief Maximum detectable distance by the LiDAR sensor in millimeters. */
#define MAX_OBJECT_DIST 10000 // 10 meters maximum.

/** @brief Distance threshold for segmenting objects in millimeters. */
#define OBJECT_DIST_THRESHOLD 100

/**
 * @brief Proportional control constant for differential motor driving.
 *
 * This constant defines the proportional factor (KP) for speed difference
 * adjustments based on angle error. For a maximum angle error of 180° and
 * a desired differential speed of 25% of max speed:
 * @code
 * KP = 25 / 180 = 0.138
 * @endcode
 * This value may need to be tuned experimentally.
 */
#define KP 0.138f

/** @brief Size of the median filter window for processing LiDAR data. */
#define MEDIAN_FILTER_WINDOW_SIZE 3

/** @brief Stack size for the strategy task in bytes. */
#define STATEGY_STACK_SIZE 1024

/** @brief Priority level for the strategy task. */
#define STRATEGY_TASK_PRIORITY 9

/**
 * @brief Enumeration for robot movement directions.
 */
typedef enum {
    LEFT = 0x00,    /**< Move to the left. */
    RIGHT = 0x01,   /**< Move to the right. */
    FORWARD = 0x03, /**< Move forward. */
    BACKWARD = 0x04 /**< Move backward. */
} DIRECTION_Enum;

/**
 * @brief Structure to hold information about detected targets.
 */
typedef struct __TARGET_HandleTypeDef {
    uint16_t start_angle;        /**< Start angle of the detected target in degrees. */
    uint16_t centroid_angle;     /**< Centroid angle of the target in degrees. */
    uint16_t end_angle;          /**< End angle of the detected target in degrees. */
    uint16_t angle_error;        /**< Difference between forward direction and target's centroid angle in degrees. */
    uint16_t min_distance;       /**< Minimum detected distance to the target in millimeters. */
    uint16_t max_distance;       /**< Maximum detected distance to the target in millimeters. */
    uint16_t avg_distance;       /**< Average distance to the target in millimeters. */
    DIRECTION_Enum direction;    /**< Target's direction relative to the robot. */
    uint16_t old_centroid_angle; /**< Previous centroid angle of the target in degrees. */
} __TARGET_HandleTypeDef;

/**
 * @brief Enumeration for robot strategies.
 */
typedef enum {
    NO_Strat = 0x00, /**< No strategy selected. */
    CAT_Strat = 0x01, /**< "Cat" strategy (details to be defined). */
    MOUSE_Strat = 0x02 /**< "Mouse" strategy (details to be defined). */
} STRATEGY_Enum;

/**
 * @brief Structure to define an action in the robot's strategy.
 */
typedef struct __ACTION_HandleTypeDef {
    uint32_t start_Time;         /**< Start time of the action in milliseconds. */
    uint32_t end_Time;           /**< End time of the action in milliseconds. */
    uint8_t enable_stop_lidar;   /**< Flag to enable/disable LiDAR during the action. */
    uint32_t speed_Motor_Left;   /**< Speed of the left motor. */
    uint32_t speed_Motor_Right;  /**< Speed of the right motor. */
} __ACTION_HandleTypeDef;

/**
 * @brief Structure to define a complete strategy with multiple actions.
 */
typedef struct __STRATEGY_HandleTypeDef {
    uint8_t id;                  /**< Unique identifier for the strategy. */
    uint8_t action_id;           /**< Current action identifier. */
    uint8_t action_nb;           /**< Total number of actions in the strategy. */
    __ACTION_HandleTypeDef action[10]; /**< Array of actions in the strategy. */
} __STRATEGY_HandleTypeDef;

/** @brief Instance of the current strategy being executed. */
extern __STRATEGY_HandleTypeDef actual_strategy;

/**
 * @brief Creates the strategy management task.
 *
 * @return HAL status indicating the success or failure of the task creation.
 */
HAL_StatusTypeDef RobotStrategy_CreateTask();

/**
 * @brief Main task for handling the robot's strategy logic.
 *
 * @param argument Pointer to task-specific arguments.
 */
void RobotStrategy_Task(void *argument);

/**
 * @brief Applies a median filter to LiDAR data.
 *
 * @param[in] unfiltered_buffer Input buffer containing raw data.
 * @param[out] filtered_buffer Output buffer to store filtered data.
 */
void RobotStrategy_MedianFilter(float unfiltered_buffer[MAX_ANGLE], int filtered_buffer[MAX_ANGLE]);

/**
 * @brief Calculates the median value from a set of integers.
 *
 * @param[in] values Array of integers.
 * @param[in] size Number of elements in the array.
 * @return Median value of the array.
 */
int RobotStrategy_CalculateMedian(int *values, int size);

#endif /* ROBOTSTRATEGY_H_ */
