 /*
 * ADXL343_driver.c
 *
 *  Created on: Sep 16, 2024
 *      Author: marie
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


// Constants for accelerometer configuration
#define TAP_THRESHOLD 0x20											// Tapping threshold 2g -> 0x20, 6g -> 0x60, 8g -> 0x80, 16g -> 0xFF
#define TAP_DURATION 0x10 											// Tapping duration 10ms
#define TAP_LATENT 0xC8 											// taping latency 250ms


//Function to initialize the accelerometer

int ADXL343_Init(void) {
	uint8_t id = 0;

	// read the id of peripherique and check if is equal 0xE5
	// doesn't work if we switch uint to int
	ADXL343_ReadRegister(ADXL343_REG_DEVID, &id, 1);

	if (id != 0xE5) {

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


	printf("Initialization done\r\n");
	return 0;
}

//Function to configure the accelerometer
void ADXL343_Configure(void){
	ADXL343_WriteRegister(ADXL343_REG_THRESH_TAP, TAP_THRESHOLD);  	// Set tap threshold : 2g ou 16g
	ADXL343_WriteRegister(ADXL343_REG_DUR, TAP_DURATION);         	// Set tap duration : 10ms
	ADXL343_WriteRegister(ADXL343_REG_LATENT, TAP_LATENT);
	ADXL343_WriteRegister(ADXL343_REG_TAP_AXES, 0x07);				// Enable axe X Y Z for tap
	ADXL343_WriteRegister(ADXL343_REG_INT_ENABLE, 0x40);			// Enable interruption for single tap
	ADXL343_WriteRegister(ADXL343_REG_INT_MAP, 0x40);				// Enable interruption on pin INT1
	printf("Configuration done\r\n");

}

// Function to read from a register
void ADXL343_ReadRegister(uint8_t reg, int8_t* rx_data, size_t length) {

	uint8_t tx_data = reg | 0x80; // MSB = 1 pour la lecture

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); 			// Activer NSS
	HAL_SPI_Transmit(&hspi1, &tx_data, 1, 200); 			// Envoyer l'adresse
	HAL_SPI_Receive(&hspi1, (uint8_t*)rx_data, 1, 200);  	// Lire la donnée
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);   			// Désactiver NSS


	//HAL_SPI_TransmitReceive(hspi, pTxData, pRxData, Size, Timeout); // a utiliser quand NSS hardware ioc enable
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



//Function to detect a tap
void ADXL343_DetectTap(void){
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

	printf(" tap status : %i\r\n", tap_status);
	if (tap_status & (1<<7)) {  // Tap for single tap
		printf("Tap detected!\r\n");
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
		HAL_Delay(100);
	}
}


// Function to read acceleration on XYZ

void ADXL343_Read_XYZ(int16_t* x, int16_t* y, int16_t* z) {

	int8_t x_buff[2]={0,0};
	int8_t y_buff[2]={0,0};
	int8_t z_buff[2]={0,0};

	// reed register from DATAX0 to DATAZ1
	ADXL343_ReadRegister(ADXL343_REG_DATAX0, x_buff, 2);
	ADXL343_ReadRegister(ADXL343_REG_DATAY0, y_buff, 2);
	ADXL343_ReadRegister(ADXL343_REG_DATAZ0, z_buff, 2);

	// Combinaison des octets MSB et LSB pour les axes X, Y et Z
	*x = (int16_t)((x_buff[1] << 8) | x_buff[0]); // MSB | LSB pour X
	*y = (int16_t)((y_buff[1] << 8) | y_buff[0]); // MSB | LSB pour Y
	*z = (int16_t)((z_buff[1] << 8) | z_buff[0]); // MSB | LSB pour Z


}


// Function to calibrate offsets for ADXL343
void calibrateOffsets(void){
	int16_t x, y, z;
	int8_t offsetx, offsety, offsetz;

	ADXL343_Read_XYZ(&x, &y, &z);

	offsetx = -((x+5)/4*100);
	offsety = -((y+5)/4*100);
	offsetz = -(((z-256)+5)/4*100);

	ADXL343_WriteRegister(ADXL343_REG_OFFSX, offsetx);
	ADXL343_WriteRegister(ADXL343_REG_OFFSY, offsety);
	ADXL343_WriteRegister(ADXL343_REG_OFFSZ, offsetz);

	printf("calibration done offset x : %i, y : %i, z :%i\r\n", offsetx, offsety, offsetz);
}

void ADXL343_Task(void*unused )
{

	while(1 == ADXL343_Init()) {}
	ADXL343_Configure();
	for(;;)
	{

		uint8_t tap_status;
		ADXL343_ReadRegister(ADXL343_REG_INT_SOURCE, &tap_status, 1); //Renvoie la valeur du registre int_source

		if (tap_status & (1<<6)) {  // Tap for single tap
			printf("Tap detected by Task!\r\n");
			vTaskDelay(250);

			/******* Ajouter la tache pour changer d'état********/
		}
		vTaskDelay(100);


	}


}

void ADXL343_TaskCreate(void * unused)
{
	xTaskCreate(ADXL343_Task, "tache ADXL343", 128, NULL, ADXL343_Task_Priority, NULL);
}
