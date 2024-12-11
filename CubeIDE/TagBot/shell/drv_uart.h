/**
 * @file drv_uart.h
 * @brief UART driver header file for handling serial transmission in the shell.
 *
 * This file defines functions for transmitting and receiving data over UART,
 * with optional support for interrupt-based mode. It is configured to work with
 * a specified UART device and can be extended for simple or interrupt-driven communication.
 *
 * @date Oct 18, 2024
 * @author exysta
 */

#ifndef DRV_UART_H_
#define DRV_UART_H_

/** @brief UART device instance used for transmission and reception. */
#define UART_DEVICE huart4

/** @brief Enable UART mode for interrupt-based transmission and reception */
//#define DRV_UART_MODE_SIMPLE
#define DRV_UART_MODE_INTERRUPT

#ifdef DRV_UART_MODE_INTERRUPT
// Variable can be used for receiving characters in interrupt mode
// char received_char;
#endif

/**
 * @brief Receives data over UART.
 *
 * This function receives a specified number of bytes over UART and stores them in the provided data buffer.
 *
 * @param pData Pointer to the buffer where received data will be stored.
 * @param size Number of bytes to receive.
 * @return Status of the receive operation, typically 0 on success.
 */
uint8_t drv_uart_receive(char *pData, uint16_t size);

/**
 * @brief Transmits data over UART.
 *
 * This function sends a specified number of bytes over UART from the provided data buffer.
 *
 * @param pData Pointer to the buffer containing data to be transmitted.
 * @param size Number of bytes to transmit.
 * @return Status of the transmit operation, typically 0 on success.
 */
uint8_t drv_uart_transmit(char *pData, uint16_t size);

/**
 * @brief Waits until UART reception is complete.
 *
 * This function blocks until the current UART receive operation is complete, if in progress.
 *
 * @return Status of the operation, typically 0 on success.
 */
uint8_t drv_uart_waitReceiveComplete(void);

/**
 * @brief Waits until UART transmission is complete.
 *
 * This function blocks until the current UART transmit operation is complete, if in progress.
 *
 * @return Status of the operation, typically 0 on success.
 */
uint8_t drv_uart_waitTransmitComplete(void);

#endif /* DRV_UART_H_ */
