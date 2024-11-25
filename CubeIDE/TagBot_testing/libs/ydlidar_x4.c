/*
 * ydlidar_x4.c
 *
 *  Created on: Nov 19, 2024
 *      Author: exysta
 */


/*
 * ydlidar_x4.c
 *
 *  Created on: Feb 14, 2024
 *      Author: nicolas
 */
#include "ydlidar_x4.h"
#include <stdlib.h>
#include "cmsis_os.h"


#define ANGLE_MIN 0
#define ANGLE_MAX 360
#define DISTANCE_MIN 200
//#define SERIAL_DEBUG


StaticTask_t UART_Processing_Task_Buffer;
StaticTask_t LiDAR_Processing_Task_Buffer;

StackType_t UART_Processing_Stack_Buffer[UART_STACK_SIZE];
StackType_t LiDAR_Processing_Stack_Buffer[LIDAR_PROCESSING_STACK_SIZE];

HAL_StatusTypeDef YDLIDAR_X4_Send_Cmd(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle, uint8_t cmd){
	uint8_t cmdBuffer[2] = {CMD_PREFIX, cmd};
	if(HAL_UART_Transmit(YDLIDAR_X4_Handle->huart, cmdBuffer, 2, 10) == HAL_OK){
		return HAL_OK;
	}
	else{
		return HAL_ERROR;
	}
}

HAL_StatusTypeDef YDLIDAR_X4_Task_Create_UART(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle)
{
	YDLIDAR_X4_Handle->UART_Processing_Task_Handle = xTaskCreateStatic(YDLIDAR_X4_UART_Processing_Task, "UART_Task", UART_STACK_SIZE,  (void *)YDLIDAR_X4_Handle, UART_TASK_PRIORITY, UART_Processing_Stack_Buffer, &UART_Processing_Task_Buffer);

	return HAL_OK;
}
HAL_StatusTypeDef YDLIDAR_X4_Task_Create_LidarProcessing(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle)
{
	YDLIDAR_X4_Handle->LiDAR_Processing_Task_Handle = xTaskCreateStatic(YDLIDAR_X4_LiDAR_Processing_Task, "LiDAR_Processing_Task", LIDAR_PROCESSING_STACK_SIZE,  (void *)YDLIDAR_X4_Handle, LIDAR_PROCESSING_TASK_PRIORITY, LiDAR_Processing_Stack_Buffer, &LiDAR_Processing_Task_Buffer);
	return HAL_OK;
}

HAL_StatusTypeDef YDLIDAR_X4_Task_Create(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle)
{
	YDLIDAR_X4_Task_Create_UART(YDLIDAR_X4_Handle);
	YDLIDAR_X4_Task_Create_LidarProcessing(YDLIDAR_X4_Handle);
	return HAL_OK;
}


HAL_StatusTypeDef YDLIDAR_X4_Init(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle, UART_HandleTypeDef *huart){
	YDLIDAR_X4_Handle->huart = huart;
	YDLIDAR_X4_Handle->state = IDLE;
	YDLIDAR_X4_Handle->trame_id = 0;
	YDLIDAR_X4_Handle->newData = 0;
	YDLIDAR_X4_Handle->scan_response.id_data = 0;

	YDLIDAR_X4_Send_Cmd(YDLIDAR_X4_Handle, CMD_SOFT_RESTART);
	HAL_Delay(20);

	//YDLIDAR_X4_Get_Device_Informations(YDLIDAR_X4_Handle);
	//	YDLIDAR_X4_Print_Device_Informations(YDLIDAR_X4_Handle);

	//YDLIDAR_X4_Get_Health_Status(YDLIDAR_X4_Handle);
	//	YDLIDAR_X4_Print_Health_Status(YDLIDAR_X4_Handle);
	HAL_Delay(200);

	//	HAL_GPIO_WritePin(OSC_TRIG_GPIO_Port, OSC_TRIG_Pin, RESET);
	YDLIDAR_X4_Start_Scan(YDLIDAR_X4_Handle);
	YDLIDAR_X4_Task_Create(YDLIDAR_X4_Handle);
	return HAL_OK;
}



