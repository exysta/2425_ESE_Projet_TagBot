/*
 * X4_driver.c
 *
 *  Created on: Sep 11, 2024
 *      Author: exysta
 */

#include <X4_driver.h>
#include "main.h"
#include <string.h>
#include <math.h>    // For atan function
#include <stdlib.h>
#include <stdbool.h>

extern UART_HandleTypeDef huart3; /**< UART handle for UART4 communication */

/**
 * @brief Sends a command to the device via UART.
 *
 * This function constructs a command message with a predefined start byte
 * and sends it to the device using the HAL_UART_Transmit function.
 *
 * @param command The command byte to be sent to the device.
 */
static void X4_SendCommand(uint8_t command)
{
    uint8_t data[2] = {X4_CMD_START, command}; /**< Array holding the command data */
    HAL_UART_Transmit(&huart3, data, 2, HAL_MAX_DELAY);  // Transmit command over UART
}

/**
 * @brief Initiates the scanning process on the device.
 *
 * This function sends a command to start scanning and handles any sustained
 * responses from the device.
 */
void X4_StartScan(void)
{
    X4_SendCommand(X4_CMD_START_SCAN); /**< Command to start scanning */
    // Additional logic to handle sustained responses if needed
    X4_HandleResponse(); /**< Process the device's response */
}

/**
 * @brief Stops the scanning process on the device.
 *
 * This function sends a command to stop scanning on the device.
 */
void X4_StopScan(void)
{
    X4_SendCommand(X4_CMD_STOP_SCAN); /**< Command to stop scanning */
}

/**
 * @brief Performs a soft restart of the device.
 *
 * This function sends a command to the device to perform a soft restart.
 */
void X4_SoftRestart(void)
{
    X4_SendCommand(X4_CMD_SOFT_RESTART); /**< Command to perform soft restart */
}

/**
 * @brief Sends a command to retrieve device information.
 *
 * This function sends the command to get device information and
 * handles the response from the device.
 */
void X4_GetDeviceInfo()
{
    // Send the device info command
    X4_SendCommand(X4_CMD_GET_INFO); /**< Command to get device information */

    // Wait for and handle response
    X4_HandleResponse(); /**< Process the device's response */
}

/**
 * @brief Sends a command to retrieve device health information.
 *
 * This function sends the command to get device health status and
 * handles the response from the device.
 */
void X4_GetDeviceHealth()
{
    // Send the device health command
    X4_SendCommand(X4_CMD_GET_HEALTH); /**< Command to get device health */

    // Wait for and handle response
    X4_HandleResponse(); /**< Process the device's response */
}



/**
 * @brief Handles the Device Info response from the device.
 *
 * This function processes the response message containing device information
 * and extracts relevant data such as model, firmware version, hardware version,
 * and serial number.
 *
 * @param response Pointer to the X4_ResponseMessage structure containing the response data.
 */
void X4_HandleDeviceInfoResponse(const X4_ResponseMessage* response) {
    X4_DeviceInfo device_info;  /**< Structure to hold the extracted device information */

    device_info.model = response->content[0];  /**< Model of the device */
    device_info.firmware[0] = response->content[1];  // Major firmware version
    device_info.firmware[1] = response->content[2];  // Minor firmware version
    device_info.hardware_version = response->content[3];  /**< Hardware version of the device */

    // Copy serial number from the response
    for (int i = 0; i < X4_SERIAL_NUMBER_SIZE; i++) {
        device_info.serial_number[i] = response->content[4 + i];  /**< Serial number */
    }

    // Log device information or update system state with device_info
    // Add logging or state update code here if needed
}

/**
 * @brief Handles the Device Health response from the device.
 *
 * This function processes the response message containing device health information
 * such as status and error codes.
 *
 * @param response Pointer to the X4_ResponseMessage structure containing the response data.
 */
