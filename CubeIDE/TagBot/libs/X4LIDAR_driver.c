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

/**
 * @brief Sends a command to the X4 LiDAR sensor via UART.
 *
 * This function sends a single byte command to the LiDAR sensor over UART.
 *
 * @param X4LIDAR_handle Pointer to the LiDAR handle containing the UART configuration.
 * @param command The command byte to send.
 *
 * @return HAL_OK if the command was successfully transmitted, else HAL_ERROR.
 */
HAL_StatusTypeDef X4LIDAR_send_command(X4LIDAR_handle_t *X4LIDAR_handle,
		uint8_t command)
{
	uint8_t cmdBuffer[2] = { X4LIDAR_CMD_PREFIX, command }; /**< Array holding the command data */
	if (HAL_UART_Transmit(X4LIDAR_handle->huart, cmdBuffer, 2, 10) == HAL_OK)
	{
		return HAL_OK;
	}
	else
	{
		return HAL_ERROR;
	}
}

/**
 * @brief Parses the response header from the LiDAR.
 *
 * This function processes the received header from the LiDAR and extracts the start sign,
 * content size, mode, and type code.
 *
 * @param X4LIDAR_handle Pointer to the LiDAR handle containing the response buffer.
 *
 * @return HAL_OK if the response header was successfully parsed.
 */
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

/**
 * @brief Retrieves the device information from the LiDAR sensor.
 *
 * This function sends the command to get the device information and then processes the
 * received response. It extracts the model, firmware, hardware version, and serial number
 * from the response data.
 *
 * @param X4LIDAR_handle Pointer to the LiDAR handle to store the device info.
 *
 * @return HAL_OK if the device information was successfully retrieved, else HAL_ERROR.
 */
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
		X4LIDAR_handle->device_info.firmware[0] = rx_buffer[RESPONSE_HEADER_SIZE + 1];
		X4LIDAR_handle->device_info.firmware[1] = rx_buffer[RESPONSE_HEADER_SIZE + 2];
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

/**
 * @brief Starts the scanning process on the LiDAR sensor.
 *
 * This function sends the start scan command to the LiDAR sensor and initiates the
 * reception of scan data. It also configures DMA for efficient data transfer.
 *
 * @param X4LIDAR_handle Pointer to the LiDAR handle.
 *
 * @return HAL_OK if the scanning process was successfully started, else HAL_ERROR.
 */
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
/**
 * @brief Creates a FreeRTOS task for handling X4 LiDAR operations.
 *
 * This function initializes and creates a FreeRTOS task using a static stack and task control block (TCB).
 *
 * @param X4LIDAR_handle Pointer to the LiDAR handle structure.
 *
 * @return HAL_OK if the task was successfully created, else HAL_ERROR.
 */
HAL_StatusTypeDef X4LIDAR_create_task(X4LIDAR_handle_t *X4LIDAR_handle)
{
    // Check for null handle
    if (X4LIDAR_handle == NULL)
    {
        return HAL_ERROR;
    }

    // Create the task with a static stack
    X4LIDAR_handle->task_handle = xTaskCreateStatic(
        X4LIDAR_task,               // Task function
        "X4LIDAR_task",             // Task name
        LIDAR_STACK_SIZE,           // Stack size
        (void *)X4LIDAR_handle,     // Parameters to task
        LIDAR_TASK_PRIORITY,        // Task priority
        X4LIDAR_handle->task_stack, // Stack buffer
        &X4LIDAR_handle->task_tcb   // TCB buffer
    );

    // Check if task creation was successful
    if (X4LIDAR_handle->task_handle == NULL)
    {
        return HAL_ERROR; // Task creation failed
    }

    return HAL_OK; // Task created successfully
}

/**
 * @brief Initializes the X4 LiDAR handle and configures basic settings.
 *
 * This function sets up the UART handle, initializes scan data indices, performs
 * a software restart of the LiDAR, and retrieves device information.
 *
 * @param X4LIDAR_handle Pointer to the LiDAR handle structure.
 * @param huart Pointer to the UART handle used for communication.
 *
 * @return HAL_OK on successful initialization.
 */
HAL_StatusTypeDef X4LIDAR_init(X4LIDAR_handle_t *X4LIDAR_handle, UART_HandleTypeDef *huart)
{
    X4LIDAR_handle->huart = huart;

    // Initialize scan data indices
    X4LIDAR_handle->scan_data.start_idx = 0;
    X4LIDAR_handle->scan_data.end_idx = SCAN_CONTENT_DMA_BUFFER_SIZE / 2;
    X4LIDAR_handle->scan_data.trame_id = 0;

    // Perform a software restart
    X4LIDAR_send_command(X4LIDAR_handle, X4LIDAR_CMD_SOFT_RESTART);
    vTaskDelay(pdMS_TO_TICKS(20));

    // Retrieve device information
    X4LIDAR_get_device_info(X4LIDAR_handle);
    vTaskDelay(pdMS_TO_TICKS(200));

    return HAL_OK;
}