HAL_StatusTypeDef YDLIDAR_X4_Get_Device_Informations(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle){
	YDLIDAR_header_response header;

	//	txBufferSize = snprintf((char *)uart2TxBuffer, UART_TX_BUFFER_SIZE, "Getting Device Informations...\r\n");
	//HAL_UART_Transmit(&huart2, uart2TxBuffer, txBufferSize, LAST_DATA_BUFFER_SIZE);

	YDLIDAR_X4_Send_Cmd(YDLIDAR_X4_Handle, CMD_STOP_SCAN);
	HAL_Delay(50);


	YDLIDAR_X4_Send_Cmd(YDLIDAR_X4_Handle, CMD_GET_DEVICE_INFO);

	HAL_UART_Abort(YDLIDAR_X4_Handle->huart); // Clean every old data in Rx Buffer

	uint8_t rx_buffer[HEADER_SIZE + DEVICE_INFORMATION_PAYLOAD_SIZE];

	if(HAL_UART_Receive_DMA(YDLIDAR_X4_Handle->huart, rx_buffer, HEADER_SIZE+DEVICE_INFORMATION_PAYLOAD_SIZE) != HAL_OK){
		//		txBufferSize = snprintf((char *)uart2TxBuffer, UART_TX_BUFFER_SIZE, "Device not responding...\r\n");
		//HAL_UART_Transmit(&huart2, uart2TxBuffer, txBufferSize, 10);
		return HAL_ERROR;
	}

	// Get parameters from response
	header.start_sign[0] 	= rx_buffer[0];
	header.start_sign[1] 	= rx_buffer[1];
	header.content_size 	= (rx_buffer[5] & 0x3F) << 24 |
			rx_buffer[4] << 16 |
			rx_buffer[3] << 8 |
			rx_buffer[2];
	header.mode 			= rx_buffer[5] >> 6;
	header.type_code 		= rx_buffer[6];

	if(header.start_sign[0] = 0xA5 &&
			header.start_sign[1] == 0x5A &&
			header.content_size == 0x14 &&
			header.mode == 0x00 &&
			header.type_code == 0x04)
	{
		YDLIDAR_X4_Handle->model = rx_buffer[HEADER_SIZE];
		YDLIDAR_X4_Handle->firmware_minor = rx_buffer[HEADER_SIZE+1];
		YDLIDAR_X4_Handle->firmware_major = rx_buffer[HEADER_SIZE+2];
		YDLIDAR_X4_Handle->hardware = rx_buffer[HEADER_SIZE+3];
		for(int idx=0; idx<16; idx++){
			YDLIDAR_X4_Handle->serial_number[idx] = rx_buffer[HEADER_SIZE+4+idx];
		}

	}
	else{
		//		txBufferSize = snprintf((char *)uart2TxBuffer, UART_TX_BUFFER_SIZE, "Wrong header response...\r\n");
		//HAL_UART_Transmit(&huart2, uart2TxBuffer, txBufferSize, 10);
		return HAL_ERROR;
	}

	//	txBufferSize = snprintf((char *)uart2TxBuffer, UART_TX_BUFFER_SIZE, "Getting Device Informations : Done !\r\n");
	//HAL_UART_Transmit(&huart2, uart2TxBuffer, txBufferSize, 10);

	return HAL_OK;
}
//HAL_StatusTypeDef YDLIDAR_X4_Print_Device_Informations(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle){
//	txBufferSize = snprintf((char *)uart2TxBuffer, UART_TX_BUFFER_SIZE,
//			"**********************************\r\n"
//			"*      Device Informations       *\r\n"
//			"**********************************\r\n"
//			"Model : 0x%02x\r\n"
//			"Firmware : %u.%u\r\n"
//			"Hardware : %u\r\n"
//			"Serial Number : ",
//			YDLIDAR_X4_Handle->model,
//			(unsigned int)YDLIDAR_X4_Handle->firmware_major,
//			(unsigned int)YDLIDAR_X4_Handle->firmware_minor,
//			(unsigned int)YDLIDAR_X4_Handle->hardware);
//
//	for(int idx = 0; idx<16; idx++){
//		uart2TxBuffer[txBufferSize+idx] = YDLIDAR_X4_Handle->serial_number[idx] + 48;
//	}
//	uart2TxBuffer[txBufferSize+16]='\r';
//	uart2TxBuffer[txBufferSize+17]='\n';
//	txBufferSize+=18;
//	HAL_UART_Transmit(&huart2, uart2TxBuffer, txBufferSize, 10);
//}
HAL_StatusTypeDef YDLIDAR_X4_Get_Health_Status(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle){
	YDLIDAR_header_response header;

	//	txBufferSize = snprintf((char *)uart2TxBuffer, UART_TX_BUFFER_SIZE, "Getting Health Status...\r\n");
	//HAL_UART_Transmit(&huart2, uart2TxBuffer, txBufferSize, 10);

	YDLIDAR_X4_Send_Cmd(YDLIDAR_X4_Handle, CMD_STOP_SCAN);
	HAL_Delay(50);

	YDLIDAR_X4_Send_Cmd(YDLIDAR_X4_Handle, CMD_GET_HEALTH_STATUS);

	HAL_UART_Abort(YDLIDAR_X4_Handle->huart); // Clean every old data in Rx Buffer
	uint8_t rx_buffer[HEADER_SIZE + HEALTH_STATUS_PAYLOAD_SIZE];

	if(HAL_UART_Receive(YDLIDAR_X4_Handle->huart, rx_buffer, HEADER_SIZE+HEALTH_STATUS_PAYLOAD_SIZE, 5000) != HAL_OK){
		//		txBufferSize = snprintf((char *)uart2TxBuffer, UART_TX_BUFFER_SIZE, "Device not responding...\r\n");
		//HAL_UART_Transmit(&huart2, uart2TxBuffer, txBufferSize, 10);
		return HAL_ERROR;
	}

	// Get parameters from response
	header.start_sign[0] 	= rx_buffer[0];
	header.start_sign[1] 	= rx_buffer[1];
	header.content_size 	= (rx_buffer[5] & 0x3F) << 24 |
			rx_buffer[4] << 16 |
			rx_buffer[3] << 8 |
			rx_buffer[2];
	header.mode 			= rx_buffer[5] >> 6;
	header.type_code 		= rx_buffer[6];

	if(header.start_sign[0] = 0xA5 &&
			header.start_sign[1] == 0x5A &&
			header.content_size == 0x03 &&
			header.mode == 0x00 &&
			header.type_code == 0x06){
		YDLIDAR_X4_Handle->health_status = rx_buffer[HEADER_SIZE];
		YDLIDAR_X4_Handle->health_error_code[0] = rx_buffer[HEADER_SIZE+1];
		YDLIDAR_X4_Handle->health_error_code[1] = rx_buffer[HEADER_SIZE+2];

	}
	else{
		//		txBufferSize = snprintf((char *)uart2TxBuffer, UART_TX_BUFFER_SIZE, "Wrong header response...\r\n");
		//HAL_UART_Transmit(&huart2, uart2TxBuffer, txBufferSize, 10);
		return HAL_ERROR;
	}

	//	txBufferSize = snprintf((char *)uart2TxBuffer, UART_TX_BUFFER_SIZE, "Health Status : Done !\r\n");
	//HAL_UART_Transmit(&huart2, uart2TxBuffer, txBufferSize, 10);

	return HAL_OK;
}
//HAL_StatusTypeDef YDLIDAR_X4_Print_Health_Status(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle){
////	txBufferSize = snprintf((char *)uart2TxBuffer, UART_TX_BUFFER_SIZE,
////			"**********************************\r\n"
////			"*      Health Status             *\r\n"
////			"**********************************\r\n"
////			"Status code : 0x%02x\r\n"
////			"Error code : ",
////			YDLIDAR_X4_Handle->health_status);
//
//	for(int idx = 0; idx<2; idx++){
////		uart2TxBuffer[txBufferSize+idx] = YDLIDAR_X4_Handle->health_error_code[idx] + 48;
//	}
//	uart2TxBuffer[txBufferSize+2]='\r';
//	uart2TxBuffer[txBufferSize+3]='\n';
//	txBufferSize+=4;
//	HAL_UART_Transmit(&huart2, uart2TxBuffer, txBufferSize, 10);
//}
HAL_StatusTypeDef YDLIDAR_X4_Start_Scan(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle)
{
	YDLIDAR_X4_Send_Cmd(YDLIDAR_X4_Handle, CMD_START_SCAN);
	YDLIDAR_X4_Handle->state = START_SYNC_CONTENT_HEADER;  //we notify the state machine that we are processing the header

	HAL_UART_Abort(YDLIDAR_X4_Handle->huart);
	HAL_UART_Receive_IT(YDLIDAR_X4_Handle->huart, YDLIDAR_X4_Handle->header_buffer, HEADER_SIZE);

	return HAL_OK;
}