void X4_HandleDeviceHealthResponse(const X4_ResponseMessage* response) {
    uint8_t status_code = response->content[0];  /**< Device status code */
    uint8_t error_code[2];  /**< Array to hold error codes */

    error_code[0] = response->content[1];  /**< First error code */
    error_code[1] = response->content[2];  /**< Second error code */

    // Log or handle device health status and error codes
    // Add logging or handling code here if needed
}

/**
 * @brief Handles the Scan response from the device.
 *
 * This function processes the response message containing scan data, including
 * packet headers, packet type, angles, and sample data.
 *
 * @param response Pointer to the X4_ResponseMessage structure containing the response data.
 */
void X4_HandleScanResponse(const X4_ResponseMessage* response) {
    X4_ScanData scan_data;  /**< Structure to hold the extracted scan data */

    // Extract packet header and relevant scan data
    scan_data.packet_header[0] = response->content[0];
    scan_data.packet_header[1] = response->content[1];
    scan_data.packet_type = response->content[2];
    scan_data.sample_quantity = response->content[3];
    scan_data.start_angle[0] = response->content[4];
    scan_data.start_angle[1] = response->content[5];
    scan_data.end_angle[0] = response->content[6];
    scan_data.end_angle[1] = response->content[7];
    scan_data.check_code[0] = response->content[8];
    scan_data.check_code[1] = response->content[9];

    // Allocate memory for sample data
    scan_data.sample_data = malloc(scan_data.sample_quantity * sizeof(uint16_t));
    if (scan_data.sample_data == NULL) {
        // Handle memory allocation failure
        return; // Or add error handling as needed
    }

    // Extract sample data from the response
    for (int i = 0; i < scan_data.sample_quantity; i += 2) {
        scan_data.sample_data[i] = response->content[10 + i];   // First byte of sample
        scan_data.sample_data[i + 1] = response->content[11 + i]; // Second byte of sample
    }

    // Free(scan_data.sample_data); // Uncomment to free memory if it's not needed anymore
    // Add any necessary processing or logging for scan_data here
}


/**
 * @brief Handles the response from the device.
 *
 * This function receives a response message from the device over UART,
 * parses the message, and calls the appropriate handler based on the response mode
 * and type code.
 */
void X4_HandleResponse(void) {
    uint8_t raw_data[X4_MAX_RESPONSE_SIZE + X4_RESPONSE_HEADER_SIZE]; /**< Buffer for raw received data */
    X4_ResponseMessage response;  /**< Structure to hold parsed response message */

    // Receive data from the UART (HAL_UART_Receive should be non-blocking or with timeout)
    HAL_StatusTypeDef status = HAL_UART_Receive(&huart3, raw_data, sizeof(raw_data), HAL_TIMEOUT);
    if (status == HAL_OK) {
        // Parse the received message
        X4_ParseMessage(raw_data, &response);

        // Handle the parsed response based on its mode and type code
        if (response.response_mode == X4_RESPONSE_SINGLE_MODE) {
            // Process the single response based on the type code
            switch (response.type_code) {
                case 0x04:  // Device info response
                    X4_HandleDeviceInfoResponse(&response);
                    break;
                case 0x06:  // Health status response
                    X4_HandleDeviceHealthResponse(&response);
                    break;
                default:
                    // Handle unknown type codes
                    // Add logging or error handling here if needed
                    break;
            }
        } else if (response.response_mode == X4_RESPONSE_CONTINUOUS_MODE) {
            // Handle continuous mode response
            X4_HandleScanResponse(&response);
        }
    } else {
        // Handle UART receive error
        // Add error handling code here, e.g., logging or retry logic
    }
}

/**
 * @brief Parses a raw message received from the device.
 *
 * This function extracts relevant information from a raw message and populates
 * the provided X4_ResponseMessage structure with parsed data.
 *
 * @param raw_data Pointer to the raw data received from the device.
 * @param response Pointer to the X4_ResponseMessage structure to store the parsed response.
 */
