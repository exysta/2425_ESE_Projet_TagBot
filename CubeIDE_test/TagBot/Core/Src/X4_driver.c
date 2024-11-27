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
	X4_HandleResponse();

}

void X4_StopScan(void)
{
	X4_SendCommand(X4_CMD_STOP_SCAN);
}

void X4_SoftRestart(void)
{
	X4_SendCommand(X4_CMD_SOFT_RESTART);
}

// Function to send the Get Device Information command (A5 90) and handle the response
void X4_GetDeviceInfo()
{
	// Send the device info command
	X4_SendCommand(X4_CMD_GET_INFO);

	// Wait for and handle response
	X4_HandleResponse();
}

// Function to send the Get Device health command (A5 91) and handle the response
void X4_GetDeviceHealth()
{
	// Send the device health command
	X4_SendCommand(X4_CMD_GET_HEALTH);

	// Wait for and handle response
	X4_HandleResponse();
}

// Function to handle Device Info response (0xà4)
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

// Function to handle Device Health response (0x06)
void X4_HandleDeviceHealthResponse(const X4_ResponseMessage* response)
{
	uint8_t status_code = response->content[0];
	uint8_t error_code[2];
	error_code [0] = response->content[1];
	error_code [1] = response->content[2];
}

void X4_HandleScanResponse(const X4_ResponseMessage* response)
{
	X4_ScanData scan_data;

	scan_data.packet_header[0] = response->content[0];
	scan_data.packet_header[1] = response->content[1];
	scan_data.packet_type = response->content[2];
	scan_data.sample_quantity = response->content[3];
	scan_data.start_angle[0] = response->content [4];
	scan_data.start_angle[1] = response->content [5];
	scan_data.end_angle[0] = response->content [6];
	scan_data.end_angle[1] = response->content [7];
	scan_data.check_code[0] = response->content [8];
	scan_data.check_code[1] = response->content [9];

	//one sample is 2 bytes
	scan_data.sample_data =  malloc(scan_data.sample_quantity * sizeof(uint16_t));
	//sample_quantity indicate the number of sample data to be received
	for(int i =0;i<scan_data.sample_quantity;i+=2)
	{
		scan_data.sample_data[i]=response->content[10 + i];
		scan_data.sample_data[i]=response->content[11 + i];

	}
	//free(scan_data.sample_data);
}


void X4_HandleResponse(void) {
	uint8_t raw_data[X4_MAX_RESPONSE_SIZE + X4_RESPONSE_HEADER_SIZE];
	X4_ResponseMessage response;

	// Receive data from the UART (HAL_UART_Receive should be non-blocking or with timeout)
	HAL_StatusTypeDef status = HAL_UART_Receive(&huart4, raw_data, sizeof(raw_data), HAL_TIMEOUT);
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
				break;
			}
		} else if (response.response_mode == X4_RESPONSE_CONTINUOUS_MODE) {
			X4_HandleScanResponse(&response);

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
//calculate distances of every point scanned

void X4_HandleScanDataDistances(X4_ScanData *scan_data)
{

	// Calculate the number of distances to be processed
	int num_distances = scan_data->sample_quantity;

	scan_data->distances =  malloc(num_distances * sizeof(int));


	//each sample has 2 bytes of value the array
	for(int i = 0 ; i < num_distances*2 ; i += 2)
	{
		uint8_t sample[2];
		sample[0] = scan_data->sample_data[i];
		sample[1] = scan_data->sample_data[i + 1];


		uint16_t hex_distance = convertBytesToUint16(sample);
		//the distance is in mm according to the development manual
		//We calucate with int instead of float for better perf
		//Adding + 2 rounds to the nearest integer when dividing by 4
		int mm_distance = (hex_distance + 2)/4;

		// Store the calculated distance in the array
		scan_data->distances [i/2] = mm_distance;
	}
}


void X4_HandleScanDataAngles(X4_ScanData *scan_data)
{

	// Get the number of samples in the frame (LSN)
	uint8_t LSN = scan_data->sample_quantity;
	scan_data->angles =  malloc(LSN * sizeof(float));

	// Step 1: First-level angle analysis
	// Convert the start_angle and end_angle from byte arrays to uint16_t and right-shift by 1
	uint16_t FSA_raw = convertBytesToUint16(scan_data->start_angle);
	uint16_t LSA_raw = convertBytesToUint16(scan_data->end_angle);

	// Calculate AngleFSA and AngleLSA by shifting right by 1 bit and dividing by 64
	float AngleFSA = (float)(FSA_raw >> 1) / 64.0f;  // First Sample Angle
	float AngleLSA = (float)(LSA_raw >> 1) / 64.0f;  // Last Sample Angle

	// Calculate angle difference and handle wrapping (if necessary)
	float angle_diff = AngleLSA - AngleFSA;
	if (angle_diff < 0)
	{
		angle_diff += 360.0f;  // Wrap-around in clockwise direction
	}

	// Step 2: Calculate intermediate angles
	scan_data->angles[0] = AngleFSA;  // First sample angle (FSA)

	for (int i = 1; i < LSN; ++i) {
		scan_data->angles[i] = (angle_diff / (LSN - 1)) * (i - 1) + AngleFSA;
	}

	// Step 3: Second-level angle correction based on distances
	for (int i = 0; i < LSN; ++i) {

		uint16_t distance =	scan_data->distances[i];
		float AngCorrect = 0.0f;

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
		// printf("Angle[%d]: %f degrees\n", i, angles[i]);
	}
}

// Function to calculate the XOR of all bytes in the packet excluding the check code
uint16_t calculateXOR(const X4_ScanData *scan_data, size_t packet_length) {
	uint16_t xor_result = 0;

	// XOR all bytes in the packet except the check code
	for (size_t i = 0; i < (packet_length - 2); ++i) {
		// Use XOR operation on each byte of the packet
		xor_result ^= ((const uint8_t*)scan_data)[i];
	}

	return xor_result;
}

// Function to verify the check code
bool verifyCheckCode(const X4_ScanData *scan_data, size_t packet_length) {
	// Calculate XOR for verification
	uint16_t calculated_xor = calculateXOR(scan_data, packet_length);

	// Convert the check code bytes to a 16-bit value
	uint16_t check_code = (scan_data->check_code[1] << 8) | scan_data->check_code[0];

	// Compare the calculated XOR with the check code
	return (calculated_xor == check_code);
}

void X4_HandleScanData(X4_ScanData *scan_data)
{
	X4_HandleScanDataDistances(scan_data);
	X4_HandleScanDataAngles(scan_data);
}

// Helper function to convert 2-byte little-endian array to uint16_t
uint16_t convertBytesToUint16(const uint8_t* byte_array)
{
	return (uint16_t)(byte_array[1] << 8 | byte_array[0]);
}