HAL_StatusTypeDef YDLIDAR_X4_Soft_Reboot(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle)
{
	return YDLIDAR_X4_Send_Cmd(YDLIDAR_X4_Handle, CMD_SOFT_RESTART);
}

HAL_StatusTypeDef YDLIDAR_X4_Compute_Payload(volatile __YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle){
	// 50us to compute this function approx, 4000 cyles.


	float start_angle = ((float)(YDLIDAR_X4_Handle->scan_response.start_angle >> 1))/64;
	float end_angle = ((float)(YDLIDAR_X4_Handle->scan_response.end_angle >> 1))/64;
	float diff_angle = end_angle - start_angle;
	float angle, distance;
	uint16_t distance_raw;
	if(diff_angle<0)// Check not negative (one turn)
	{
		diff_angle = (diff_angle+360)/(YDLIDAR_X4_Handle->scan_response.sample_quantity-1);
	}
	else
	{
		diff_angle /= YDLIDAR_X4_Handle->scan_response.sample_quantity;
	}

	// Compute distance
	//HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, SET);
	for(int idx=0; idx<YDLIDAR_X4_Handle->scan_response.sample_quantity; idx++)
	{
		distance_raw = YDLIDAR_X4_Handle->scan_response.scan_content_buffer_raw_distances[2*idx];
		distance_raw |= YDLIDAR_X4_Handle->scan_response.scan_content_buffer_raw_distances[2*idx+1] << 8;
		distance = ((float)distance_raw)/4;
		angle = diff_angle*idx+start_angle;
		YDLIDAR_X4_Handle->scan_response.distance[(uint32_t)angle]=distance;
	}

	//	int idx_angle_min_distance = -1;
	//	float min_distance = 10000;
	//	for(int idx_angle=120; idx_angle<240; idx_angle++){
	//		if((10 < YDLIDAR_X4_Handle->scan_response.distance[idx_angle]) &&
	//				(YDLIDAR_X4_Handle->scan_response.distance[idx_angle] < min_distance)){
	//			idx_angle_min_distance = idx_angle;
	//			min_distance = YDLIDAR_X4_Handle->scan_response.distance[idx_angle];
	//		}
	//	}
	//	if(min_distance < 200){
	//		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, SET);
	//	}
	//	else{
	//		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, RESET);
	//	}
	//HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, RESET);
	//txBufferSize = snprintf((char *)uart2TxBuffer, UART_TX_BUFFER_SIZE, "Angle %3d , Distance : %4.3f mm\r\n", idx_angle_min_distance, min_distance);
	//HAL_UART_Transmit(&huart2, uart2TxBuffer, txBufferSize, 10);



	return HAL_OK;
}