/**
 * @brief Detects the start indexes of data messages in the DMA buffer.
 *
 * This function analyzes a portion of the DMA buffer to locate the indexes
 * of the headers for incoming data messages.
 *
 * @param X4LIDAR_handle Pointer to the LiDAR handle structure.
 *
 * @return HAL_OK if data message headers are successfully identified, else HAL_ERROR.
 */
HAL_StatusTypeDef X4LIDAR_get_data_start_indexes(X4LIDAR_handle_t *X4LIDAR_handle)
{
    X4LIDAR_handle->scan_data.message_quantity = 0;

    for (uint16_t i = X4LIDAR_handle->scan_data.start_idx;
         i < X4LIDAR_handle->scan_data.end_idx - 1; i++)
    {
        if (X4LIDAR_handle->scan_data.dma_buffer[i] == SCAN_CONTENT_HEADER_PH_1_VALUE &&
            X4LIDAR_handle->scan_data.dma_buffer[i + 1] == SCAN_CONTENT_HEADER_PH_2_VALUE)
        {
            // Check if we overrun the index buffer
            if (X4LIDAR_handle->scan_data.message_quantity >= SCAN_CONTENT_DATA_START_IDX_BUFFER_SIZE)
            {
                return HAL_ERROR;
            }

            // Store the start index of the current data frame
            X4LIDAR_handle->scan_data.data_frame_start_idx_buffer[X4LIDAR_handle->scan_data.message_quantity] = i;
            X4LIDAR_handle->scan_data.message_quantity++;
        }
    }

    return HAL_OK;
}

/**
 * @brief Parses the header of the current data frame in the DMA buffer.
 *
 * This function extracts the header information (e.g., packet type, start angle, end angle)
 * for the current data frame stored in the DMA buffer.
 *
 * @param X4LIDAR_handle Pointer to the LiDAR handle structure.
 *
 * @return HAL_OK if the header is successfully parsed, else HAL_ERROR.
 */
HAL_StatusTypeDef X4LIDAR_parse_data_frame_header(X4LIDAR_handle_t *X4LIDAR_handle)
{
    uint16_t header_index = X4LIDAR_handle->scan_data.current_data_frame_start_idx;

    // Ensure the header index stays within the bounds of the DMA buffer
    if (header_index + SCAN_CONTENT_HEADER_SIZE > X4LIDAR_handle->scan_data.end_idx)
    {
        return HAL_ERROR;
    }

    // Parse the header fields
    X4LIDAR_handle->scan_data.scan_header.packet_header =
        (X4LIDAR_handle->scan_data.dma_buffer[header_index + SCAN_CONTENT_HEADER_PH_2_INDEX] << 8) |
        X4LIDAR_handle->scan_data.dma_buffer[header_index + SCAN_CONTENT_HEADER_PH_1_INDEX];

    X4LIDAR_handle->scan_data.scan_header.packet_type =
        X4LIDAR_handle->scan_data.dma_buffer[header_index + SCAN_CONTENT_HEADER_CT_INDEX];

    X4LIDAR_handle->scan_data.scan_header.sample_quantity =
        X4LIDAR_handle->scan_data.dma_buffer[header_index + SCAN_CONTENT_HEADER_LSN_INDEX];

    X4LIDAR_handle->scan_data.scan_header.start_angle =
        (X4LIDAR_handle->scan_data.dma_buffer[header_index + SCAN_CONTENT_HEADER_FSA_2_INDEX] << 8) |
        X4LIDAR_handle->scan_data.dma_buffer[header_index + SCAN_CONTENT_HEADER_FSA_1_INDEX];

    X4LIDAR_handle->scan_data.scan_header.end_angle =
        (X4LIDAR_handle->scan_data.dma_buffer[header_index + SCAN_CONTENT_HEADER_LSA_2_INDEX] << 8) |
        X4LIDAR_handle->scan_data.dma_buffer[header_index + SCAN_CONTENT_HEADER_LSA_1_INDEX];

    X4LIDAR_handle->scan_data.scan_header.check_code =
        (X4LIDAR_handle->scan_data.dma_buffer[header_index + SCAN_CONTENT_HEADER_CS_2_INDEX] << 8) |
        X4LIDAR_handle->scan_data.dma_buffer[header_index + SCAN_CONTENT_HEADER_CS_1_INDEX];

    return HAL_OK;
}
/**
 * @brief Calculates the maximum index for the current data frame.
 *
 * This function determines the end index of the current data frame based on the header information
 * and ensures valid memory access by staying within the bounds of the DMA buffer.
 *
 * @param X4LIDAR_handle Pointer to the LiDAR handle structure.
 *
 * @return HAL_OK on successful calculation.
 */
