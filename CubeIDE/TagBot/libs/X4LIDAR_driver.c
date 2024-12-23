/*
 * LIDARX4_driver.c
 *
 *  Created on: Nov 26, 2024
 *      Author: exysta
 */

#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "X4LIDAR_driver.h"
#include <math.h>   // For atan function

//#define PRINT_DEBUG

#define CONSTANT_1 21.8f  // Precompute the constant
#define CONSTANT_2 155.3f // Precompute the divisor constant

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

HAL_StatusTypeDef X4LIDAR_parse_response_header(
		X4LIDAR_handle_t *X4LIDAR_handle)
{
	X4LIDAR_handle->response_header.start_sign =
			X4LIDAR_handle->response_header.buffer[RESPONSE_HEADER_PH_1_INDEX]
												   << 8
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
			RESPONSE_HEADER_SIZE + DEVICE_INFORMATION_PAYLOAD_SIZE, 300) != HAL_OK)
	{
		return HAL_ERROR;
	}
	X4LIDAR_handle->response_header.start_sign =
			rx_buffer[RESPONSE_HEADER_PH_1_INDEX] << 8
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

HAL_StatusTypeDef X4LIDAR_create_task(X4LIDAR_handle_t *X4LIDAR_handle)
{
	// Check for null handle
	if (X4LIDAR_handle == NULL)
	{
		return HAL_ERROR;
	}

	// Create the task with a static stack
	X4LIDAR_handle->task_handle = xTaskCreateStatic(X4LIDAR_task, // Task function
			"X4LIDAR_task",                           // Task name
			LIDAR_STACK_SIZE,                    // Stack size
			(void*) X4LIDAR_handle,                     // Parameters to task
			LIDAR_TASK_PRIORITY,                       // Task priority
			X4LIDAR_handle->task_stack,          // Stack buffer
			&X4LIDAR_handle->task_tcb            // TCB buffer
	);

	// Check if task creation was successful
	if (X4LIDAR_handle->task_handle == NULL)
	{
		return HAL_ERROR; // Task creation failed
	}

	return HAL_OK; // Task created successfully
}

HAL_StatusTypeDef X4LIDAR_init(X4LIDAR_handle_t *X4LIDAR_handle,
		UART_HandleTypeDef *huart)
{
	X4LIDAR_handle->huart = huart;

	X4LIDAR_handle->scan_data.start_idx = 0;
	X4LIDAR_handle->scan_data.end_idx = SCAN_CONTENT_DMA_BUFFER_SIZE / 2;

	X4LIDAR_handle->scan_data.trame_id = 0;

	X4LIDAR_send_command(X4LIDAR_handle, X4LIDAR_CMD_SOFT_RESTART);
	vTaskDelay(pdMS_TO_TICKS(20));

	X4LIDAR_get_device_info(X4LIDAR_handle);

	vTaskDelay(pdMS_TO_TICKS(200));


	//	HAL_GPIO_WritePin(OSC_TRIG_GPIO_Port, OSC_TRIG_Pin, RESET);
	return HAL_OK;
}

//this function analyze half of the buffer to detect the indexes start of data messages

HAL_StatusTypeDef X4LIDAR_get_data_start_indexes(
		X4LIDAR_handle_t *X4LIDAR_handle)
{
	X4LIDAR_handle->scan_data.message_quantity = 0;
	for (uint16_t i = X4LIDAR_handle->scan_data.start_idx;
			i < X4LIDAR_handle->scan_data.end_idx - 1; i++)
	{
		if (X4LIDAR_handle->scan_data.dma_buffer[i]
												 == SCAN_CONTENT_HEADER_PH_1_VALUE
												 && X4LIDAR_handle->scan_data.dma_buffer[i + 1]
																						 == SCAN_CONTENT_HEADER_PH_2_VALUE)
		{
			//we check if we overrun the index buffer before accessing its values
			if (X4LIDAR_handle->scan_data.message_quantity
					>= SCAN_CONTENT_DATA_START_IDX_BUFFER_SIZE)
			{

				return HAL_ERROR;
			}
			X4LIDAR_handle->scan_data.data_frame_start_idx_buffer[X4LIDAR_handle->scan_data.message_quantity] =
					i;
			X4LIDAR_handle->scan_data.message_quantity++;
		}
	}
	return HAL_OK;
}

