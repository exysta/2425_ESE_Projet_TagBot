/*
 * LIDARX4_driver.c
 *
 *  Created on: Nov 26, 2024
 *      Author: exysta
 */

#include <stdlib.h>
#include "cmsis_os.h"
#include "X4LIDAR_driver.h"

HAL_StatusTypeDef X4LIDAR_send_command(X4LIDAR_handle_t *X4LIDAR_handle,
		uint8_t command)
{
	uint8_t cmdBuffer[2] =
	{ X4LIDAR_CMD_PREFIX, command }; /**< Array holding the command data */
	if (HAL_UART_Transmit(X4LIDAR_handle->huart, cmdBuffer, 2, 10) == HAL_OK)
	{
		return HAL_OK;
	}
	else
	{
		return HAL_ERROR;
	}
}

HAL_StatusTypeDef X4LIDAR_parse_response_header(X4LIDAR_handle_t *X4LIDAR_handle)
{
	X4LIDAR_handle->response_header.start_sign =
			X4LIDAR_handle->response_header.buffer[RESPONSE_HEADER_PH_1_INDEX]
					>> 8
					| X4LIDAR_handle->response_header.buffer[RESPONSE_HEADER_PH_2_INDEX];

	X4LIDAR_handle->response_header.content_size =
			(X4LIDAR_handle->response_header.buffer[RESPONSE_HEADER_LEN_4_INDEX]
					& 0x3F) << 24
					| X4LIDAR_handle->response_header.buffer[RESPONSE_HEADER_LEN_3_INDEX]
							<< 16
					| X4LIDAR_handle->response_header.buffer[RESPONSE_HEADER_LEN_2_INDEX]
							<< 8
					| X4LIDAR_handle->response_header.buffer[RESPONSE_HEADER_LEN_1_INDEX];
	X4LIDAR_handle->response_header.mode =
			X4LIDAR_handle->response_header.buffer[RESPONSE_HEADER_LEN_4_INDEX]
					>> 6;
	X4LIDAR_handle->response_header.type_code =
			X4LIDAR_handle->response_header.buffer[RESPONSE_HEADER_TYPECODE_INDEX];
	return HAL_OK;


}
HAL_StatusTypeDef X4LIDAR_get_device_info(X4LIDAR_handle_t *X4LIDAR_handle)
{
	X4LIDAR_send_command(X4LIDAR_handle, X4LIDAR_CMD_STOP_SCAN);
	HAL_Delay(50);

	HAL_UART_Abort(X4LIDAR_handle->huart); // Clean every old data in Rx Buffer

	uint8_t rx_buffer[RESPONSE_HEADER_SIZE + DEVICE_INFORMATION_PAYLOAD_SIZE];

	X4LIDAR_send_command(X4LIDAR_handle, X4LIDAR_CMD_GET_DEVICE_INFO);
	if (HAL_UART_Receive(X4LIDAR_handle->huart, rx_buffer,
			RESPONSE_HEADER_SIZE + DEVICE_INFORMATION_PAYLOAD_SIZE, 300)
			!= HAL_OK)
	{
		return HAL_ERROR;
	}
	X4LIDAR_handle->response_header.start_sign =
			rx_buffer[RESPONSE_HEADER_PH_1_INDEX] >> 8
					| rx_buffer[RESPONSE_HEADER_PH_2_INDEX];

	X4LIDAR_handle->response_header.content_size =
			(rx_buffer[RESPONSE_HEADER_LEN_4_INDEX] & 0x3F) << 24
					| rx_buffer[RESPONSE_HEADER_LEN_3_INDEX] << 16
					| rx_buffer[RESPONSE_HEADER_LEN_2_INDEX] << 8
					| rx_buffer[RESPONSE_HEADER_LEN_1_INDEX];
	X4LIDAR_handle->response_header.mode =
			rx_buffer[RESPONSE_HEADER_LEN_4_INDEX] >> 6;
	X4LIDAR_handle->response_header.type_code =
			rx_buffer[RESPONSE_HEADER_TYPECODE_INDEX];

	if ((X4LIDAR_handle->response_header.start_sign == 0xA55A)
			&& (X4LIDAR_handle->response_header.content_size == 0x03)
			&& (X4LIDAR_handle->response_header.mode == 0x00)
			&& (X4LIDAR_handle->response_header.type_code == 0x06))
	{
		X4LIDAR_handle->device_info.model = rx_buffer[RESPONSE_HEADER_SIZE];
		X4LIDAR_handle->device_info.firmware[0] = rx_buffer[RESPONSE_HEADER_SIZE
				+ 1];
		X4LIDAR_handle->device_info.firmware[1] = rx_buffer[RESPONSE_HEADER_SIZE
				+ 2];
		X4LIDAR_handle->device_info.hardware_version =
				rx_buffer[RESPONSE_HEADER_SIZE + 3];
		for (int idx = 0; idx < 16; idx++)
		{
			X4LIDAR_handle->device_info.serial_number[idx] =
					rx_buffer[RESPONSE_HEADER_SIZE + 4 + idx];
		}

	}
	else
	{

		return HAL_ERROR;
	}

	return HAL_OK;
}

HAL_StatusTypeDef X4LIDAR_start_scan(X4LIDAR_handle_t *X4LIDAR_handle)
{

	X4LIDAR_send_command(X4LIDAR_handle, X4LIDAR_CMD_START_SCAN);

	HAL_UART_Receive(X4LIDAR_handle->huart,
			X4LIDAR_handle->response_header.buffer, RESPONSE_HEADER_SIZE, 200);
	X4LIDAR_parse_response_header(X4LIDAR_handle);

	if ((X4LIDAR_handle->response_header.start_sign == 0xA55A)
			&& (X4LIDAR_handle->response_header.mode == 0x01)
			&& (X4LIDAR_handle->response_header.type_code == 0x81))
	{
		HAL_UART_Receive_DMA(X4LIDAR_handle->huart,
				X4LIDAR_handle->scan_data.dma_buffer, SCAN_DMA_BUFFER_SIZE);
	}


	return HAL_OK;
}

HAL_StatusTypeDef X4LIDAR_init(X4LIDAR_handle_t *X4LIDAR_handle,
		UART_HandleTypeDef *huart)
{
	X4LIDAR_handle->huart = huart;

	X4LIDAR_handle->scan_data.data_start_idx = 0;
	X4LIDAR_handle->scan_data.start_idx = 0;
	X4LIDAR_handle->scan_data.end_idx = SCAN_CONTENT_BUFFER_SIZE / 2;
	X4LIDAR_handle->scan_data.dma_state = SCAN_DATA_FULL_CPLT;

	X4LIDAR_handle->scan_data.trame_id = 0;

	X4LIDAR_send_command(X4LIDAR_handle, X4LIDAR_CMD_SOFT_RESTART);
	HAL_Delay(20);

	X4LIDAR_get_device_info(X4LIDAR_handle);

	HAL_Delay(200);

	//	HAL_GPIO_WritePin(OSC_TRIG_GPIO_Port, OSC_TRIG_Pin, RESET);
	//X4LIDAR_Task_Create(X4LIDAR_handle);
	return HAL_OK;
}
