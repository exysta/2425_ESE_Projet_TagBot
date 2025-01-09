/**
 * @file X4LIDAR_driver.h
 * @brief Driver header for the X4 LiDAR sensor.
 *
 * This header file defines structures, constants, and functions to interact with the
 * X4 LiDAR sensor over UART. It includes functionality to control the sensor, retrieve
 * device information, manage scanning operations, and process scan data.
 *
 * @date Created on: Nov 26, 2024
 * @author exysta
 */

#ifndef X4LIDAR_DRIVER_H_
#define X4LIDAR_DRIVER_H_

#include "main.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"

/** @brief Command prefix for X4 LiDAR communication. */
#define X4LIDAR_CMD_PREFIX                        0xA5

/** @brief Command to start scanning. */
#define X4LIDAR_CMD_START_SCAN                    0x60

/** @brief Command to stop scanning. */
#define X4LIDAR_CMD_STOP_SCAN                     0x65

/** @brief Command to get device information. */
#define X4LIDAR_CMD_GET_DEVICE_INFO               0x90

/** @brief Command to get the health status of the device. */
#define X4LIDAR_CMD_GET_HEALTH_STATUS             0x91

/** @brief Command to perform a soft restart of the device. */
#define X4LIDAR_CMD_SOFT_RESTART                  0x80

/** @brief Size of the response header. */
#define RESPONSE_HEADER_SIZE                      7

/** @brief Size of the payload containing device information. */
#define DEVICE_INFORMATION_PAYLOAD_SIZE           20

/** @brief Size of the payload containing health status data. */
#define HEALTH_STATUS_PAYLOAD_SIZE                3

/** @brief Size of the header in scan content data packets. */
#define SCAN_CONTENT_HEADER_SIZE                  10

/** @brief Index of the first byte in the response header. */
#define RESPONSE_HEADER_PH_1_INDEX                0

/** @brief Index of the second byte in the response header. */
#define RESPONSE_HEADER_PH_2_INDEX                1

/** @brief Index of the length bytes in the response header. */
#define RESPONSE_HEADER_LEN_1_INDEX               2
#define RESPONSE_HEADER_LEN_2_INDEX               3
#define RESPONSE_HEADER_LEN_3_INDEX               4
#define RESPONSE_HEADER_LEN_4_INDEX               5

/** @brief Index of the type code in the response header. */
#define RESPONSE_HEADER_TYPECODE_INDEX            6

/** @brief Index of the first byte in the scan content header. */
#define SCAN_CONTENT_HEADER_PH_1_INDEX            0

/** @brief Index of the second byte in the scan content header. */
#define SCAN_CONTENT_HEADER_PH_2_INDEX            1

/** @brief Index of the sample quantity in the scan content header. */
#define SCAN_CONTENT_HEADER_CT_INDEX              2

/** @brief Index of the sample quantity in the scan content header. */
#define SCAN_CONTENT_HEADER_LSN_INDEX             3

/** @brief Index of the start angle in the scan content header. */
#define SCAN_CONTENT_HEADER_FSA_1_INDEX           4
#define SCAN_CONTENT_HEADER_FSA_2_INDEX           5

/** @brief Index of the end angle in the scan content header. */
#define SCAN_CONTENT_HEADER_LSA_1_INDEX           6
#define SCAN_CONTENT_HEADER_LSA_2_INDEX           7

/** @brief Checksum index in the scan content header. */
#define SCAN_CONTENT_HEADER_CS_1_INDEX            8
#define SCAN_CONTENT_HEADER_CS_2_INDEX            9

/** @brief Expected values for the header bytes in the scan content. */
#define SCAN_CONTENT_HEADER_PH_1_VALUE            0xAA
#define SCAN_CONTENT_HEADER_PH_2_VALUE            0x55

/** @brief Type code for the reply message to the start scan command. */
#define SCAN_COMMAND_REPLY_TYPE_CODE              0x81

/** @brief Value indicating a start packet in scan content data. */
#define SCAN_CONTENT_CT_START_PACKET              1

/** @brief Value indicating a data packet in scan content data. */
#define SCAN_CONTENT_CT_DATA_PACKET               0

/** @brief Baud rate for UART communication with the LiDAR sensor. */
#define LIDAR_BAUDRATE                            128000

/** @brief Size of the DMA buffer for storing scan content data. */
#define SCAN_CONTENT_DMA_BUFFER_SIZE              500

/** @brief Size of the buffer storing the starting indices of data frames. */
#define SCAN_CONTENT_DATA_START_IDX_BUFFER_SIZE   10

/** @brief Stack size for the LiDAR task in FreeRTOS. */
#define LIDAR_STACK_SIZE                          256

/** @brief Priority of the LiDAR task in FreeRTOS. */
#define LIDAR_TASK_PRIORITY                       7

/** @brief Minimum and maximum angle for LiDAR scanning (in degrees). */
#define MIN_ANGLE                                 0
#define MAX_ANGLE                                 360

/** @brief Size of the DMA buffer for scan data. */
#define SCAN_DMA_BUFFER_SIZE                      500

