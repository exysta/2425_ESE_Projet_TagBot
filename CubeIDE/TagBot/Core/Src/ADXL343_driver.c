/*
 * ADXL343_driver.c
 *
 *  Created on: Sep 16, 2024
 *      Author: marie & charlotte
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
//#define TAP_THRESHOLD 0xFF // Tapping threshold 16g
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
	//ADXL343_WriteRegister(ADXL343_REG_POWER_CTL, 0x04); 			// init the power control (sleep)

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // Activer NSS
	ADXL343_WriteRegister(ADXL343_REG_DATA_FORMAT, 0x00);  			// establish format for data :full resolution and ±2g
	//ADXL343_WriteRegister(ADXL343_REG_DATA_FORMAT, 0x0B);  			// establish format for data :full resolution and ±16g
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET); // Activer NSS

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // Activer NSS
	ADXL343_WriteRegister(ADXL343_REG_BW_RATE, 0X0A);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET); // Activer NSS

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // Activer NSS
	ADXL343_WriteRegister(ADXL343_REG_POWER_CTL, 0x08);  			// measurement mode of power control (active)
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET); // Activer NSS


	printf("Initialization done\r\n");
	return 0;


}

//Function to configure the accelerometer
void ADXL343_Configure(void){
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // Activer NSS
	ADXL343_WriteRegister(ADXL343_REG_THRESH_TAP, TAP_THRESHOLD);  	// Set tap threshold : 2g ou 16g
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET); // Activer NSS

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // Activer NSS
	ADXL343_WriteRegister(ADXL343_REG_DUR, TAP_DURATION);         	// Set tap duration : 10ms
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET); // Activer NSS

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // Activer NSS
	ADXL343_WriteRegister(ADXL343_REG_LATENT, TAP_LATENT);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET); // Activer NSS

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // Activer NSS
	ADXL343_WriteRegister(ADXL343_REG_TAP_AXES, 0x07);				// Enable axe X Y Z for tap
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET); // Activer NSS

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
void ADXL343_ReadXYZ(uint16_t* x, uint16_t* y, uint16_t* z) {
    uint8_t buffer[2]; // Tableau de deux octets pour stocker les données

    ADXL343_ReadRegister(ADXL343_REG_DATAX0, &buffer[0], 1); // Adresse de buffer[0]
    ADXL343_ReadRegister(ADXL343_REG_DATAX1, &buffer[1], 1); // Adresse de buffer[1]

    // Affichage des valeurs lues
    *x = (uint16_t)((buffer[1] << 8) | buffer[0]); // MSB | LSB pour X

    ADXL343_ReadRegister(ADXL343_REG_DATAY0, &buffer[0], 1); // Adresse de buffer[0]
    ADXL343_ReadRegister(ADXL343_REG_DATAY1, &buffer[1], 1); // Adresse de buffer[1]

    *y = (int16_t)((buffer[3] << 8) | buffer[2]); // MSB | LSB pour Y

    ADXL343_ReadRegister(ADXL343_REG_DATAZ0, &buffer[0], 1); // Adresse de buffer[0]
    ADXL343_ReadRegister(ADXL343_REG_DATAZ1, &buffer[1], 1); // Adresse de buffer[1]

    *z = (int16_t)((buffer[5] << 8) | buffer[4]); // MSB | LSB pour Z

    HAL_Delay(2000);
}


void ADXL343_Read_CHAT(uint16_t* x) {
    uint8_t buffer[2];

    // Lecture multiple des registres DATAX0 à DATAZ1
    ADXL343_ReadRegister(ADXL343_REG_DATAX0, buffer, 2);

    printf("buffer, %d \r\n:", buffer);

    // Combinaison des octets MSB et LSB pour les axes X, Y et Z
    *x = (uint16_t)((buffer[1] << 8) | buffer[0]); // MSB | LSB pour X
    //*y = (int16_t)((buffer[3] << 8) | buffer[2]); // MSB | LSB pour Y
    //*z = (int16_t)((buffer[5] << 8) | buffer[4]); // MSB | LSB pour Z
}









/*
void printAccelerometerData(uint8_t reg) {
    int16_t X = 0, Y = 0, Z = 0;
    ADXL343_ReadXYZ(reg, &X, &Y, &Z);  // Lire les valeurs X, Y, Z de l'accéléromètre

    // Afficher les valeurs brutes avant toute conversion
    printf("Brut X : %d\r\n", X);
    printf("Brut Y : %d\r\n", Y);
    printf("Brut Z : %d\r\n", Z);

//    int16_t Xg = (int16_t)X / 256;  // Diviser par 256 pour la plage ±16g
//    int16_t Yg = (int16_t)Y / 256;
//    int16_t Zg = (int16_t)Z / 256;
//
//    printf("X : %d g\r\n", Xg);
//    printf("Y : %d g\r\n", Yg);
//    printf("Z : %d g\r\n", Zg);

    HAL_Delay(2000);  // Attendre 500 ms entre les lectures
}*/
