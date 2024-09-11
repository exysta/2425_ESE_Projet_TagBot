/*
 * lidar.c
 *
 *  Created on: Sep 11, 2024
 *      Author: exysta
 */

#include "main.h"
#include "stdio.h"
#include "string.h"
//include uart

//#define LIDAR_UART &huart1 // define the UART handle for the LIDAR

#define LIDAR_BUFFER_SIZE 128 // Adjust as necessary
uint8_t lidar_rx_buffer[LIDAR_BUFFER_SIZE]; // Buffer to hold incoming data

void Lidar_Init(void) {
    HAL_UART_Receive_DMA(LIDAR_UART, lidar_rx_buffer, LIDAR_BUFFER_SIZE);
}

#define FRAME_START 0x55AA
#define FRAME_SIZE 22 // Example frame size

typedef struct {
    uint16_t distance;
    uint16_t angle;
} LidarData;

LidarData lidar_data;

void Lidar_ParseFrame(uint8_t* buffer) {
    // Check for the start of the frame
    if ((buffer[0] == 0x55) && (buffer[1] == 0xAA)) {
        // Assuming distance and angle are at specific offsets (depends on the lidar's protocol)
        lidar_data.distance = (buffer[4] | (buffer[5] << 8)); // Example: 16-bit distance
        lidar_data.angle = (buffer[6] | (buffer[7] << 8));     // Example: 16-bit angle
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == LIDAR_UART) {
        // Parse the incoming frame
        Lidar_ParseFrame(lidar_rx_buffer);

        // Process the lidar data, e.g., store or print
        printf("Distance: %u mm, Angle: %u deg\n", lidar_data.distance, lidar_data.angle);

        // Restart the UART reception
        HAL_UART_Receive_DMA(LIDAR_UART, lidar_rx_buffer, LIDAR_BUFFER_SIZE);
    }
}