HAL_StatusTypeDef YDLIDAR_X4_Process_Scan_Data(volatile  __YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle,uint16_t start_index)
{
	YDLIDAR_X4_Handle->scan_response.package_type =
			YDLIDAR_X4_Handle->scan_response.scan_content_buffer_dma[start_index + SCAN_CONTENT_HEADER_CT_INDEX];

	YDLIDAR_X4_Handle->scan_response.sample_quantity = YDLIDAR_X4_Handle->scan_response.scan_content_buffer_dma[start_index + SCAN_CONTENT_HEADER_LSN_INDEX];

	YDLIDAR_X4_Handle->scan_response.start_angle =
			(YDLIDAR_X4_Handle->scan_response.scan_content_buffer_dma[start_index + SCAN_CONTENT_HEADER_FSA_2_INDEX] << 8) |
			(YDLIDAR_X4_Handle->scan_response.scan_content_buffer_dma[start_index + SCAN_CONTENT_HEADER_FSA_1_INDEX]);

	YDLIDAR_X4_Handle->scan_response.end_angle =
			(YDLIDAR_X4_Handle->scan_response.scan_content_buffer_dma[start_index + SCAN_CONTENT_HEADER_LSA_2_INDEX] << 8) |
			(YDLIDAR_X4_Handle->scan_response.scan_content_buffer_dma[start_index + SCAN_CONTENT_HEADER_LSA_1_INDEX]);

	YDLIDAR_X4_Handle->scan_response.check_code =
			(YDLIDAR_X4_Handle->scan_response.scan_content_buffer_dma[start_index + SCAN_CONTENT_HEADER_CS_2_INDEX] << 8) |
			(YDLIDAR_X4_Handle->scan_response.scan_content_buffer_dma[start_index + SCAN_CONTENT_HEADER_CS_1_INDEX]);

	int buffer_index;
	for(int idx = 0; idx < YDLIDAR_X4_Handle->scan_response.sample_quantity * 2; idx++)
	{
		if(start_index + SCAN_CONTENT_HEADER_SIZE + idx > SCAN_CONTENT_DMA_BUFFER_SIZE-1 )
		{
			buffer_index = start_index + SCAN_CONTENT_HEADER_SIZE + idx - SCAN_CONTENT_DMA_BUFFER_SIZE;
		}
		else
		{
			buffer_index = start_index + SCAN_CONTENT_HEADER_SIZE + idx;
		}
		YDLIDAR_X4_Handle->scan_response.scan_content_buffer_raw_distances[idx] = YDLIDAR_X4_Handle->scan_response.scan_content_buffer_dma [buffer_index];
	}

	if (YDLIDAR_X4_Handle->scan_response.package_type == SCAN_CONTENT_CT_DATA_PACKET)
	{
		YDLIDAR_X4_Handle->trame_id++;
		YDLIDAR_X4_Handle->newData = 1;
		YDLIDAR_X4_Compute_Payload(YDLIDAR_X4_Handle);
	}

	return HAL_OK;

}