HAL_StatusTypeDef X4LIDAR_calculate_max_index(X4LIDAR_handle_t *X4LIDAR_handle)
{
    uint16_t data_end_index = X4LIDAR_handle->scan_data.current_data_frame_start_idx +
                              SCAN_CONTENT_HEADER_SIZE +
                              X4LIDAR_handle->scan_data.scan_header.sample_quantity * 2;

    // Ensure valid memory access within the DMA buffer
    if (data_end_index > X4LIDAR_handle->scan_data.end_idx)
    {
        X4LIDAR_handle->scan_data.current_data_frame_end_idx = X4LIDAR_handle->scan_data.end_idx;
    }
    else
    {
        X4LIDAR_handle->scan_data.current_data_frame_end_idx = data_end_index;
    }

    // Ensure end index aligns with complete data points (2 bytes per data point)
    if ((X4LIDAR_handle->scan_data.current_data_frame_end_idx % 2) ==
        (X4LIDAR_handle->scan_data.current_data_frame_start_idx + SCAN_CONTENT_HEADER_SIZE) % 2)
    {
        X4LIDAR_handle->scan_data.current_data_frame_end_idx--;
    }

    return HAL_OK;
}

/**
 * @brief Computes the payload (distances and angles) for the current data frame.
 *
 * This function calculates the distance and corrected angle for each data point in the frame.
 * It handles edge cases such as zero distance and ensures angles remain within valid bounds.
 *
 * @param X4LIDAR_handle Pointer to the LiDAR handle structure.
 *
 * @return HAL_OK on successful computation, else HAL_ERROR.
 */
HAL_StatusTypeDef X4LIDAR_compute_payload(X4LIDAR_handle_t *X4LIDAR_handle)
{
    float start_angle = ((float)(X4LIDAR_handle->scan_data.scan_header.start_angle >> 1)) / 64;
    float end_angle = ((float)(X4LIDAR_handle->scan_data.scan_header.end_angle >> 1)) / 64;

    if (X4LIDAR_handle->scan_data.scan_header.sample_quantity < 2)
    {
        return HAL_ERROR;
    }

    // Calculate angle increment per sample
    float diff_angle = end_angle - start_angle;
    if (diff_angle < 0) // Handle case for one full turn
    {
        diff_angle = (diff_angle + 360.0) / (X4LIDAR_handle->scan_data.scan_header.sample_quantity - 1);
    }
    else
    {
        diff_angle /= X4LIDAR_handle->scan_data.scan_header.sample_quantity;
    }

    uint8_t data_point_number = 0;
    uint8_t iteration_counter = 0; // Tracks iterations

    // Compute distances and angles
    for (int idx = X4LIDAR_handle->scan_data.current_data_frame_start_idx + SCAN_CONTENT_HEADER_SIZE;
         idx < X4LIDAR_handle->scan_data.current_data_frame_end_idx;
         idx += 2)
    {
        // Extract raw distance
        uint16_t distance_raw = X4LIDAR_handle->scan_data.dma_buffer[idx];
        distance_raw |= X4LIDAR_handle->scan_data.dma_buffer[idx + 1] << 8;

        float distance = ((float)distance_raw) / 4;
        float angle = diff_angle * data_point_number + start_angle;

        // Correct angle if distance is valid
        float angle_correct = 0;
        if (distance != 0)
        {
            angle_correct = atan(CONSTANT_1 * ((CONSTANT_2 - distance) / (CONSTANT_2 * distance)));
        }
        angle += angle_correct;

        // Normalize angle to stay within bounds
        if ((uint32_t)angle > MAX_ANGLE)
        {
            angle -= MAX_ANGLE;

#ifdef PRINT_DEBUG
            printf("Invalid angle detected, corrected.\r\n");
#endif
        }

        uint32_t rounded_angle = (uint32_t)(angle + 0.5f); // Round angle
        X4LIDAR_handle->scan_data.distances[rounded_angle] = distance;

        // Increment data point number every two iterations
        if (iteration_counter % 2 == 1)
        {
            data_point_number++;
        }
        iteration_counter++; // Increment the iteration counter
    }

    return HAL_OK;
}

/**
 * @brief Parses the DMA buffer to extract and process data frames.
 *
 * This function iterates through all identified data frames in the buffer and processes them
 * by parsing their headers, calculating their maximum indexes, and computing their payloads.
 *
 * @param X4LIDAR_handle Pointer to the LiDAR handle structure.
 *
 * @return HAL_OK if all frames are successfully parsed, else HAL_ERROR.
 */
