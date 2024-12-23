/**
 * @file shell.h
 * @brief Header file for shell driver interface and structure definitions.
 *
 * This file provides the interface and structure definitions for the shell
 * implementation on STM32, including command handling, buffer management,
 * and driver transmit/receive functionality.
 *
 * @date Oct 11, 2024
 * @author exysta
 */

#ifndef SHELL_H_
#define SHELL_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "cmsis_os.h"

/** @brief Stack size in bytes for the shell task */
#define TASK_SHELL_STACK_SIZE 1024

/** @brief Stack depth calculated as stack size divided by element size */
#define TASK_SHELL_STACK_DEPTH (TASK_SHELL_STACK_SIZE / sizeof(StackType_t))

/** @brief Priority level for the shell task */
#define TASK_SHELL_PRIORITY 7

/** @brief Maximum number of arguments for shell commands */
#define ARGC_MAX 8

/** @brief Size of buffers used in the shell for command and print operations */
#define BUFFER_SIZE 100

/** @brief Maximum number of shell functions that can be registered */
#define SHELL_FUNC_LIST_MAX_SIZE 64

/**
 * @typedef drv_shell_transmit_t
 * @brief Function pointer type for the shell's transmit driver function.
 *
 * @param pData Pointer to the data buffer to transmit.
 * @param size Size of the data to transmit.
 * @return Status of transmission, typically 0 on success.
 */
typedef uint8_t (*drv_shell_transmit_t)(char *pData, uint16_t size);

/**
 * @typedef drv_shell_receive_t
 * @brief Function pointer type for the shell's receive driver function.
 *
 * @param pData Pointer to the data buffer for receiving data.
 * @param size Size of the data to receive.
 * @return Status of reception, typically 0 on success.
 */
typedef uint8_t (*drv_shell_receive_t)(char *pData, uint16_t size);

/**
 * @typedef shell_func_pointer_t
 * @brief Function pointer type for shell command functions.
 *
 * @param h_shell Pointer to the main shell structure.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return Integer status, typically 0 on success.
 */
struct h_shell_struct;
typedef int (*shell_func_pointer_t)(struct h_shell_struct *h_shell, int argc, char **argv);

/**
 * @struct drv_shell_struct
 * @brief Structure for shell driver functions.
 *
 * Holds function pointers for transmit and receive operations for the shell.
 */
typedef struct drv_shell_struct {
    drv_shell_transmit_t drv_shell_transmit;  /**< Transmit function pointer */
    drv_shell_receive_t drv_shell_receive;    /**< Receive function pointer */
} drv_shell_t;

/**
 * @struct shell_func_struct
 * @brief Structure for individual shell functions.
 *
 * Stores the name, function pointer, and description of each shell command.
 */
typedef struct shell_func_struct {
    char *name;                /**< Name of the shell function */
    shell_func_pointer_t func; /**< Function pointer for the shell function */
    char *description;         /**< Description of the shell function */
} shell_func_t;

/**
 * @struct h_shell_struct
 * @brief Main shell structure for managing shell tasks and commands.
 *
 * Manages shell functionality, including command processing, buffer management,
 * and a list of registered shell functions.
 */
typedef struct h_shell_struct {
    int shell_func_list_size;                          /**< Number of registered shell functions */
    char print_buffer[BUFFER_SIZE];                    /**< Buffer for printing output */
    shell_func_t shell_func_list[SHELL_FUNC_LIST_MAX_SIZE]; /**< List of shell functions */
    char cmd_buffer[BUFFER_SIZE];                      /**< Buffer for storing commands */
    drv_shell_t drv_shell;                             /**< Shell driver structure */
    TaskHandle_t h_task;                               /**< Task handle for the shell task */
} h_shell_t;

extern h_shell_t h_shell; /**< Global shell structure instance */

/**
 * @brief Creates a shell task in the operating system.
 *
 * Initializes and creates a task for managing shell interactions.
 *
 * @return Integer status, typically 0 on success.
 */
int shell_createShellTask();

/**
 * @brief Initializes the shell with specified parameters.
 *
 * @param h_shell Pointer to the shell structure to initialize.
 */
void shell_init(h_shell_t *h_shell);

/**
 * @brief Adds a function to the shell's list of available commands.
 *
 * @param h_shell Pointer to the shell structure.
 * @param name Name of the command.
 * @param pfunc Function pointer for the command function.
 * @param description Description of the command.
 * @return Integer status, typically 0 on success.
 */
int shell_add(h_shell_t *h_shell, char *name, shell_func_pointer_t pfunc, char *description);

/**
 * @brief Runs the shell, processing commands and interacting with the user.
 *
 * @param h_shell Pointer to the shell structure.
 * @return Integer status, typically 0 on success.
 */
int shell_run(h_shell_t *h_shell);

#endif /* SHELL_H_ */