HAL_StatusTypeDef YDLIDAR_X4_Parse_Buffer(volatile __YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle)
{
	uint16_t start_idx, end_idx;

	// Determine the DMA state and set the corresponding buffer range
	if (YDLIDAR_X4_Handle->scan_response.dma_state == START_SCAN_DATA_HALF_CPLT)
	{
		start_idx = 0;
		end_idx = (SCAN_CONTENT_DMA_BUFFER_SIZE / 2) - 1;
	}
	else if (YDLIDAR_X4_Handle->scan_response.dma_state == START_SCAN_DATA_FULL_CPLT)
	{
		start_idx = SCAN_CONTENT_DMA_BUFFER_SIZE / 2;
		end_idx = SCAN_CONTENT_DMA_BUFFER_SIZE - 1;
	}
	else
	{
		return HAL_ERROR; // Invalid DMA state
	}

	// Iterate over the specified half of the buffer
	for (uint16_t i = start_idx; i < end_idx - 1; i++) // Ensure there's room to check i+1
	{
		// Check for header pattern
		if ((YDLIDAR_X4_Handle->scan_response.scan_content_buffer_dma[i] == SCAN_CONTENT_HEADER_PH_1_VALUE) &&
				(YDLIDAR_X4_Handle->scan_response.scan_content_buffer_dma[i + 1] == SCAN_CONTENT_HEADER_PH_2_VALUE))
		{
			// Process header and associated data
			HAL_StatusTypeDef status = YDLIDAR_X4_Process_Scan_Data(YDLIDAR_X4_Handle, i);
			if (status != HAL_OK)
			{
				return status; // Return error if processing fails
			}

			// Skip past header to avoid redundant checks (increment by 2 to skip both header bytes)
			i += 2; // Skip both header bytes
		}
	}

	return HAL_OK;
}



