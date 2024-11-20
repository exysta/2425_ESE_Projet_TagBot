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

// Constants for accelerometer configuration
#define TAP_THRESHOLD 0x20 // Tapping threshold 2g
#define TAP_DURATION 0x10 // Tapping duration 10ms
#define TAP_LATENT 0xC8 // taping latency 250ms


//Function to initialize the accelerometer

int ADXL343_Init(void) {
	uint8_t id = 0;

	// read the id of peripherique and check if is equal 0xE5
	ADXL343_ReadRegister(ADXL343_REG_DEVID, &id, 1);

	if (id != 0xE5) {

		printf("ADXL343 no detected ! ID: %02X\r\n", id);
		return 1;
	}


	// Configuration of registers
	ADXL343_WriteRegister(ADXL343_REG_POWER_CTL, 0x04); 			// init the power control (sleep)
	ADXL343_WriteRegister(ADXL343_REG_DATA_FORMAT, 0x08);  			// establish format for data :full resolution and ±2g
	ADXL343_WriteRegister(ADXL343_REG_POWER_CTL, 0x08);  			// measurement mode of power control (active)

	printf("Initialization done\r\n");
	return 0;


}

//Function to configure the accelerometer
void ADXL343_Configure(void){
	ADXL343_WriteRegister(ADXL343_REG_THRESH_TAP, TAP_THRESHOLD);  	// Set tap threshold : 2g
	ADXL343_WriteRegister(ADXL343_REG_DUR, TAP_DURATION);         	// Set tap duration : 10ms
	ADXL343_WriteRegister(ADXL343_REG_LATENT, TAP_LATENT);
	ADXL343_WriteRegister(ADXL343_REG_TAP_AXES, 0x06);				// Enable axe X Y for tap
	//ADXL343_WriteRegister(ADXL343_REG_INT_ENABLE, 0x40);			// Enable interruption for single tap
	printf("Configuration done\r\n");

}

// Function to read from a register
void ADXL343_ReadRegister(uint8_t reg, uint8_t* rx_data, size_t length) {

    uint8_t tx_data = reg | 0x80; // MSB = 1 pour la lecture

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // Activer NSS
    HAL_SPI_Transmit(&hspi1, &tx_data, 1, HAL_MAX_DELAY); // Envoyer l'adresse
    HAL_SPI_Receive(&hspi1, rx_data, 1, HAL_MAX_DELAY);  // Lire la donnée
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);   // Désactiver NSS


    //HAL_SPI_TransmitReceive(hspi, pTxData, pRxData, Size, Timeout); // a utiliser quand NSS hardware ioc enable
}


//Function to write in the register
void ADXL343_WriteRegister(uint8_t reg, uint8_t data) {
    uint8_t buffer[2];
    buffer[0] = reg;	//register
    buffer[1] = data;	//data

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // enable NSS (PA4)
    HAL_SPI_Transmit(&hspi1, buffer, 2, HAL_MAX_DELAY); // Send data thanks to SPI
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET); // disable NSS (PA4)
}

//Function to detect a tap
void ADXL343_DetectTap(void){
	uint8_t tap_status;
	ADXL343_ReadRegister(ADXL343_REG_INT_SOURCE, &tap_status, 1);
	printf(" tap status : %u\r\n", tap_status);
	HAL_Delay(100);
	if (tap_status & 0x40) {  			// Tap for single tap
		printf("Tap detected!\r\n");
	}
}

//Function to read the accelerometer data
void ADXL343_ReadXYZ(int16_t* x, int16_t* y, int16_t* z) {
    uint8_t buffer[6];
    ADXL343_ReadRegister(ADXL343_REG_DATAX0, buffer, 6);		// read 6 octets from DATAx0 address

    *x = (int16_t)((buffer[1] << 8) | buffer[0]);
    *y = (int16_t)((buffer[3] << 8) | buffer[2]);
    *z = (int16_t)((buffer[5] << 8) | buffer[4]);
}
