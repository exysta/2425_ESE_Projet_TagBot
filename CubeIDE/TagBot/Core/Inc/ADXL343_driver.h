/*
 * ADXL343_driver.h
 *
 *  Created on: Sep 16, 2024
 *      Author: marie
 */

#ifndef INC_ADXL343_DRIVER_H_
#define INC_ADXL343_DRIVER_H_


// register for ADXL343
#define ADXL343_REG_DEVID 0x00
#define ADXL343_REG_THRESH_TAP 0x1D
#define ADXL343_REG_DUR 0X21
#define ADSL343_REG_THRESH_ACT 0x24
#define ADXL343_REG_POWER_CTL 0x2D
#define ADXL343_REG_DATA_FORMAT 0x31
#define ADXL343_REG_DATAX0 0x32
#define ADXL343_REG_DATAX1 0x33
#define ADXL343_REG_DATAY0 0x34
#define ADXL343_REG_DATAY1 0x35
#define ADXL343_REG_DATAZ0 0x36
#define ADXL343_REG_DATAZ1 0x37

// functions for the accelerometer
void ADXL343_Init(void);
void ADxl343_Configure(void);
void ADXL343_ReadRegister(void);
void ADXL343_DetectTap(void);



#endif /* INC_ADXL343_DRIVER_H_ */
