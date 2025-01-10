/*
 * ADXL343_driver.h
 *
 *  Created on: Sep 16, 2024
 *      Author: marie et charlotte
 */

#ifndef INC_ADXL343_DRIVER_H_
#define INC_ADXL343_DRIVER_H_

#include <stdint.h>
#include <stddef.h>

// register for ADXL343
#define ADXL343_REG_DEVID 		0x00
#define ADXL343_REG_THRESH_TAP  0x1D
#define ADXL343_REG_OFFSX 		0x1E
#define ADXL343_REG_OFFSY 		0x1F
#define ADXL343_REG_OFFSZ 		0x20
#define ADXL343_REG_DUR 		0x21
#define ADXL343_REG_LATENT 		0x22
#define ADXL343_REG_THRESH_ACT 	0x24
#define ADXL343_REG_TAP_AXES 	0x2A
#define ADXL343_REG_BW_RATE 	0x2C
#define ADXL343_REG_POWER_CTL 	0x2D
#define ADXL343_REG_INT_ENABLE 	0x2E
#define ADXL343_REG_INT_MAP 	0x2F
#define ADXL343_REG_INT_SOURCE 	0x30
#define ADXL343_REG_DATA_FORMAT 0x31
#define ADXL343_REG_DATAX0 		0x32
#define ADXL343_REG_DATAX1 		0x33
#define ADXL343_REG_DATAY0 		0x34
#define ADXL343_REG_DATAY1 		0x35
#define ADXL343_REG_DATAZ0 		0x36
#define ADXL343_REG_DATAZ1 		0x37

#define ADXL343_Task_Priority 	8

// Threshold and timing constants
#define TAP_THRESHOLD 			0x20											// Tapping threshold 2g -> 0x20, 6g -> 0x60, 8g -> 0x80, 16g -> 0xFF
#define TAP_DURATION 			0x10 											// Tapping duration 10ms
#define TAP_LATENT 				0xC8

// Enumerations for ADXL343 states
typedef enum {
    ADXL343_STATUS_NOT_DETECTED = 0,
    ADXL343_STATUS_INITIALIZED = 1,
} ADXL343_Status_t;

typedef enum {
    ADXL343_AXIS_X = 0,
    ADXL343_AXIS_Y = 1,
    ADXL343_AXIS_Z = 2,
} ADXL343_Axis_t;

typedef enum {
    ADXL343_TAP_NONE = 0,
    ADXL343_TAP_SINGLE = 1,
    // ADXL343_TAP_DOUBLE = 2,
} ADXL343_TapStatus_t;

typedef enum {
    ROBOT_STATE_CHAT,
    ROBOT_STATE_SOURIS,
} RobotState_t;


// Structure for accelerometer data
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
    ADXL343_TapStatus_t tap_status;
    ADXL343_Status_t status;
} ADXL343_Handle_t;

// functions for the accelerometer
int ADXL343_Init(ADXL343_Handle_t* handle);
void ADXL343_Configure(ADXL343_Handle_t* handle);
void ADXL343_ReadRegister(uint8_t reg, int8_t* rx_data, size_t length);
void ADXL343_WriteRegister(uint8_t reg, uint8_t data);
void ADXL343_DetectTap(ADXL343_Handle_t* handle);
void ADXL343_Read_XYZ(ADXL343_Handle_t* handle);
void ADXL343_Task(void * unused);
void ADXL343_TaskCreate(void * unused);

void ADXL343_CalibrateOffsets(ADXL343_Handle_t* handle);

#endif /* INC_ADXL343_DRIVER_H_ */