void X4_ParseMessage(const uint8_t *raw_data, X4_ResponseMessage *response) {
    // Parse the start sign
    response->start_sign = (raw_data[1] << 8) | raw_data[0];  /**< Combine two bytes for the start sign */

    if (response->start_sign != X4_RESPONSE_START_SIGN) {
        // Invalid message, handle error (log, return, etc.)
        return;
    }

    // Parse response length (lower 6 bits of the 6th byte) and mode (upper 2 bits)
    response->response_length = raw_data[2] & 0x3F;  // Lower 6 bits
    response->response_mode = (raw_data[2] >> 6) & 0x03;  // Upper 2 bits

    // Parse the type code
    response->type_code = raw_data[3];  /**< Type code of the response */

    // Copy the content (assuming fixed-length content for simplicity)
    uint32_t content_length = (response->response_mode == X4_RESPONSE_CONTINUOUS_MODE) ?
            X4_MAX_RESPONSE_SIZE : response->response_length; /**< Determine content length based on mode */

    for (int i = 0; i < content_length; i++) {
        response->content[i] = raw_data[4 + i];  /**< Copy content from raw data */
    }
}


/**
 * @brief Calculates the distances of every point scanned.
 *
 * This function processes the scanned sample data and calculates the distances
 * in millimeters for each point scanned. The calculated distances are stored
 * in the `distances` array of the provided X4_ScanData structure.
 *
 * @param scan_data Pointer to the X4_ScanData structure containing sample data.
 */
void X4_HandleScanDataDistances(X4_ScanData *scan_data) {
    // Calculate the number of distances to be processed
    int num_distances = scan_data->sample_quantity;  /**< Number of distance samples */

    // Allocate memory for distances
    scan_data->distances = malloc(num_distances * sizeof(int));
    if (scan_data->distances == NULL) {
        // Handle memory allocation failure
        return; // Or add error handling as needed
    }

    // Each sample has 2 bytes; process each sample
    for (int i = 0; i < num_distances * 2; i += 2) {
        uint8_t sample[2];
        sample[0] = scan_data->sample_data[i];
        sample[1] = scan_data->sample_data[i + 1];

        uint16_t hex_distance = convertBytesToUint16(sample);  /**< Convert bytes to uint16_t */
        // The distance is in mm according to the development manual
        // We calculate with int instead of float for better performance
        // Adding +2 rounds to the nearest integer when dividing by 4
        int mm_distance = (hex_distance + 2) / 4;

        // Store the calculated distance in the array
        scan_data->distances[i / 2] = mm_distance;  /**< Store distance in mm */
    }
}

/**
 * @brief Calculates the angles of every point scanned.
 *
 * This function processes the angle data from the scan and calculates the angles
 * for each scanned point. The calculated angles are stored in the `angles` array
 * of the provided X4_ScanData structure, applying corrections based on the distances.
 *
 * @param scan_data Pointer to the X4_ScanData structure containing angle data.
 */
