/*
 * X4LIDAR_driver.h
 *
 *  Created on: Nov 26, 2024
 *      Author: exysta
 */

#ifndef X4LIDAR_DRIVER_H_
#define X4LIDAR_DRIVER_H_

#include "main.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"

#define X4LIDAR_CMD_PREFIX						0xA5
#define X4LIDAR_CMD_START_SCAN  				0x60
#define X4LIDAR_CMD_STOP_SCAN 	 				0x65
#define X4LIDAR_CMD_GET_DEVICE_INFO				0x90
#define X4LIDAR_CMD_GET_HEALTH_STATUS 			0x91
#define X4LIDAR_CMD_SOFT_RESTART 				0x80

#define RESPONSE_HEADER_SIZE 					7
#define DEVICE_INFORMATION_PAYLOAD_SIZE	20
#define HEALTH_STATUS_PAYLOAD_SIZE		3
#define SCAN_CONTENT_HEADER_SIZE		10

#define RESPONSE_HEADER_PH_1_INDEX       0
#define RESPONSE_HEADER_PH_2_INDEX       1
#define RESPONSE_HEADER_LEN_1_INDEX       2
#define RESPONSE_HEADER_LEN_2_INDEX       3
#define RESPONSE_HEADER_LEN_3_INDEX 	   4
#define RESPONSE_HEADER_LEN_4_INDEX	       5
#define RESPONSE_HEADER_TYPECODE_INDEX       6

#define SCAN_CONTENT_HEADER_PH_1_INDEX       0  // index of packet header 1
#define SCAN_CONTENT_HEADER_PH_2_INDEX        1  // index of packet header 2
#define SCAN_CONTENT_HEADER_CT_INDEX          2  // index of packet type. Indicates the number of sampling points contained in the current packet.
// There is only one initial point of data in the zero packet. The value is 1.
#define SCAN_CONTENT_HEADER_LSN_INDEX   3  // index of sample_quantity
#define SCAN_CONTENT_HEADER_FSA_1_INDEX       4  // index of start angle 1
#define SCAN_CONTENT_HEADER_FSA_2_INDEX       5  // index of start angle 2
#define SCAN_CONTENT_HEADER_LSA_1_INDEX       6  // index of start angle 1
#define SCAN_CONTENT_HEADER_LSA_2_INDEX       7  // index of start angle 2
#define SCAN_CONTENT_HEADER_CS_1_INDEX        8  // index of check code 1
#define SCAN_CONTENT_HEADER_CS_2_INDEX        9  // index of check code 2

#define SCAN_CONTENT_HEADER_PH_1_VALUE       0xAA  // index of check code 1
#define SCAN_CONTENT_HEADER_PH_2_VALUE       0x55  // index of check code 2

#define SCAN_COMMAND_REPLY_TYPE_CODE		0x81  //the type code of the reply message to the start command

#define SCAN_CONTENT_CT_START_PACKET		1  //the value of package tpye for the scan header if the packet data is a start packet
#define SCAN_CONTENT_CT_DATA_PACKET		0  //the value of package tpye for the scan header if the packet data is a start packet

#define LIDAR_BAUDRATE 					128000

#define SCAN_CONTENT_BUFFER_SIZE					200

#define SCAN_CONTENT_DMA_BUFFER_SIZE					500
#define SCAN_CONTENT_DATA_START_IDX_BUFFER_SIZE					10

#define LIDAR_STACK_SIZE 						512

#define LIDAR_TASK_PRIORITY 			3

#define MIN_ANGLE 			0
#define MAX_ANGLE 			360

#define SCAN_DMA_BUFFER_SIZE 			500


/**
 * @def X4_SERIAL_NUMBER_SIZE
 * @brief Size of the LiDAR serial number.
 */
#define X4_SERIAL_NUMBER_SIZE 16

/**
 * @def X4_SERIAL_FIRMWARE_SIZE
 * @brief Size of the firmware version information.
 */
#define X4_SERIAL_FIRMWARE_SIZE 2

//check if we are processing the first half or the second half of the buffer
typedef enum {
	SCAN_DATA_HALF_CPLT,
	SCAN_DATA_FULL_CPLT
} DMA_State;

typedef struct X4LIDAR_response_header
{
	uint16_t		start_sign;
	uint32_t	content_size;
	uint8_t 	mode;
	uint8_t 	type_code;
	uint8_t buffer[RESPONSE_HEADER_SIZE];
}X4LIDAR_response_header;

/**
 * @brief Structure to store device information for the X4 LiDAR.
 */
typedef struct X4LIDAR_device_info{
	uint8_t model;                          /**< Model of the LiDAR. */
	uint8_t firmware[X4_SERIAL_FIRMWARE_SIZE]; /**< Firmware version of the LiDAR. */
	uint8_t hardware_version;               /**< Hardware version of the LiDAR. */
	uint8_t serial_number[X4_SERIAL_NUMBER_SIZE]; /**< Serial number of the LiDAR. */
} X4LIDAR_device_info;

/**
 * @brief Structure to store parsed scan data from the X4 LiDAR.
 */
typedef struct X4LIDAR_scan_header{
	uint16_t packet_header;       /**< Header of the scan data packet. */
	uint8_t packet_type;            /**< Type of the scan data packet. */
	uint8_t sample_quantity;        /**< Number of samples in the scan data. */
	uint16_t start_angle;         /**< Start angle of the scan. */
	uint16_t end_angle;           /**< End angle of the scan. */
	uint16_t check_code;          /**< Checksum for data verification. */
} X4LIDAR_scan_header;

/**
 * @brief Structure to store parsed scan data from the X4 LiDAR.
 */
typedef struct X4LIDAR_scan_data{
	uint8_t distances[MAX_ANGLE];
	uint8_t dma_buffer[SCAN_DMA_BUFFER_SIZE];
	X4LIDAR_scan_header scan_header;
	uint16_t start_idx;
	uint16_t end_idx;
	//indice du nombre de messages trouvés dans la moitié du buffer en train d'etre process
	uint16_t message_quantity;
	//buffer contenant les indices de tout les messages de data trouvés
	uint16_t data_start_idx_buffer[SCAN_CONTENT_DATA_START_IDX_BUFFER_SIZE];
	DMA_State dma_state;
	int trame_id;

} X4LIDAR_scan_data;


typedef struct X4LIDAR_handle_t{
	X4LIDAR_device_info device_info;
	uint8_t health_status;
	uint8_t health_error_code[2];
	X4LIDAR_response_header response_header;
	UART_HandleTypeDef *huart;
	X4LIDAR_scan_data scan_data;
    TaskHandle_t task_handle;               // Handle for the FreeRTOS task
    StaticTask_t task_tcb;                  // Static Task Control Block
    StackType_t task_stack[LIDAR_STACK_SIZE]; // Static Stack for the task

} X4LIDAR_handle_t;

#endif /* X4LIDAR_DRIVER_H_ */
