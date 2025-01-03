/*
 * ADXL343_driver.h
 *
 *  Created on: Sep 16, 2024
 *      Author: marie
 */

#ifndef INC_ADXL343_DRIVER_H_
#define INC_ADXL343_DRIVER_H_

#include <stdint.h>
#include <stddef.h>

// register for ADXL343
#define ADXL343_REG_DEVID 0x00
#define ADXL343_REG_THRESH_TAP 0x1D
#define ADXL343_REG_OFFSX 0x1E
#define ADXL343_REG_OFFSY 0x1F
#define ADXL343_REG_OFFSZ 0x20
#define ADXL343_REG_DUR 0x21
#define ADXL343_REG_LATENT 0x22
#define ADXL343_REG_THRESH_ACT 0x24
#define ADXL343_REG_TAP_AXES 0x2A
#define ADXL343_REG_BW_RATE 0x2C
#define ADXL343_REG_POWER_CTL 0x2D
#define ADXL343_REG_INT_ENABLE 0x2E
#define ADXL343_REG_INT_MAP 0x2F
#define ADXL343_REG_INT_SOURCE 0x30
#define ADXL343_REG_DATA_FORMAT 0x31
#define ADXL343_REG_DATAX0 0x32
#define ADXL343_REG_DATAX1 0x33
#define ADXL343_REG_DATAY0 0x34
#define ADXL343_REG_DATAY1 0x35
#define ADXL343_REG_DATAZ0 0x36
#define ADXL343_REG_DATAZ1 0x37

#define ADXL343_Task_Priority 8
// functions for the accelerometer
int ADXL343_Init(void);
void ADXL343_Configure(void);
void ADXL343_ReadRegister(uint8_t reg, int8_t* rx_data, size_t length);

void ADXL343_WriteRegister(uint8_t reg, uint8_t data);
void ADXL343_DetectTap(void);

void ADXL343_Read_XYZ(int16_t* x, int16_t*y, int16_t*z);

void calibrateOffsets(void);

void ADXL343_Task(void * unused);
void ADXL343_TaskCreate(void * unused);



#endif /* INC_ADXL343_DRIVER_H_ */
