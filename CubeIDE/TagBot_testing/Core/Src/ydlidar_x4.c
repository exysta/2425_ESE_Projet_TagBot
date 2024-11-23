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

#define ANGLE_MIN 120
#define ANGLE_MAX 240
#define DISTANCE_MIN 200
//#define SERIAL_DEBUG
__YDLIDAR_X4_HandleTypeDef hlidar;
TaskHandle_t UART_Processing_TaskHandle;
TaskHandle_t LiDAR_Processing_TaskHandle;

volatile uint8_t uart_lidar_processing = 0;

//variable to be used in stat machine



HAL_StatusTypeDef YDLIDAR_X4_Send_Cmd(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle, uint8_t cmd){
	uint8_t cmdBuffer[2] = {CMD_PREFIX, cmd};
	if(HAL_UART_Transmit(YDLIDAR_X4_Handle->huart, cmdBuffer, 2, 10) == HAL_OK){
		return HAL_OK;
	}
	else{
		return HAL_ERROR;
	}
}
HAL_StatusTypeDef YDLIDAR_X4_Init(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle, UART_HandleTypeDef *huart){
	YDLIDAR_X4_Handle->huart = huart;
	YDLIDAR_X4_Handle->state = IDLE;
	YDLIDAR_X4_Handle->trame_id = 0;
	YDLIDAR_X4_Handle->newData = 0;
	YDLIDAR_X4_Handle->scan_response.id_data = 0;
	YDLIDAR_X4_Handle->scan_response.scan_header_buffer_current = YDLIDAR_X4_Handle->scan_response.scan_header_buffer_1;
	YDLIDAR_X4_Handle->scan_response.scan_content_buffer_current = YDLIDAR_X4_Handle->scan_response.scan_content_buffer_1;
	YDLIDAR_X4_Handle->scan_response.scan_header_buffer_old = YDLIDAR_X4_Handle->scan_response.scan_header_buffer_2;
	YDLIDAR_X4_Handle->scan_response.scan_content_buffer_old = YDLIDAR_X4_Handle->scan_response.scan_content_buffer_2;
	YDLIDAR_X4_Send_Cmd(YDLIDAR_X4_Handle, CMD_SOFT_RESTART);
	HAL_Delay(20);

	//YDLIDAR_X4_Get_Device_Informations(YDLIDAR_X4_Handle);
	//	YDLIDAR_X4_Print_Device_Informations(YDLIDAR_X4_Handle);

	//YDLIDAR_X4_Get_Health_Status(YDLIDAR_X4_Handle);
	//	YDLIDAR_X4_Print_Health_Status(YDLIDAR_X4_Handle);
	HAL_Delay(200);

	//	HAL_GPIO_WritePin(OSC_TRIG_GPIO_Port, OSC_TRIG_Pin, RESET);
	YDLIDAR_X4_Start_Scan(YDLIDAR_X4_Handle);
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
HAL_StatusTypeDef YDLIDAR_X4_Start_Scan(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle){
	YDLIDAR_X4_Send_Cmd(YDLIDAR_X4_Handle, CMD_START_SCAN);
	YDLIDAR_X4_Handle->state = START_SYNC_CONTENT_HEADER;  //we notify the state machine that we are processing the header
	HAL_UART_Abort(YDLIDAR_X4_Handle->huart);
	HAL_UART_Receive_DMA(YDLIDAR_X4_Handle->huart, YDLIDAR_X4_Handle->header_buffer, HEADER_SIZE  );
	return HAL_OK;
}
HAL_StatusTypeDef YDLIDAR_X4_Soft_Reboot(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle){
	return YDLIDAR_X4_Send_Cmd(YDLIDAR_X4_Handle, CMD_SOFT_RESTART);
}
HAL_StatusTypeDef YDLIDAR_X4_Compute_Payload(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle){
	// 50us to compute this function approx, 4000 cyles.

	if(YDLIDAR_X4_Handle->scan_response.scan_header_buffer_old [0] == 170)
	{
		int a = YDLIDAR_X4_Handle->scan_response.scan_header_buffer_old [1];
	}
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
		distance_raw = YDLIDAR_X4_Handle->scan_response.scan_content_buffer_old[2*idx];
		distance_raw |= YDLIDAR_X4_Handle->scan_response.scan_content_buffer_old[2*idx+1]<<8;
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

HAL_StatusTypeDef YDLIDAR_X4_State_Machine(__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle)
{

	switch(YDLIDAR_X4_Handle->state)
	{
	case IDLE:
		YDLIDAR_X4_Handle->state = IDLE;
		break;
	case STOP:
		YDLIDAR_X4_Handle->state = STOP;
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
			YDLIDAR_X4_Handle->state = START_SCAN_DATA_HEADER;
			HAL_UART_Receive_DMA(YDLIDAR_X4_Handle->huart, YDLIDAR_X4_Handle->scan_response.scan_header_buffer_current, SCAN_CONTENT_HEADER_SIZE);
			break;
		}
		return HAL_ERROR;

	case START_SCAN_DATA_HEADER:

	    // Swap the old and current header buffers
	    uint8_t* temp_header = YDLIDAR_X4_Handle->scan_response.scan_header_buffer_current;
	    YDLIDAR_X4_Handle->scan_response.scan_header_buffer_current = YDLIDAR_X4_Handle->scan_response.scan_header_buffer_old;
	    YDLIDAR_X4_Handle->scan_response.scan_header_buffer_old = temp_header;




	    // Parse header data from the "old" buffer
	    YDLIDAR_X4_Handle->scan_response.package_type =
	        YDLIDAR_X4_Handle->scan_response.scan_header_buffer_old[SCAN_CONTENT_HEADER_CT];

	    YDLIDAR_X4_Handle->scan_response.sample_quantity = YDLIDAR_X4_Handle->scan_response.scan_header_buffer_old[SCAN_CONTENT_HEADER_LSN_INDEX];

	    YDLIDAR_X4_Handle->scan_response.start_angle =
	        (YDLIDAR_X4_Handle->scan_response.scan_header_buffer_old[SCAN_CONTENT_HEADER_FSA_2] << 8) |
	        (YDLIDAR_X4_Handle->scan_response.scan_header_buffer_old[SCAN_CONTENT_HEADER_FSA_1]);

	    YDLIDAR_X4_Handle->scan_response.end_angle =
	        (YDLIDAR_X4_Handle->scan_response.scan_header_buffer_old[SCAN_CONTENT_HEADER_LSA_2] << 8) |
	        (YDLIDAR_X4_Handle->scan_response.scan_header_buffer_old[SCAN_CONTENT_HEADER_LSA_1]);

	    YDLIDAR_X4_Handle->scan_response.check_code =
	        (YDLIDAR_X4_Handle->scan_response.scan_header_buffer_old[SCAN_CONTENT_HEADER_CS_2] << 8) |
	        (YDLIDAR_X4_Handle->scan_response.scan_header_buffer_old[SCAN_CONTENT_HEADER_CS_1]);


		YDLIDAR_X4_Handle->state = START_SCAN_DATA_CONTENT;


	     //Re-arm DMA for receiving content
	    HAL_UART_Receive_DMA(
	        YDLIDAR_X4_Handle->huart,
	        YDLIDAR_X4_Handle->scan_response.scan_content_buffer_current,
	        YDLIDAR_X4_Handle->scan_response.sample_quantity * 2 // 2 bytes per data
	    );

//	    HAL_UART_Receive_DMA(
//	        YDLIDAR_X4_Handle->huart,
//	        YDLIDAR_X4_Handle->scan_response.scan_content_buffer_current,
//	        100// 2 bytes per data
//	    );
	    break;

	case START_SCAN_DATA_CONTENT:

	    // Swap the old and current header buffers
	    uint8_t* temp_content = YDLIDAR_X4_Handle->scan_response.scan_content_buffer_current;
	    YDLIDAR_X4_Handle->scan_response.scan_content_buffer_current = YDLIDAR_X4_Handle->scan_response.scan_content_buffer_old;
	    YDLIDAR_X4_Handle->scan_response.scan_content_buffer_old = temp_content;



		YDLIDAR_X4_Handle->state=START_SCAN_DATA_HEADER;
		YDLIDAR_X4_Handle->trame_id++;
		YDLIDAR_X4_Handle->newData = 1;


		HAL_UART_Receive_DMA(YDLIDAR_X4_Handle->huart, YDLIDAR_X4_Handle->scan_response.scan_header_buffer_current, SCAN_CONTENT_HEADER_SIZE);
		break;

	default:
		YDLIDAR_X4_Handle->state = START_SCAN_DATA_HEADER;

		HAL_UART_Receive_DMA(YDLIDAR_X4_Handle->huart, YDLIDAR_X4_Handle->scan_response.scan_header_buffer_current, SCAN_CONTENT_HEADER_SIZE);
	}
	return HAL_OK;
}


void UART_Processing_Task(void *argument)
{

    // Retrieve the handle (hlidar) passed as argument
	__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle = (__YDLIDAR_X4_HandleTypeDef*) argument;

    for (;;)
    {

        // Wait for the DMA completion notification
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100)); // Blocks indefinitely until notified

        // Process the received data
        YDLIDAR_X4_State_Machine(YDLIDAR_X4_Handle);
        // Start UART DMA to receive data in the buffer


        // Delay (optional)
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void LiDAR_Processing_Task(void *argument)
{
	float min_distance = 10000;
	int idx_angle_min_distance;
	uint8_t object_detected = 1;

	__YDLIDAR_X4_HandleTypeDef *YDLIDAR_X4_Handle = (__YDLIDAR_X4_HandleTypeDef*) argument;
    for (;;)
    {
  	  if(hlidar.newData)
  	  {
  		YDLIDAR_X4_Compute_Payload(&hlidar);
  		min_distance = 10000;
  		for(int idx_angle=ANGLE_MIN; idx_angle<ANGLE_MAX; idx_angle++){
  			if((10 < hlidar.scan_response.distance[idx_angle]) &&
  					(hlidar.scan_response.distance[idx_angle] < min_distance)){
  				idx_angle_min_distance = idx_angle;
  				min_distance = hlidar.scan_response.distance[idx_angle];
  			}
  		}
  		if(min_distance < DISTANCE_MIN){
  			object_detected = 1;

  		}
  		else{
  			object_detected = 0;
  		}

  		hlidar.newData = 0;
  	}

        // Optional delay
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == huart3.Instance)
	{
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        vTaskNotifyGiveFromISR(UART_Processing_TaskHandle, &xHigherPriorityTaskWoken);

        // Perform context switch if required
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

	}
}