HAL_StatusTypeDef YDLIDAR_X4_State_Machine(volatile  __YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle)
{

	switch(YDLIDAR_X4_Handle->state)
	{
	case IDLE:
		YDLIDAR_X4_Handle->state = IDLE;
		break;
	case STOP:
		YDLIDAR_X4_Handle->state = STOP;
		break;

		/************************************************************************
		 * START_WAIT_CONTENT_HEADER											*
		 * After sync, get header of content part								*
		 * PH 	2 bytes		packet header										*
		 * CT 	1 byte 		packet type (bit(0)=point cloud data packet			*
		 * LSN  1 byte		Sample quantity in hald-word, 1 point = uint16_t	*
		 * FSA 	2 bytes		Start angle											*
		 * LSA  2 bytes		End angle											*
		 ************************************************************************/
	case START_SYNC_CONTENT_HEADER:

		//
		if(YDLIDAR_X4_Handle->header_buffer[X4_REPLY_TYPE_CODE_INDEX] == SCAN_COMMAND_REPLY_TYPE_CODE ) // we check if the message is the start packet representing the beginning of a cycle of data
		{
			YDLIDAR_X4_Handle->state = START_SCANNING;
			YDLIDAR_X4_Handle->scan_response.dma_state = START_SCAN_DATA_FULL_CPLT; // le prochain interrupt sera celui du half complete

			HAL_UART_Receive_DMA(YDLIDAR_X4_Handle->huart,  (uint8_t *)YDLIDAR_X4_Handle->scan_response.scan_content_buffer_dma, SCAN_CONTENT_DMA_BUFFER_SIZE);
			break;
		}
		return HAL_ERROR;

	case START_SCANNING:
		YDLIDAR_X4_Handle->scan_response.dma_state = (YDLIDAR_X4_Handle->scan_response.dma_state == START_SCAN_DATA_HALF_CPLT)
		? START_SCAN_DATA_FULL_CPLT
				: START_SCAN_DATA_HALF_CPLT;
		YDLIDAR_X4_Parse_Buffer(YDLIDAR_X4_Handle);
		break;



	default:
		YDLIDAR_X4_Handle->state = STOP;

	}
	return HAL_OK;
}


void YDLIDAR_X4_UART_Processing_Task(void *argument)
{

	// Retrieve the handle (hlidar) passed as argument
	volatile __YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle =
			(volatile __YDLIDAR_X4_HandleTypeDef *) argument;
	for (;;)
	{

		// Wait for the DMA completion notification
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // Blocks indefinitely until notified

		// Process the received data
		YDLIDAR_X4_State_Machine(YDLIDAR_X4_Handle);

		// Delay (optional)
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

void YDLIDAR_X4_LiDAR_Processing_Task(void *argument)
{
	float min_distance = 10000;
	int idx_angle_min_distance;
	uint8_t object_detected = 1;

	// Retrieve the handle (hlidar) passed as argument
	volatile __YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle =
			(volatile __YDLIDAR_X4_HandleTypeDef *) argument;
	for (;;)
	{
		if(YDLIDAR_X4_Handle->newData)
		{
			min_distance = 10000;
			for(int idx_angle=ANGLE_MIN; idx_angle<ANGLE_MAX; idx_angle++){
				if((10 < YDLIDAR_X4_Handle->scan_response.distance[idx_angle]) &&
						(YDLIDAR_X4_Handle->scan_response.distance[idx_angle] < min_distance)){
					idx_angle_min_distance = idx_angle;
					min_distance =YDLIDAR_X4_Handle->scan_response.distance[idx_angle];
				}
			}
			if(min_distance < DISTANCE_MIN){
				object_detected = 1;

			}
			else{
				object_detected = 0;
			}

			YDLIDAR_X4_Handle->newData = 0;
		}

		// Optional delay
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

void YDLIDAR_X4_HAL_UART_ErrorCallback(UART_HandleTypeDef *huart,volatile __YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle)
{
	if (huart->Instance == YDLIDAR_X4_Handle->huart->Instance)
	{
		uint32_t error = HAL_UART_GetError(huart);
		volatile int a = 5;
		// Log or handle the error
	}
}

// to be called inside the callback function in the main.
//
void YDLIDAR_X4_HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart,volatile __YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle)
{
	if (huart->Instance == YDLIDAR_X4_Handle->huart->Instance)
	{
		if(YDLIDAR_X4_Handle->state == START_SCANNING)
		{
			BaseType_t xHigherPriorityTaskWoken = pdFALSE;

			vTaskNotifyGiveFromISR(YDLIDAR_X4_Handle->UART_Processing_Task_Handle, &xHigherPriorityTaskWoken);

			// Perform context switch if required
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
	}
}



void YDLIDAR_X4_HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart,volatile __YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle)
{
	if (huart->Instance == YDLIDAR_X4_Handle->huart->Instance)
	{

		if(YDLIDAR_X4_Handle->state == START_SCANNING || YDLIDAR_X4_Handle->state ==START_SYNC_CONTENT_HEADER)
		{
			BaseType_t xHigherPriorityTaskWoken = pdFALSE;

			vTaskNotifyGiveFromISR(YDLIDAR_X4_Handle->UART_Processing_Task_Handle, &xHigherPriorityTaskWoken);

			// Perform context switch if required
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}

	}
}
