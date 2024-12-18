/**
 * @file shell.c
 * @brief Shell implementation for STM32 microcontroller.
 *
 * This file contains the shell task creation, initialization, and main execution loop.
 * It also includes a basic help command to list available shell functions and their descriptions.
 *
 * @date Oct 11, 2024
 * @author exysta
 */

#include "shell.h"
#include "shell_drv_uart.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

/** @brief Global shell structure instance. */
h_shell_t h_shell;

/** @brief Stack for the shell task. */
static StackType_t task_stack[TASK_SHELL_STACK_SIZE];

/** @brief TCB (Task Control Block) for the shell task. */
static StaticTask_t task_tcb;

/**
 * @brief Stack overflow hook function.
 *
 * This function is called if a stack overflow is detected, printing an error message.
 * It is an application-specific FreeRTOS hook function.
 */
//void vApplicationStackOverflowHook(void)
//{
//	printf("oupsie daisy stack overflow \r\n");
//}

/**
 * @brief Main shell task function.
 *
 * This function runs the shell in an infinite loop, processing commands by calling
 * `shell_run`. It yields to other tasks with a delay to ensure the RTOS remains responsive.
 *
 * @param unused Unused parameter required by FreeRTOS task function signature.
 */
void task_shell(void *unused)
{
	while (1)
	{
		shell_run(&h_shell);  // Main shell processing loop

		// Delay to allow other tasks to run
		vTaskDelay(pdMS_TO_TICKS(100));  // 10 ms delay for yielding
	}

	// Task should never reach this point; delete task if exited
	vTaskDelete(0);
}

/**
 * @brief Creates and initializes the shell task.
 *
 * This function initializes the shell task's TCB and stack and creates the task
 * using FreeRTOS. It sets the task handle in the shell structure for later reference.
 *
 * @param h_shell Pointer to the main shell structure.
 * @return 0 on success, 1 on failure.
 */
int shell_createShellTask(h_shell_t *h_shell)
{
	// Initialize the TCB to zero
	h_shell->h_task = NULL;

	// Create the shell task statically
	h_shell->h_task = xTaskCreateStatic(
			task_shell, "task_shell", TASK_SHELL_STACK_DEPTH,
			NULL, TASK_SHELL_PRIORITY, task_stack, &task_tcb);

	// Check if task creation was successful
	if (h_shell->h_task == NULL)
	{
		return 1;  // Task creation failed
	}
	return 0;
}

/**
 * @brief Help command for the shell.
 *
 * Prints a list of all available shell functions and their descriptions.
 * Iterates through the function list stored in the shell structure and transmits
 * descriptions over UART.
 *
 * @param h_shell Pointer to the main shell structure.
 * @param argc Argument count (unused in this function).
 * @param argv Argument vector (unused in this function).
 * @return 0 on success.
 */
static int sh_help(h_shell_t *h_shell, int argc, char **argv)
{
	int i;
	for (i = 0; i < h_shell->shell_func_list_size; i++)
	{
		int size;
		memset(h_shell->print_buffer, 0, BUFFER_SIZE);
		size = snprintf(h_shell->print_buffer, BUFFER_SIZE, "%s: %s\r\n",
				h_shell->shell_func_list[i].name,
				h_shell->shell_func_list[i].description);

		h_shell->drv_shell.drv_shell_transmit(h_shell->print_buffer, size);
		shell_drv_uart_waitTransmitComplete();  // Wait for transmission to complete
	}

	return 0;
}


/**
 * @brief Initializes the shell.
 *
 * Sets up the shell's function list size, initializes UART transmission and
 * reception functions, and adds a default "help" command.
 *
 * @param h_shell Pointer to the main shell structure.
 */
void shell_init(h_shell_t *h_shell)
{
	h_shell->shell_func_list_size = 0;
	h_shell->drv_shell.drv_shell_receive = shell_drv_uart_receive;
	h_shell->drv_shell.drv_shell_transmit = shell_drv_uart_transmit;

	// Add the default help command
	shell_add(h_shell, "help", sh_help, "Help");
}