//fill the scan_header for the current message
HAL_StatusTypeDef X4LIDAR_parse_data_frame_header(
		X4LIDAR_handle_t *X4LIDAR_handle)
{
	uint16_t header_index =
			X4LIDAR_handle->scan_data.current_data_frame_start_idx;
	//we check that we stay in the bond of the current part of dma_buffer
	if (header_index + SCAN_CONTENT_HEADER_SIZE
			> X4LIDAR_handle->scan_data.end_idx)
	{
		return HAL_ERROR;
	}
	X4LIDAR_handle->scan_data.scan_header.packet_header =
			X4LIDAR_handle->scan_data.dma_buffer[header_index
												 + SCAN_CONTENT_HEADER_PH_2_INDEX] << 8
												 | X4LIDAR_handle->scan_data.dma_buffer[header_index
																						+ SCAN_CONTENT_HEADER_PH_1_INDEX];
	X4LIDAR_handle->scan_data.scan_header.packet_type =
			X4LIDAR_handle->scan_data.dma_buffer[header_index
												 + SCAN_CONTENT_HEADER_CT_INDEX];
	X4LIDAR_handle->scan_data.scan_header.sample_quantity =
			X4LIDAR_handle->scan_data.dma_buffer[header_index
												 + SCAN_CONTENT_HEADER_LSN_INDEX];
	X4LIDAR_handle->scan_data.scan_header.start_angle =
			X4LIDAR_handle->scan_data.dma_buffer[header_index
												 + SCAN_CONTENT_HEADER_FSA_2_INDEX] << 8
												 | X4LIDAR_handle->scan_data.dma_buffer[header_index
																						+ SCAN_CONTENT_HEADER_FSA_1_INDEX];
	X4LIDAR_handle->scan_data.scan_header.end_angle =
			X4LIDAR_handle->scan_data.dma_buffer[header_index
												 + SCAN_CONTENT_HEADER_LSA_2_INDEX] << 8
												 | X4LIDAR_handle->scan_data.dma_buffer[header_index
																						+ SCAN_CONTENT_HEADER_LSA_1_INDEX];
	X4LIDAR_handle->scan_data.scan_header.check_code =
			X4LIDAR_handle->scan_data.dma_buffer[header_index
												 + SCAN_CONTENT_HEADER_CS_2_INDEX] << 8
												 | X4LIDAR_handle->scan_data.dma_buffer[header_index
																						+ SCAN_CONTENT_HEADER_CS_1_INDEX];
	return HAL_OK;
}

HAL_StatusTypeDef X4LIDAR_calculate_max_index(X4LIDAR_handle_t *X4LIDAR_handle)
{
	uint16_t data_end_index =
			X4LIDAR_handle->scan_data.current_data_frame_start_idx
			+ SCAN_CONTENT_HEADER_SIZE
			+ X4LIDAR_handle->scan_data.scan_header.sample_quantity * 2;
	//we check that we don't have invalid memory access for the dma_buffer
	if (data_end_index > X4LIDAR_handle->scan_data.end_idx)
	{
		X4LIDAR_handle->scan_data.current_data_frame_end_idx =
				X4LIDAR_handle->scan_data.end_idx;

	}
	else
	{
		X4LIDAR_handle->scan_data.current_data_frame_end_idx = data_end_index;
	}
	//since data point come in 2 bytes, we make sure that the last byte we take is not the first half of a data point
	if ((X4LIDAR_handle->scan_data.current_data_frame_end_idx % 2)
			== (X4LIDAR_handle->scan_data.current_data_frame_start_idx
					+ SCAN_CONTENT_HEADER_SIZE % 2))
	{
		X4LIDAR_handle->scan_data.current_data_frame_end_idx--;
	}
	return HAL_OK;
}

