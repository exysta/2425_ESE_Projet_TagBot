/*
 * X4_driver.c.h
 *
 *  Created on: Sep 11, 2024
 *      Author: exysta
 */

#ifndef INC_X4_DRIVER_H_
#define INC_X4_DRIVER_H_

#include "stdint.h"
// System command bytes
// a command always start by CMD_START, the following byte in the command content
#define X4_CMD_START 0xA5
#define X4_CMD_START_SCAN 0x60
#define X4_CMD_STOP_SCAN 0x65
#define X4_CMD_GET_INFO 0x90
#define X4_CMD_GET_HEALTH 0x91
#define X4_CMD_SOFT_RESTART 0x80

// System response protocol definitions
#define X4_RESPONSE_START_SIGN 0xA55A
#define X4_RESPONSE_SINGLE_MODE 0x0
#define X4_RESPONSE_CONTINUOUS_MODE 0x1

// Define size of different parts of the response
#define X4_RESPONSE_HEADER_SIZE 6
#define X4_MAX_RESPONSE_SIZE 64  // Adjust as needed for content size
#define X4_SERIAL_NUMBER_SIZE 16
#define X4_SERIAL_FIRMWARE_SIZE 2

// Function prototypes
void X4_StartScan(void);
void X4_StopScan(void);
void X4_GetDeviceInfo(void);
void X4_GetHealthStatus(void);
void X4_SoftRestart(void);

// Structure for parsed response message
typedef struct {
    uint16_t start_sign;
    uint32_t response_length; //30 bits in reality
    uint8_t response_mode; //2 bits in reality
    uint8_t type_code;
    uint8_t content[X4_MAX_RESPONSE_SIZE];
} X4_ResponseMessage;

// Structure for device info
typedef struct {
	uint8_t model;
	uint8_t firmware[X4_SERIAL_FIRMWARE_SIZE];
    uint8_t hardware_version;
    uint8_t serial_number[X4_SERIAL_NUMBER_SIZE];
} X4_DeviceInfo;


// Structure for parsed response message
typedef struct {
	uint8_t packet_header[2];
	uint8_t packet_type;
	uint8_t sample_quantity;
	uint8_t start_angle[2];
	uint8_t end_angle[2];
	uint8_t check_code[2];
	uint8_t * sample_data;
	int * distances;
	float * angles;
} X4_ScanData;

// Function prototypes
void X4_StartScan(void);
void X4_StopScan(void);
void X4_GetDeviceInfo(void);
void X4_GetHealthStatus(void);
void X4_SoftRestart(void);

void X4_HandleResponse(void);
void X4_ParseMessage(const uint8_t *raw_data, X4_ResponseMessage *response);
uint16_t convertBytesToUint16(const uint8_t* byte_array);

#endif /* INC_X4_DRIVER_H_ */
