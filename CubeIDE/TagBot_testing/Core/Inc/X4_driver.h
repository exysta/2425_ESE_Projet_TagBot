/**
 * @file X4_driver.h
 * @brief Header file for X4 LiDAR driver.
 *
 * This file contains definitions, structures, and function prototypes
 * for controlling the X4 model LiDAR sensor. It includes commands for
 * starting and stopping scans, obtaining device information and health
 * status, and handling responses.
 *
 * @date Created on: Sep 11, 2024
 * @author exysta
 */

#ifndef INC_X4_DRIVER_H_
#define INC_X4_DRIVER_H_

#include "stdint.h"

#define X4_UART huart3
/**
 * @def X4_CMD_START
 * @brief Start command identifier for LiDAR communication.
 */
#define X4_CMD_START 0xA5

/**
 * @def X4_CMD_START_SCAN
 * @brief Command to start scanning with the LiDAR.
 */
#define X4_CMD_START_SCAN 0x60

/**
 * @def X4_CMD_STOP_SCAN
 * @brief Command to stop the LiDAR scan.
 */
#define X4_CMD_STOP_SCAN 0x65

/**
 * @def X4_CMD_GET_INFO
 * @brief Command to request the device information.
 */
#define X4_CMD_GET_INFO 0x90

/**
 * @def X4_CMD_GET_HEALTH
 * @brief Command to retrieve the health status of the LiDAR.
 */
#define X4_CMD_GET_HEALTH 0x91

/**
 * @def X4_CMD_SOFT_RESTART
 * @brief Command to trigger a soft restart of the LiDAR.
 */
#define X4_CMD_SOFT_RESTART 0x80

/**
 * @def X4_RESPONSE_START_SIGN
 * @brief Start sign for LiDAR response.
 */
#define X4_RESPONSE_START_SIGN 0xA55A

/**
 * @def X4_RESPONSE_SINGLE_MODE
 * @brief Response mode indicating single mode operation.
 */
#define X4_RESPONSE_SINGLE_MODE 0x0

/**
 * @def X4_RESPONSE_CONTINUOUS_MODE
 * @brief Response mode indicating continuous mode operation.
 */
#define X4_RESPONSE_CONTINUOUS_MODE 0x1

/**
 * @def X4_RESPONSE_HEADER_SIZE
 * @brief Size of the header portion in the response.
 */
#define X4_RESPONSE_HEADER_SIZE 6

/**
 * @def X4_MAX_RESPONSE_SIZE
 * @brief Maximum size of the response content.
 */
#define X4_MAX_RESPONSE_SIZE 64

/**
 * @def X4_SERIAL_NUMBER_SIZE
 * @brief Size of the LiDAR serial number.
 */
#define X4_SERIAL_NUMBER_SIZE 16

/**
 * @def X4_SERIAL_FIRMWARE_SIZE
 * @brief Size of the firmware version information.
 */
#define X4_SERIAL_FIRMWARE_SIZE 2

/**
 * @brief Starts a scan operation on the X4 LiDAR.
 *
 * This function sends the command to start a scanning operation,
 * continuously acquiring distance measurements.
 */
void X4_StartScan(void);

/**
 * @brief Stops the scanning operation on the X4 LiDAR.
 *
 * This function sends the command to stop the ongoing scanning operation.
 */
void X4_StopScan(void);

/**
 * @brief Retrieves the device information from the X4 LiDAR.
 *
 * This function requests and retrieves the device model, firmware version,
 * hardware version, and serial number from the LiDAR.
 */
void X4_GetDeviceInfo(void);

/**
 * @brief Retrieves the health status of the X4 LiDAR.
 *
 * This function requests and retrieves the health status of the LiDAR,
 * indicating whether the device is functioning properly.
 */
void X4_GetHealthStatus(void);

/**
 * @brief Performs a soft restart of the X4 LiDAR.
 *
 * This function sends the command to perform a soft restart,
 * reinitializing the device without physically powering it off.
 */
void X4_SoftRestart(void);

/**
 * @brief Structure to represent a parsed response message from the X4 LiDAR.
 */
typedef struct {
    uint16_t start_sign;               /**< Start sign of the response message. */
    uint32_t response_length;          /**< Length of the response content (30 bits). */
    uint8_t response_mode;             /**< Mode of the response (2 bits). */
    uint8_t type_code;                 /**< Type code of the response. */
    uint8_t content[X4_MAX_RESPONSE_SIZE]; /**< Content data of the response. */
} X4_ResponseMessage;

/**
 * @brief Structure to store device information for the X4 LiDAR.
 */
typedef struct {
    uint8_t model;                          /**< Model of the LiDAR. */
    uint8_t firmware[X4_SERIAL_FIRMWARE_SIZE]; /**< Firmware version of the LiDAR. */
    uint8_t hardware_version;               /**< Hardware version of the LiDAR. */
    uint8_t serial_number[X4_SERIAL_NUMBER_SIZE]; /**< Serial number of the LiDAR. */
} X4_DeviceInfo;

/**
 * @brief Structure to store parsed scan data from the X4 LiDAR.
 */
typedef struct {
    uint8_t packet_header[2];       /**< Header of the scan data packet. */
    uint8_t packet_type;            /**< Type of the scan data packet. */
    uint8_t sample_quantity;        /**< Number of samples in the scan data. */
    uint8_t start_angle[2];         /**< Start angle of the scan. */
    uint8_t end_angle[2];           /**< End angle of the scan. */
    uint8_t check_code[2];          /**< Checksum for data verification. */
    uint8_t *sample_data;           /**< Pointer to the sample data. */
    int *distances;                 /**< Pointer to the array of distance values. */
    float *angles;                  /**< Pointer to the array of angle values. */
} X4_ScanData;

/**
 * @brief Handles the response received from the X4 LiDAR.
 *
 * This function processes the incoming response and determines
 * the appropriate action based on the type and content of the message.
 */
void X4_HandleResponse(void);

/**
 * @brief Parses a raw response message from the X4 LiDAR.
 *
 * This function takes the raw data received from the LiDAR and parses
 * it into a structured response message.
 *
 * @param raw_data Pointer to the raw data buffer.
 * @param response Pointer to the X4_ResponseMessage structure to store the parsed data.
 */
void X4_ParseMessage(const uint8_t *raw_data, X4_ResponseMessage *response);

/**
 * @brief Converts two bytes into a uint16_t value.
 *
 * This function converts a two-byte array into a 16-bit unsigned integer.
 *
 * @param byte_array Pointer to the two-byte array.
 * @return The converted 16-bit unsigned integer.
 */
uint16_t convertBytesToUint16(const uint8_t* byte_array);

#endif /* INC_X4_DRIVER_H_ */