HAL_StatusTypeDef X4LIDAR_compute_payload(X4LIDAR_handle_t *X4LIDAR_handle)
{
	float start_angle =
			((float) (X4LIDAR_handle->scan_data.scan_header.start_angle >> 1))
			/ 64;
	float end_angle = ((float) (X4LIDAR_handle->scan_data.scan_header.end_angle
			>> 1)) / 64;
	if(X4LIDAR_handle->scan_data.scan_header.sample_quantity < 2)
	{
		return HAL_ERROR;
	}

	float diff_angle = end_angle - start_angle;
	float angle, distance,angle_correct;
	uint16_t distance_raw;
	if (diff_angle < 0) // Check not negative (one turn)
	{
		diff_angle = (diff_angle + 360.0)
						/ (X4LIDAR_handle->scan_data.scan_header.sample_quantity - 1);
	}
	else
	{
		diff_angle /= X4LIDAR_handle->scan_data.scan_header.sample_quantity;
	}
	uint8_t data_point_number = 0;
	uint8_t iteration_counter = 0; // Tracks iterations
	// Compute distance
	for (int idx = X4LIDAR_handle->scan_data.current_data_frame_start_idx
			+ SCAN_CONTENT_HEADER_SIZE;
			idx < X4LIDAR_handle->scan_data.current_data_frame_end_idx; idx+=2)
	{
		distance_raw = X4LIDAR_handle->scan_data.dma_buffer[idx];
		distance_raw |= X4LIDAR_handle->scan_data.dma_buffer[idx + 1] << 8;
		distance = ((float) distance_raw) / 4;
		angle = diff_angle * data_point_number + start_angle;
		if(distance == 0)
		{
			angle_correct = 0;
		}
		else
		{
			angle_correct = atan(CONSTANT_1*((CONSTANT_2 - distance)/(CONSTANT_2 * distance)));
		}
		angle +=  angle_correct;
		if ((uint32_t) angle > MAX_ANGLE)
		{
			angle = angle - MAX_ANGLE;

#ifdef PRINT_DEBUG

			printf("wrong angle detected\r\n");
#endif
		}
		uint32_t rounded_angle = (uint32_t)(angle + 0.5f); // For positive `angle`
		X4LIDAR_handle->scan_data.distances[ rounded_angle] = distance;
		// Increment data_point_number every two iterations
		if (iteration_counter % 2 == 1)
		{
			data_point_number++;
		}
		iteration_counter++; // Increment the iteration counter
	}

	return HAL_OK;
}

HAL_StatusTypeDef X4LIDAR_parse_buffer(X4LIDAR_handle_t *X4LIDAR_handle)
{
	for (int data_frame_idx = 0;
			data_frame_idx < X4LIDAR_handle->scan_data.message_quantity;
			data_frame_idx++)
	{
		X4LIDAR_handle->scan_data.current_data_frame_start_idx =
				X4LIDAR_handle->scan_data.data_frame_start_idx_buffer[data_frame_idx];

		if ((X4LIDAR_parse_data_frame_header(X4LIDAR_handle) != HAL_OK)
				|| (X4LIDAR_calculate_max_index(X4LIDAR_handle) != HAL_OK)
				||(X4LIDAR_compute_payload(X4LIDAR_handle) != HAL_OK))
		{
			return HAL_ERROR;
		}
	}
	return HAL_OK;
}

void X4LIDAR_task(void *argument)
{
	// Retrieve the handle (hlidar) passed as argument
	X4LIDAR_handle_t *X4LIDAR_handle = (X4LIDAR_handle_t*) argument;
	X4LIDAR_init(X4LIDAR_handle,&huart3);
	X4LIDAR_start_scan(X4LIDAR_handle);
	for (;;)
	{
		ulTaskNotifyTake(pdTRUE, 100);

		if (X4LIDAR_get_data_start_indexes(X4LIDAR_handle) != HAL_OK)
		{
#ifdef print_debug

			printf("Lidar error while getting scan data indexes\r\n");
#endif

			continue;  // Skip the rest of the loop and start next iteration

		}
#ifdef PRINT_DEBUG

		//		UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL); // NULL for current task
		//		printf("Remaining stack space: %u bytes\r\n", uxHighWaterMark);
#endif

		if (X4LIDAR_parse_buffer(X4LIDAR_handle) != HAL_OK)
		{
#ifdef PRINT_DEBUG
			printf("Lidar error while parsing buffer\r\n");
#endif
			continue;  // Skip the rest of the loop and start next iteration
		}


		vTaskDelay(pdMS_TO_TICKS(20));

	}
}

// to be called inside the callback function in the main.
//
void X4LIDAR_HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart,
		X4LIDAR_handle_t *X4LIDAR_handle)
{
	if (huart->Instance == X4LIDAR_handle->huart->Instance)
	{

		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		X4LIDAR_handle->scan_data.start_idx = 0;
		X4LIDAR_handle->scan_data.end_idx = SCAN_CONTENT_DMA_BUFFER_SIZE / 2;

		vTaskNotifyGiveFromISR(X4LIDAR_handle->task_handle,
				&xHigherPriorityTaskWoken);

		// Perform context switch if required
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

	}
}

void X4LIDAR_HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart,
		X4LIDAR_handle_t *X4LIDAR_handle)
{
	if (huart->Instance == X4LIDAR_handle->huart->Instance)
	{

		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		X4LIDAR_handle->scan_data.start_idx = SCAN_CONTENT_DMA_BUFFER_SIZE / 2;
		X4LIDAR_handle->scan_data.end_idx = SCAN_CONTENT_DMA_BUFFER_SIZE;

		vTaskNotifyGiveFromISR(X4LIDAR_handle->task_handle,
				&xHigherPriorityTaskWoken);

		// Perform context switch if required
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

	}
}