/**
 * @brief Adds a command to the shell's function list.
 *
 * Registers a new command by name, function pointer, and description. If the
 * maximum number of commands is reached, the command will not be added.
 *
 * @param h_shell Pointer to the main shell structure.
 * @param name Command name.
 * @param pfunc Pointer to the function that implements the command.
 * @param description Command description.
 * @return 0 on success, -1 if the function list is full.
 */
int shell_add(h_shell_t *h_shell, char *name, shell_func_pointer_t pfunc, char *description)
{
	if (h_shell->shell_func_list_size < SHELL_FUNC_LIST_MAX_SIZE)
	{
		h_shell->shell_func_list[h_shell->shell_func_list_size].name = name;
		h_shell->shell_func_list[h_shell->shell_func_list_size].func = pfunc;
		h_shell->shell_func_list[h_shell->shell_func_list_size].description = description;
		h_shell->shell_func_list_size++;
		return 0;
	}
	return -1;
}

/**
 * @brief Executes a command entered by the user.
 *
 * Parses the command stored in `cmd_buffer`, tokenizes it into arguments, and
 * searches for the command in the function list. If found, executes the command
 * function and returns the result.
 *
 * @param h_shell Pointer to the main shell structure.
 * @return Result of command execution, or -1 if the command is invalid or if
 *         memory allocation fails.
 */
static int shell_exec(h_shell_t *h_shell)
{
	if (h_shell == NULL)
	{
		return -1; // Invalid parameters
	}

	char buf[BUFFER_SIZE];
	char *token;
	char *argv[ARGC_MAX];
	int argc = 0;

	// Safely copy the command buffer
	strncpy(buf, h_shell->cmd_buffer, BUFFER_SIZE - 1);
	buf[BUFFER_SIZE - 1] = '\0'; // Null-terminate to avoid overflow

	// Tokenize the command buffer into arguments
	token = strtok(buf, " ");
	while (token != NULL && argc < ARGC_MAX)
	{
		argv[argc] = strdup(token); // Duplicate each token
		if (argv[argc] == NULL)
		{
			// Memory allocation failed; clean up and return
			for (int j = 0; j < argc; j++)
			{
				free(argv[j]);
			}
			snprintf(h_shell->print_buffer, BUFFER_SIZE, "Error: Memory allocation failed\r\n");
			h_shell->drv_shell.drv_shell_transmit(h_shell->print_buffer, strlen(h_shell->print_buffer));
			shell_drv_uart_waitTransmitComplete();  // Wait for transmission to complete
			return -1;
		}
		argc++; // Increment argument count
		token = strtok(NULL, " ");
	}

	// Check if any command was entered
	if (argc == 0)
	{
		snprintf(h_shell->print_buffer, BUFFER_SIZE, "Error: No command entered\r\n");
		h_shell->drv_shell.drv_shell_transmit(h_shell->print_buffer, strlen(h_shell->print_buffer));
		shell_drv_uart_waitTransmitComplete();  // Wait for transmission to complete
		return -1;
	}

	// Lookup and execute the command
	char *user_func = argv[0]; // First token is the command
	for (int i = 0; i < h_shell->shell_func_list_size; i++)
	{
		if (strcmp(h_shell->shell_func_list[i].name, user_func) == 0)
		{
			// Execute the command
			int result = h_shell->shell_func_list[i].func(h_shell, argc, argv);

			// Clean up dynamically allocated memory
			for (int j = 0; j < argc; j++)
			{
				free(argv[j]);
			}
			return result;
		}
	}
	snprintf(h_shell->print_buffer, BUFFER_SIZE, "Error: No such command %s\r\n",user_func);
	h_shell->drv_shell.drv_shell_transmit(h_shell->print_buffer, strlen(h_shell->print_buffer));
	shell_drv_uart_waitTransmitComplete();  // Wait for transmission to complete

	return -1;

}
/**
 * @brief Backspace sequence for removing characters from the terminal.
 *
 * Used to visually erase the last character by moving the cursor back, printing
 * a space to clear, and moving the cursor back again.
 */
