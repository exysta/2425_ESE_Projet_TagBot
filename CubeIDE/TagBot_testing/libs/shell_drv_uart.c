/**
 * @file shell_drv_uart.c
 * @brief UART driver implementation for handling serial transmission in the shell.
 *
 * Provides UART receive and transmit functions, supporting both simple (blocking) and
 * interrupt-based (non-blocking) modes. This file also manages task notifications
 * for completion in interrupt mode, using FreeRTOS task handles.
 *
 * @date Oct 18, 2024
 * @author exysta
 */

#include <stdlib.h>
#include "usart.h"
#include "shell_drv_uart.h"
#include "cmsis_os.h"

#ifdef DRV_UART_MODE_SIMPLE
/**
 * @brief Receives data over UART in blocking mode.
 *
 * Uses HAL UART to receive data and blocks until the specified amount of data is received.
 *
 * @param pData Pointer to the buffer where received data will be stored.
 * @param size Number of bytes to receive.
 * @return Number of bytes received, or 0 if an error occurs.
 */
uint8_t shell_drv_uart_receive(char *pData, uint16_t size)
{
    HAL_UART_Receive(&UART_DEVICE, (uint8_t*)pData, size, HAL_MAX_DELAY);
    return size;
}

/**
 * @brief Transmits data over UART in blocking mode.
 *
 * Uses HAL UART to transmit data and blocks until the specified amount of data is transmitted.
 *
 * @param pData Pointer to the buffer containing data to be transmitted.
 * @param size Number of bytes to transmit.
 * @return Number of bytes transmitted, or 0 if an error occurs.
 */
uint8_t shell_drv_uart_transmit(char *pData, uint16_t size)
{
    HAL_UART_Transmit(&UART_DEVICE, (uint8_t*)pData, size, HAL_MAX_DELAY);
    return size;
}
#endif

#ifdef DRV_UART_MODE_INTERRUPT

/** @brief Task handle for receiving notification of UART RX completion. */
static TaskHandle_t uartRxTaskHandle = NULL;

/** @brief Task handle for receiving notification of UART TX completion. */
static TaskHandle_t uartTxTaskHandle = NULL;

/** @brief Flag indicating RX completion status. */
static volatile uint8_t uartRxComplete = 0;

/** @brief Flag indicating TX completion status. */
static volatile uint8_t uartTxComplete = 0;

/**
 * @brief Non-blocking UART Receive function using interrupt mode.
 *
 * Initiates a UART receive operation in interrupt mode, allowing the task to continue
 * and be notified upon completion.
 *
 * @param pData Pointer to the buffer where received data will be stored.
 * @param size Number of bytes to receive.
 * @return Number of bytes expected to be received.
 */
uint8_t shell_drv_uart_receive(char *pData, uint16_t size)
{
    // Register the current task as the one to notify when RX is complete
    uartRxTaskHandle = xTaskGetCurrentTaskHandle();

    // Start UART reception in interrupt mode (non-blocking)
    HAL_UART_Receive_IT(&UART_DEVICE, (uint8_t*)pData, size);

    // Task will check completion via notification, so return immediately
    return size;
}

/**
 * @brief Non-blocking UART Transmit function using interrupt mode.
 *
 * Initiates a UART transmit operation in interrupt mode, allowing the task to continue
 * and be notified upon completion.
 *
 * @param pData Pointer to the buffer containing data to be transmitted.
 * @param size Number of bytes to transmit.
 * @return Number of bytes expected to be transmitted.
 */
uint8_t shell_drv_uart_transmit(char *pData, uint16_t size)
{
    // Register the current task as the one to notify when TX is complete
    uartTxTaskHandle = xTaskGetCurrentTaskHandle();

    // Start UART transmission in interrupt mode (non-blocking)
    HAL_UART_Transmit_IT(&UART_DEVICE, (uint8_t*)pData, size);

    // Task will check completion via notification, so return immediately
    return size;
}


// Function to check if UART receive is complete (blocking until notification received)
/**
 * @brief Waits for UART receive completion in blocking mode.
 *
 * Blocks the calling task until the RX task notification is received, indicating
 * the completion of the UART receive operation.
 *
 * @return Status of receive completion, typically 1 on success.
 */
uint8_t shell_drv_uart_waitReceiveComplete(void)
{
    // Wait for RX task notification (blocking if necessary)
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    return 1;
}

// Function to check if UART transmit is complete (blocking until notification received)
/**
 * @brief Waits for UART transmit completion in blocking mode.
 *
 * Blocks the calling task until the TX task notification is received, indicating
 * the completion of the UART transmit operation.
 *
 * @return Status of transmit completion, typically 1 on success.
 */
uint8_t shell_drv_uart_waitTransmitComplete(void)
{
    // Wait for TX task notification (blocking if necessary)
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    return 1;
}


void shell_drv_uart_HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    // Check if the interrupt was triggered for the correct UART
    if (huart->Instance == UART_DEVICE.Instance)
    {
        // Notify the task waiting for RX completion
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (uartRxTaskHandle != NULL)
        {
            vTaskNotifyGiveFromISR(uartRxTaskHandle, &xHigherPriorityTaskWoken);
            uartRxTaskHandle = NULL;  // Clear task handle after notifying
        }

        // Context switch if necessary
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

}

void shell_drv_uart_HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    // Check if the interrupt was triggered for the correct UART
    if (huart->Instance == UART_DEVICE.Instance)
    {
        // Notify the task waiting for TX completion
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (uartTxTaskHandle != NULL)
        {
            vTaskNotifyGiveFromISR(uartTxTaskHandle, &xHigherPriorityTaskWoken);
            uartTxTaskHandle = NULL;  // Clear task handle after notifying
        }

        // Context switch if necessary
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
#endif
