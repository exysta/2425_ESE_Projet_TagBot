/*
 * ydlidar_x4.h
 *
 *  Created on: Feb 14, 2024
 *      Author: nicolas
 */

#ifndef INC_YDLIDAR_X4_H_
#define INC_YDLIDAR_X4_H_

#include "main.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>

#define CMD_PREFIX						0xA5
#define CMD_START_SCAN  				0x60
#define CMD_STOP_SCAN 	 				0x65
#define CMD_GET_DEVICE_INFO				0x90
#define CMD_GET_HEALTH_STATUS 			0x91
#define CMD_SOFT_RESTART 				0x80

#define HEADER_SIZE 					7
#define DEVICE_INFORMATION_PAYLOAD_SIZE	20
#define HEALTH_STATUS_PAYLOAD_SIZE		3
#define SCAN_CONTENT_HEADER_SIZE		10

#define X4_REPLY_TYPE_CODE_INDEX 	6 // index of the type code in the reply message
// the following define are for parsing the header of scan command

#define SCAN_CONTENT_HEADER_PH_1       0  // index of packet header 1
#define SCAN_CONTENT_HEADER_PH_2       1  // index of packet header 2
#define SCAN_CONTENT_HEADER_CT         2  // index of packet type. Indicates the number of sampling points contained in the current packet.
                                          // There is only one initial point of data in the zero packet. The value is 1.
#define SCAN_CONTENT_HEADER_LSN_INDEX  3  // index of sample_quantity
#define SCAN_CONTENT_HEADER_FSA_1      4  // index of start angle 1
#define SCAN_CONTENT_HEADER_FSA_2      5  // index of start angle 2
#define SCAN_CONTENT_HEADER_LSA_1      6  // index of start angle 1
#define SCAN_CONTENT_HEADER_LSA_2      7  // index of start angle 2
#define SCAN_CONTENT_HEADER_CS_1       8  // index of check code 1
#define SCAN_CONTENT_HEADER_CS_2       9  // index of check code 2


#define SCAN_COMMAND_REPLY_TYPE_CODE		0x81  //the type code of the reply message to the start command

#define SCAN_CONTENT_CT_START_PACKET		1  //the value of package tpye for the scan header if the packet data is a start packet


#define LAST_DATA_BUFFER_SIZE			10

#define LIDAR_BAUDRATE 					128000

#define RX_BUFFER_SIZE					100

typedef enum {
	IDLE,
	STOP,
	START_SYNC_CONTENT_HEADER, // for the first receive where header is message header + scan_data header of len 7 + 10
	START_SCAN_DATA_HEADER, // to handle scan header begining by t 0x55AA
	START_WAIT_CONTENT,

} YDLIDAR_State;

typedef struct YDLIDAR_Scan_Response{
	uint8_t 	id_data;
	uint8_t		packet_header[2];
	uint8_t		package_type;
	uint8_t 	sample_quantity;
	uint16_t 	start_angle;
	uint16_t 	end_angle;
	uint16_t 	check_code;
	uint8_t scan_header_buffer[SCAN_CONTENT_HEADER_SIZE];
	uint8_t * scan_content_buffer;

	float 		distance[360];
}YDLIDAR_Scan_Response;

typedef struct __YDLIDAR_X4_HandleTypeDef
{
	UART_HandleTypeDef *huart;
	YDLIDAR_State state;
	uint8_t model;
	uint8_t firmware_major;
	uint8_t firmware_minor;
	uint8_t hardware;
	uint8_t	serial_number[16];
	uint8_t health_status;
	uint8_t health_error_code[2];

	uint8_t header_buffer[HEADER_SIZE];
	uint8_t rx_buffer[RX_BUFFER_SIZE];

	YDLIDAR_Scan_Response scan_response;
	uint8_t newData;
	int trame_id;
} __YDLIDAR_X4_HandleTypeDef;

typedef struct YDLIDAR_header_response{
	uint8_t		start_sign[2];
	uint32_t	content_size;
	uint8_t 	mode;
	uint8_t 	type_code;
}YDLIDAR_header_response;



extern __YDLIDAR_X4_HandleTypeDef hlidar;

HAL_StatusTypeDef YDLIDAR_X4_Send_Cmd(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle, uint8_t cmd);
HAL_StatusTypeDef YDLIDAR_X4_Init(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle, UART_HandleTypeDef *huart);
HAL_StatusTypeDef YDLIDAR_X4_Get_Device_Informations(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle);
HAL_StatusTypeDef YDLIDAR_X4_Print_Device_Informations(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle);
HAL_StatusTypeDef YDLIDAR_X4_Get_Health_Status(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle);
HAL_StatusTypeDef YDLIDAR_X4_Print_Health_Status(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle);
HAL_StatusTypeDef YDLIDAR_X4_Start_Scan(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle);
HAL_StatusTypeDef YDLIDAR_X4_Soft_Reboot(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle);
HAL_StatusTypeDef YDLIDAR_X4_Compute_Payload(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle);
HAL_StatusTypeDef YDLIDAR_X4_State_Machine(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle);

void UART_Processing_Task(void *argument);
void LiDAR_Processing_Task(void *argument);


#endif /* INC_YDLIDAR_X4_H_ */
