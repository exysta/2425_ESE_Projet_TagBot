/*
 * X4_driver.c
 *
 *  Created on: Sep 11, 2024
 *      Author: exysta
 */

#include <X4_driver.h>
#include "main.h"
#include <string.h>


extern UART_HandleTypeDef huart4;

// Function to send a command to the device
static void X4_SendCommand(uint8_t command)
{
    uint8_t data[2] = {X4_CMD_START, command};
    HAL_UART_Transmit(&huart4, data, 2, HAL_MAX_DELAY);  // Transmit command over UART
}

void X4_StartScan(void)
{
    X4_SendCommand(X4_CMD_START_SCAN);
    // Additional logic to handle sustained responses if needed
}

void X4_StopScan(void)
{
    X4_SendCommand(X4_CMD_STOP_SCAN);
}

void X4_GetDeviceInfo(void)
{
    X4_SendCommand(X4_CMD_GET_INFO);
    // Logic to receive and parse device info
}

void X4_GetHealthStatus(void)
{
    X4_SendCommand(X4_CMD_GET_HEALTH);
    // Logic to receive and parse health status
}

void X4_SoftRestart(void)
{
    X4_SendCommand(X4_CMD_SOFT_RESTART);
}

void X4_HandleResponse(void) {
    uint8_t raw_data[X4_MAX_RESPONSE_SIZE + X4_RESPONSE_HEADER_SIZE];
    X4_ResponseMessage response;

    // Receive data from the UART (HAL_UART_Receive should be non-blocking or with timeout)
    HAL_StatusTypeDef status = HAL_UART_Receive(&huart1, raw_data, sizeof(raw_data), HAL_TIMEOUT);
    if (status == HAL_OK) {
        // Parse the received message
        X4_ParseMessage(raw_data, &response);

        // Handle the parsed response based on its mode and type code
        if (response.response_mode == X4_RESPONSE_SINGLE_MODE) {
            // Process the single response based on the type code
            switch (response.type_code) {
                case 0x90:  // Device info response
                	X4_HandleDeviceInfoResponse(response);
                    break;
                case 0x91:  // Health status response
                    // Parse and handle health status
                    break;
                // Add more case handling as necessary
                default:
                    // Handle unknown type codes
                    break;
            }
        } else if (response.response_mode == X4_RESPONSE_CONTINUOUS_MODE) {
            // For continuous mode (e.g., scan data), keep receiving data
            // You can handle this in an interrupt or DMA mode
        }
    } else {
        // Handle UART receive error
    }
}

// Function to parse a raw message received from X4
void X4_ParseMessage(const uint8_t *raw_data, X4_ResponseMessage *response) {
    // Parse the start sign
    response->start_sign = (raw_data[1] << 8) | raw_data[0];

    if (response->start_sign != X4_RESPONSE_START_SIGN) {
        // Invalid message, handle error
        return;
    }

    // Parse response length (lower 6 bits of the 6th byte) and mode (upper 2 bits)
    response->response_length = raw_data[2] & 0x3F;  // Lower 6 bits
    response->response_mode = (raw_data[2] >> 6) & 0x03;  // Upper 2 bits

    // Parse the type code
    response->type_code = raw_data[3];

    // Copy the content (assuming fixed-length content for simplicity)
    uint32_t content_length = (response->response_mode == X4_RESPONSE_CONTINUOUS_MODE) ?
                               X4_MAX_RESPONSE_SIZE : response->response_length;
    for (int i = 0; i < content_length; i++) {
        response->content[i] = raw_data[4 + i];
    }
}

// Function to send the Get Device Information command (A5 90) and handle the response
void X4_GetDeviceInfo()
{
    uint8_t info_cmd[2] = {0xA5, 0x90};  // Command to get device info

    // Send the device info command
    HAL_UART_Transmit(&huart4, info_cmd, sizeof(info_cmd), HAL_MAX_DELAY);

    // Wait for and handle response
    X4_HandleResponse();
}

// Function to handle Device Info response (0x90)
void X4_HandleDeviceInfoResponse(const X4_ResponseMessage* response) {
    X4_DeviceInfo device_info;
    device_info.model = response->content[0];
    device_info.firmware[0] = response->content[1];  // Major firmware version
    device_info.firmware[1] = response->content[2];  // Minor firmware version
    device_info.hardware_version = response->content[3];

    // Copy serial number
    for (int i = 0; i < X4_SERIAL_NUMBER_SIZE; i++) {
        device_info.serial_number[i] = response->content[4 + i];
    }

    // Log device information or update system state with device_info
}