/** @brief Size of the LiDAR sensor's serial number. */
#define X4_SERIAL_NUMBER_SIZE                     16

/** @brief Size of the firmware version information. */
#define X4_SERIAL_FIRMWARE_SIZE                   2

/**
 * @brief Structure to represent the response header from the LiDAR.
 *
 * This structure holds the header data for responses received from the X4 LiDAR sensor.
 */
typedef struct X4LIDAR_response_header
{
    uint16_t    start_sign;        /**< Start signature of the response. */
    uint32_t    content_size;      /**< Size of the content in the response. */
    uint8_t     mode;              /**< Mode of operation for the LiDAR. */
    uint8_t     type_code;         /**< Type code of the response. */
    uint8_t     buffer[RESPONSE_HEADER_SIZE]; /**< Buffer holding the full response header. */
} X4LIDAR_response_header;

/**
 * @brief Structure to store device information for the X4 LiDAR sensor.
 */
typedef struct X4LIDAR_device_info
{
    uint8_t model;                          /**< Model of the LiDAR. */
    uint8_t firmware[X4_SERIAL_FIRMWARE_SIZE]; /**< Firmware version of the LiDAR. */
    uint8_t hardware_version;               /**< Hardware version of the LiDAR. */
    uint8_t serial_number[X4_SERIAL_NUMBER_SIZE]; /**< Serial number of the LiDAR. */
} X4LIDAR_device_info;

/**
 * @brief Structure to store the scan header data from the X4 LiDAR.
 */
typedef struct X4LIDAR_scan_header
{
    uint16_t packet_header;       /**< Header of the scan data packet. */
    uint8_t packet_type;          /**< Type of the scan data packet. */
    uint8_t sample_quantity;      /**< Number of samples in the scan data. */
    uint16_t start_angle;         /**< Start angle of the scan. */
    uint16_t end_angle;           /**< End angle of the scan. */
    uint16_t check_code;          /**< Checksum for data verification. */
} X4LIDAR_scan_header;

/**
 * @brief Structure to store scan data from the X4 LiDAR sensor.
 */
typedef struct X4LIDAR_scan_data
{
    float distances[MAX_ANGLE];   /**< Array to store the distances for each angle in the scan. */
    uint8_t dma_buffer[SCAN_DMA_BUFFER_SIZE]; /**< DMA buffer for storing scan data. */
    X4LIDAR_scan_header scan_header; /**< Header information for the scan data. */
    volatile uint16_t start_idx; /**< Start index for the current scan data. */
    volatile uint16_t end_idx;   /**< End index for the current scan data. */
    uint16_t message_quantity;   /**< Number of data messages found in the buffer. */
    uint16_t data_frame_start_idx_buffer[SCAN_CONTENT_DATA_START_IDX_BUFFER_SIZE]; /**< Buffer for start indices of data frames. */
    uint16_t current_data_frame_start_idx; /**< Current index of the data frame being processed. */
    uint16_t current_data_frame_end_idx; /**< Current end index of the data frame being processed. */
    int trame_id; /**< Frame ID of the scan data. */
} X4LIDAR_scan_data;

/**
 * @brief Handle structure for managing the X4 LiDAR sensor.
 */
typedef struct X4LIDAR_handle_t
{
    X4LIDAR_device_info device_info; /**< Device information for the LiDAR. */
    uint8_t health_status;           /**< Health status of the LiDAR. */
    uint8_t health_error_code[2];    /**< Error codes related to health status. */
    X4LIDAR_response_header response_header; /**< Response header from the LiDAR. */
    UART_HandleTypeDef *huart;       /**< UART handle for communication with the LiDAR. */
    X4LIDAR_scan_data scan_data;     /**< Scan data structure. */
    TaskHandle_t task_handle;        /**< FreeRTOS task handle for LiDAR operations. */
    StaticTask_t task_tcb;           /**< Static task control block for LiDAR task. */
    StackType_t task_stack[LIDAR_STACK_SIZE]; /**< Static stack for LiDAR task. */
} X4LIDAR_handle_t;

/** @brief External handle for the X4 LiDAR sensor. */
extern X4LIDAR_handle_t X4LIDAR_handle;

/** @brief Initializes the X4 LiDAR sensor. */
HAL_StatusTypeDef X4LIDAR_init(X4LIDAR_handle_t *X4LIDAR_handle, UART_HandleTypeDef *huart);

/** @brief Task function for handling LiDAR operations in FreeRTOS. */
void X4LIDAR_task(void *argument);

/** @brief Creates the LiDAR task in FreeRTOS. */
HAL_StatusTypeDef X4LIDAR_create_task(X4LIDAR_handle_t *X4LIDAR_handle);

/** @brief Callback function for UART RX half completion. */
void X4LIDAR_HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart, X4LIDAR_handle_t *X4LIDAR_handle);

/** @brief Callback function for UART RX completion. */
void X4LIDAR_HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart, X4LIDAR_handle_t *X4LIDAR_handle);

#endif /* X4LIDAR_DRIVER_H_ */