void X4_HandleScanDataAngles(X4_ScanData *scan_data) {
    // Get the number of samples in the frame (LSN)
    uint8_t LSN = scan_data->sample_quantity;  /**< Number of samples in the scan */

    // Allocate memory for angles
    scan_data->angles = malloc(LSN * sizeof(float));
    if (scan_data->angles == NULL) {
        // Handle memory allocation failure
        return; // Or add error handling as needed
    }

    // Step 1: First-level angle analysis
    // Convert the start_angle and end_angle from byte arrays to uint16_t
    uint16_t FSA_raw = convertBytesToUint16(scan_data->start_angle);  /**< First Sample Angle (FSA) */
    uint16_t LSA_raw = convertBytesToUint16(scan_data->end_angle);    /**< Last Sample Angle (LSA) */

    // Calculate AngleFSA and AngleLSA by right-shifting and dividing
    float AngleFSA = (float)(FSA_raw >> 1) / 64.0f;  // First Sample Angle
    float AngleLSA = (float)(LSA_raw >> 1) / 64.0f;  // Last Sample Angle

    // Calculate angle difference and handle wrapping (if necessary)
    float angle_diff = AngleLSA - AngleFSA;
    if (angle_diff < 0) {
        angle_diff += 360.0f;  // Wrap-around in clockwise direction
    }

    // Step 2: Calculate intermediate angles
    scan_data->angles[0] = AngleFSA;  // First sample angle (FSA)
    for (int i = 1; i < LSN; ++i) {
        scan_data->angles[i] = (angle_diff / (LSN - 1)) * (i - 1) + AngleFSA;  /**< Interpolated angles */
    }

    // Step 3: Second-level angle correction based on distances
    for (int i = 0; i < LSN; ++i) {
        uint16_t distance = scan_data->distances[i];  /**< Distance of the current sample */
        float AngCorrect = 0.0f;  /**< Angle correction factor */

        if (distance > 0) {
            // Apply the angle correction formula
            AngCorrect = atanf(21.8f * (155.3f - (float)distance) / (155.3f * (float)distance));
            AngCorrect = AngCorrect * (180.0f / M_PI);  // Convert from radians to degrees
        }

        // Apply correction to the angle
        scan_data->angles[i] += AngCorrect;

        // Ensure angle is within [0, 360) range
        if (scan_data->angles[i] >= 360.0f) {
            scan_data->angles[i] -= 360.0f;
        } else if (scan_data->angles[i] < 0.0f) {
            scan_data->angles[i] += 360.0f;
        }

        // Handle or store the corrected angle for each scan point (angles[i])
        // For example, you could print or log these angles:
        // printf("Angle[%d]: %f degrees\n", i, scan_data->angles[i]);
    }
}

/**
 * @brief Calculate the XOR of all bytes in the packet excluding the check code.
 *
 * This function iterates through the bytes of the given packet data,
 * applying the XOR operation to each byte except for the last two bytes,
 * which are assumed to be the check code.
 *
 * @param scan_data Pointer to the packet data structure of type X4_ScanData.
 * @param packet_length Length of the packet in bytes.
 * @return The calculated XOR value as a 16-bit unsigned integer.
 */
uint16_t calculateXOR(const X4_ScanData *scan_data, size_t packet_length) {
    uint16_t xor_result = 0;

    // XOR all bytes in the packet except the check code
    for (size_t i = 0; i < (packet_length - 2); ++i) {
        // Use XOR operation on each byte of the packet
        xor_result ^= ((const uint8_t*)scan_data)[i];
    }

    return xor_result;
}

/**
 * @brief Verify the check code of the packet.
 *
 * This function calculates the XOR of the packet data and compares it
 * against the provided check code in the packet to verify its integrity.
 *
 * @param scan_data Pointer to the packet data structure of type X4_ScanData.
 * @param packet_length Length of the packet in bytes.
 * @return True if the calculated XOR matches the check code, false otherwise.
 */
bool verifyCheckCode(const X4_ScanData *scan_data, size_t packet_length) {
    // Calculate XOR for verification
    uint16_t calculated_xor = calculateXOR(scan_data, packet_length);

    // Convert the check code bytes to a 16-bit value
    uint16_t check_code = (scan_data->check_code[1] << 8) | scan_data->check_code[0];

    // Compare the calculated XOR with the check code
    return (calculated_xor == check_code);
}

/**
 * @brief Handle the scan data processing.
 *
 * This function processes the scan data by handling distances and angles.
 *
 * @param scan_data Pointer to the packet data structure of type X4_ScanData.
 */
void X4_HandleScanData(X4_ScanData *scan_data) {
    X4_HandleScanDataDistances(scan_data);
    X4_HandleScanDataAngles(scan_data);
}

/**
 * @brief Convert a 2-byte little-endian array to a uint16_t value.
 *
 * This helper function takes a pointer to an array of 2 bytes and converts
 * it to a 16-bit unsigned integer, interpreting the bytes as little-endian.
 *
 * @param byte_array Pointer to a 2-byte array containing the data.
 * @return The converted value as a 16-bit unsigned integer.
 */
uint16_t convertBytesToUint16(const uint8_t* byte_array) {
    return (uint16_t)(byte_array[1] << 8 | byte_array[0]);
}