HAL_StatusTypeDef X4LIDAR_parse_buffer(X4LIDAR_handle_t *X4LIDAR_handle)
{
    for (int data_frame_idx = 0; data_frame_idx < X4LIDAR_handle->scan_data.message_quantity; data_frame_idx++)
    {
        X4LIDAR_handle->scan_data.current_data_frame_start_idx =
            X4LIDAR_handle->scan_data.data_frame_start_idx_buffer[data_frame_idx];

        // Parse header, calculate max index, and compute payload for the current frame
        if ((X4LIDAR_parse_data_frame_header(X4LIDAR_handle) != HAL_OK) ||
            (X4LIDAR_calculate_max_index(X4LIDAR_handle) != HAL_OK) ||
            (X4LIDAR_compute_payload(X4LIDAR_handle) != HAL_OK))
        {
            return HAL_ERROR;
        }
    }

    return HAL_OK;
}

/**
 * @brief Task to handle LiDAR operations.
 *
 * This task initializes the LiDAR, starts the scanning process, and continuously processes
 * received data from the DMA buffer. Errors are logged (if debug is enabled) and skipped for
 * the current loop iteration.
 *
 * @param argument Pointer to the X4LIDAR_handle_t structure.
 */
void X4LIDAR_task(void *argument)
{
    // Retrieve the LiDAR handle passed as argument
    X4LIDAR_handle_t *X4LIDAR_handle = (X4LIDAR_handle_t*) argument;

    // Initialize the LiDAR hardware
    X4LIDAR_init(X4LIDAR_handle, &huart3);
    X4LIDAR_start_scan(X4LIDAR_handle);

    for (;;)
    {
        // Wait for notification (with timeout of 100 ms)
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100));

        // Process the received data
        if (X4LIDAR_get_data_start_indexes(X4LIDAR_handle) != HAL_OK)
        {
#ifdef PRINT_DEBUG
            printf("LiDAR error: Failed to retrieve scan data indexes.\r\n");
#endif
            continue;  // Skip to the next iteration
        }

#ifdef PRINT_DEBUG
        // Uncomment to monitor stack usage (optional)
        // UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        // printf("Remaining stack space: %u bytes\r\n", uxHighWaterMark);
#endif

        // Parse the buffer and compute the results
        if (X4LIDAR_parse_buffer(X4LIDAR_handle) != HAL_OK)
        {
#ifdef PRINT_DEBUG
            printf("LiDAR error: Failed to parse buffer.\r\n");
#endif
            continue;  // Skip to the next iteration
        }

        // Delay to throttle task execution
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

/**
 * @brief UART DMA Half Transfer Complete Callback.
 *
 * This callback is invoked when the DMA transfer reaches halfway.
 * It processes the first half of the DMA buffer and notifies the task for handling.
 *
 * @param huart Pointer to the UART handle.
 * @param X4LIDAR_handle Pointer to the LiDAR handle.
 */
void X4LIDAR_HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart, X4LIDAR_handle_t *X4LIDAR_handle)
{
    if (huart->Instance == X4LIDAR_handle->huart->Instance)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        // Set buffer bounds for the first half
        X4LIDAR_handle->scan_data.start_idx = 0;
        X4LIDAR_handle->scan_data.end_idx = SCAN_CONTENT_DMA_BUFFER_SIZE / 2;

        // Notify the LiDAR task from ISR
        vTaskNotifyGiveFromISR(X4LIDAR_handle->task_handle, &xHigherPriorityTaskWoken);

        // Perform context switch if a higher-priority task was woken
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/**
 * @brief UART DMA Transfer Complete Callback.
 *
 * This callback is invoked when the DMA transfer is fully complete.
 * It processes the second half of the DMA buffer and notifies the task for handling.
 *
 * @param huart Pointer to the UART handle.
 * @param X4LIDAR_handle Pointer to the LiDAR handle.
 */
void X4LIDAR_HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart, X4LIDAR_handle_t *X4LIDAR_handle)
{
    if (huart->Instance == X4LIDAR_handle->huart->Instance)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        // Set buffer bounds for the second half
        X4LIDAR_handle->scan_data.start_idx = SCAN_CONTENT_DMA_BUFFER_SIZE / 2;
        X4LIDAR_handle->scan_data.end_idx = SCAN_CONTENT_DMA_BUFFER_SIZE;

        // Notify the LiDAR task from ISR
        vTaskNotifyGiveFromISR(X4LIDAR_handle->task_handle, &xHigherPriorityTaskWoken);

        // Perform context switch if a higher-priority task was woken
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

