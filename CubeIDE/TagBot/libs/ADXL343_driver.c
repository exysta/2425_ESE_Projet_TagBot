 /*
 * ADXL343_driver.c
 *
 *  Created on: Sep 16, 2024
 *      Author: marie et charlotte
 *  Code pour l'accéleromètre ADXL343
 */

#include <ADXL343_driver.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <spi.h>
#include <stddef.h>
#include "FreeRTOS.h"
#include "task.h"

volatile RobotState_t robot_state = ROBOT_STATE_CHAT;


// Function to read from a register
void ADXL343_ReadRegister(uint8_t reg, int8_t* rx_data, size_t length) {

	uint8_t tx_data = reg | 0x80; // MSB = 1 pour la lecture

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); 			// Activer NSS
	HAL_SPI_Transmit(&hspi1, &tx_data, 1, 200); 			// Envoyer l'adresse
	HAL_SPI_Receive(&hspi1, (uint8_t*)rx_data, 1, 200);  	// Lire la donnée
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);   			// Désactiver NSS

}

//Function to write in the register
void ADXL343_WriteRegister(uint8_t reg, uint8_t data) {
	uint8_t buffer[2];
	buffer[0] = reg;	//register
	buffer[1] = data;	//data

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); 			// enable NSS (PA4)
	HAL_SPI_Transmit(&hspi1, buffer, 2, 200); 			// Send data thanks to SPI
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET); 			// disable NSS (PA4)
}

//Function to initialize the accelerometer
int ADXL343_Init(ADXL343_Handle_t* handle) {
	uint8_t id = 0;

	// read the id of peripherique and check if is equal 0xE5
	// doesn't work if we switch uint to int
	ADXL343_ReadRegister(ADXL343_REG_DEVID, &id, 1);

	if (id != 0xE5) {
        handle->status = ADXL343_STATUS_NOT_DETECTED;
		printf("ADXL343 no detected ! ID: %02X\r\n", id);
		return 1;
	}

	/* Configuration of registers */

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	ADXL343_WriteRegister(ADXL343_REG_POWER_CTL, 0x04); 			// init the power control (sleep)
	ADXL343_WriteRegister(ADXL343_REG_DATA_FORMAT, 0x08);  			// establish format for data :full resolution and ±2g ->0x08, 16g ->0x0B
	ADXL343_WriteRegister(ADXL343_REG_BW_RATE, 0X0B);
	ADXL343_WriteRegister(ADXL343_REG_POWER_CTL, 0x08);  			// measurement mode of power control (active)

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET); 			// Desactiver NSS

    handle->status = ADXL343_STATUS_INITIALIZED;
	printf("ADXL343 Initialization done\r\n");
	return 0;
}

//Function to configure the accelerometer
void ADXL343_Configure(ADXL343_Handle_t* handle){
	ADXL343_WriteRegister(ADXL343_REG_THRESH_TAP, TAP_THRESHOLD);  	// Set tap threshold : 2g ou 16g
	ADXL343_WriteRegister(ADXL343_REG_DUR, TAP_DURATION);         	// Set tap duration : 10ms
	ADXL343_WriteRegister(ADXL343_REG_LATENT, TAP_LATENT);
	ADXL343_WriteRegister(ADXL343_REG_TAP_AXES, 0x07);				// Enable axe X Y Z for tap
	ADXL343_WriteRegister(ADXL343_REG_INT_ENABLE, 0x40);			// Enable interruption for single tap
	ADXL343_WriteRegister(ADXL343_REG_INT_MAP, 0x40);				// Enable interruption on pin INT1
	printf("ADXL343 Configuration done\r\n");

}

// Function to read acceleration on XYZ
void ADXL343_Read_XYZ(ADXL343_Handle_t* handle) {

    int8_t buff[6] = {0};
    ADXL343_ReadRegister(ADXL343_REG_DATAX0, buff, 6);

    handle->x = (int16_t)((buff[1] << 8) | buff[0]);
    handle->y = (int16_t)((buff[3] << 8) | buff[2]);
    handle->z = (int16_t)((buff[5] << 8) | buff[4]);


}

//Function to detect a tap
void ADXL343_DetectTap(ADXL343_Handle_t* handle){
	int8_t tap_status;
	ADXL343_ReadRegister(ADXL343_REG_INT_SOURCE, &tap_status, 1); 	//Renvoie la valeur du registre int_source

	//	D7 -> data_ready
	//	D6 -> single_tap
	//	D5 -> double_tap
	//	D4 -> activity
	//	D3 -> inactivity
	//	D2 -> free_fall
	//	D1 -> watermark
	//	D0 -> overrun

	//printf(" tap status : %i\r\n", tap_status);

	if (tap_status & (1<<6))
	{   // Tap for single tap
        handle->tap_status = ADXL343_TAP_SINGLE;
		printf("Tap detected!\r\n");

	    if (robot_state == ROBOT_STATE_CHAT) {
	        robot_state = ROBOT_STATE_SOURIS;
			HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);

	        printf("Robot state changed to SOURIS\r\n");
	        // Ajouter des actions spécifiques à l’état SOURIS

	        HAL_Delay(1000);
	    } else {
	        robot_state = ROBOT_STATE_CHAT;
			HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);

	        printf("Robot state changed to CHAT\r\n");
	        HAL_Delay(1000);
	        // Ajouter des actions spécifiques à l’état CHAT
	    }
	    HAL_Delay(1000);
	}
	else {
		handle->tap_status = ADXL343_TAP_NONE;
		//printf("Tap not detected!\r\n");

	}

}


// Function to calibrate offsets for ADXL343
void ADXL343_CalibrateOffsets(ADXL343_Handle_t* handle) {
    int8_t offsetx, offsety, offsetz;

    ADXL343_Read_XYZ(handle);

    offsetx = -((handle->x + 5) / 4*100);
    offsety = -((handle->y + 5) / 4*100);
    offsetz = -(((handle->z - 256) + 5) / 4*100);

    ADXL343_WriteRegister(ADXL343_REG_OFFSX, offsetx);
    ADXL343_WriteRegister(ADXL343_REG_OFFSY, offsety);
    ADXL343_WriteRegister(ADXL343_REG_OFFSZ, offsetz);

    printf("Calibration done - Offset X: %d, Y: %d, Z: %d\r\n", offsetx, offsety, offsetz);
}

void ADXL343_Task(void*unused )
{
    ADXL343_Handle_t accel;
    while (ADXL343_Init(&accel)) {}

	ADXL343_Configure(&accel);

	for(;;)
	{
		ADXL343_Read_XYZ(&accel);
        ADXL343_DetectTap(&accel);
        vTaskDelay(250);

        //printf("X: %d, Y: %d, Z: %d\r\n", accel.x, accel.y, accel.z);


	}
}

void ADXL343_TaskCreate(void * unused)
{
	xTaskCreate(ADXL343_Task, "ADXL343 task", 128, NULL, ADXL343_Task_Priority, NULL);
}