static char backspace[] = "\b \b";

/**
 * @brief Prompt sequence displayed in the terminal for user commands.
 */
static char prompt[] = "> ";

/**
 * @brief Runs the shell in non-blocking mode, handling user input and executing commands.
 *
 * The function displays a prompt, reads characters entered by the user, and processes them.
 * It supports editing with backspace, executing commands with the RETURN key, and provides
 * feedback when no command is entered. `shell_run` is non-blocking and should be called
 * in a loop, returning control to other tasks each time.
 *
 * @param h_shell Pointer to the main shell structure.
 * @return Always returns 0 as it is non-blocking and runs indefinitely.
 */
int shell_run(h_shell_t *h_shell)
{
	static int pos = 0;      /**< Position in the command buffer */
	static int reading = 0;  /**< Indicates if the shell is currently reading input */
	char c;
	int size;

	// State 1: Show prompt if not already reading input
	if (!reading)
	{
		h_shell->drv_shell.drv_shell_transmit(prompt, 2); // Send the prompt
		shell_drv_uart_waitTransmitComplete();  // Wait for prompt transmission to complete
		reading = 1;  // Switch to reading mode
	}

	// State 2: Check for received character (non-blocking)
	if (shell_drv_uart_receive(&c, 1))
	{
		shell_drv_uart_waitReceiveComplete();  // Wait for character reception

		// Process the received character
		switch (c)
		{
		case '\r':  // Process RETURN key
			if (pos > 0)  // Only process if there's something in the buffer
			{
				// Transmit newline to indicate command entry completion
				size = snprintf(h_shell->print_buffer, BUFFER_SIZE, "\r\n");
				h_shell->drv_shell.drv_shell_transmit(h_shell->print_buffer, size);
				shell_drv_uart_waitTransmitComplete();

				// Add NULL terminator to the command buffer and echo the command
				h_shell->cmd_buffer[pos++] = 0;
				size = snprintf(h_shell->print_buffer, BUFFER_SIZE, ":%s\r\n", h_shell->cmd_buffer);
				h_shell->drv_shell.drv_shell_transmit(h_shell->print_buffer, size);
				shell_drv_uart_waitTransmitComplete();

				shell_exec(h_shell);  // Execute the command after input is complete
			}
			else  // If buffer is empty, print an error or ignore
			{
				size = snprintf(h_shell->print_buffer, BUFFER_SIZE, "\r\nError: No command entered\r\n");
				h_shell->drv_shell.drv_shell_transmit(h_shell->print_buffer, size);
				shell_drv_uart_waitTransmitComplete();
			}
			// Reset reading state and buffer
			reading = 0;  // Exit reading state
			pos = 0;      // Reset buffer
			break;

		case '\b':  // Process BACKSPACE key
			if (pos > 0)
			{
				pos--;  // Remove last character from buffer
				h_shell->drv_shell.drv_shell_transmit(backspace, sizeof(backspace) - 1); // Send backspace sequence
				shell_drv_uart_waitTransmitComplete();  // Wait for transmission to complete
				h_shell->cmd_buffer[pos] = 0;  // Null terminate the command buffer after removing char
			}
			break;

		default:  // Handle other characters
			if (pos < BUFFER_SIZE)
			{
				h_shell->drv_shell.drv_shell_transmit(&c, 1);  // Echo the character back to terminal
				shell_drv_uart_waitTransmitComplete();

				h_shell->cmd_buffer[pos++] = c;  // Store the character in buffer
			}
			break;
		}
	}

	return 0;  // Function returns immediately without blocking
}

