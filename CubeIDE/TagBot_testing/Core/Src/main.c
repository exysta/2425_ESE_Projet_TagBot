/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ydlidar_x4.h"
#include "shell.h"
#include "shell_drv_uart.h"


#include <stdbool.h>
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static volatile __YDLIDAR_X4_HandleTypeDef hlidar;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#ifdef SSD1306TEST
const uint8_t garfield_32x32 [] = {
		0xff, 0xff, 0xff, 0xff, 	/*Color of index 0*/
		0xff, 0xff, 0xff, 0xff, 	/*Color of index 1*/

		0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff,
		0xff, 0xdf, 0xdf, 0xff,
		0xff, 0xaf, 0xaf, 0xff,
		0xff, 0x30, 0x67, 0xff,
		0xff, 0x35, 0x67, 0xff,
		0xfe, 0xf7, 0x7b, 0xff,
		0xfc, 0xff, 0xfb, 0xff,
		0xfd, 0xff, 0xfd, 0xff,
		0xfd, 0xcf, 0x9d, 0xff,
		0xf9, 0xdf, 0xbc, 0xff,
		0xf0, 0xcf, 0x98, 0x7f,
		0xf9, 0xf8, 0xf8, 0xff,
		0xf0, 0xfd, 0xf8, 0x7f,
		0xfd, 0xe5, 0xbd, 0xff,
		0xfe, 0xf2, 0x73, 0xcf,
		0xff, 0x2d, 0xe7, 0xd7,
		0xff, 0x80, 0x1f, 0xdb,
		0xff, 0xbf, 0xef, 0xed,
		0xff, 0x7f, 0xf7, 0xed,
		0xfe, 0xff, 0xfb, 0xed,
		0xfe, 0xff, 0xf9, 0xdd,
		0xfc, 0xff, 0xf8, 0xbd,
		0xfc, 0xff, 0xfc, 0x7b,
		0xf8, 0xff, 0xfd, 0x77,
		0xfa, 0xb7, 0x6d, 0x6f,
		0xf2, 0xb7, 0x69, 0x5f,
		0xfb, 0x37, 0x63, 0x5f,
		0xfb, 0x37, 0x66, 0x1f,
		0xfc, 0x12, 0x20, 0x3f,
		0xfc, 0x00, 0x00, 0x7f,
		0xff, 0xff, 0xff, 0xff,
};
__io_put_char
#endif

// Declare your UART handle (make sure it's initialized in your code, usually in main or a separate UART initialization function)
extern UART_HandleTypeDef huart2;  // Example for UART2, modify according to your setup

// The function to send a character to UART
int __io_putchar(int ch)
{
    // Send the character using HAL_UART_Transmit
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);

    return ch;  // Return the character back to the caller
}

void print_lidar_distances(h_shell_t *h_shell,int argc, char **argv)
{
	for(int i = ANGLE_MIN;i < ANGLE_MAX; i++)
	{
		int size;
		memset(h_shell->print_buffer, 0, BUFFER_SIZE);
		size = snprintf(h_shell->print_buffer, BUFFER_SIZE, "%s %d: %d \r\n",
				"angle ",
				i,
				(int)hlidar.scan_response.distance[i]);

		h_shell->drv_shell.drv_shell_transmit(h_shell->print_buffer, size);
		shell_drv_uart_waitTransmitComplete();  // Wait for transmission to complete
	}
}
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
	Error_Handler();
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	SCnSCB->ACTLR |= SCnSCB_ACTLR_DISDEFWBUF_Msk;  // Disable write buffer

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART3_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
	YDLIDAR_X4_Init(&hlidar, &huart3);

//	shell_init(&h_shell);
//	shell_add(&h_shell, "print_dist", print_lidar_distances, "print lidar buffer containing scanned distances");
//	shell_createShellTask(&h_shell);

	vTaskStartScheduler();

  /* USER CODE END 2 */

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV6;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{

	YDLIDAR_X4_HAL_UART_RxHalfCpltCallback(huart,&hlidar);

}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

	shell_drv_uart_HAL_UART_RxCpltCallback(huart);
	YDLIDAR_X4_HAL_UART_RxCpltCallback(huart,&hlidar);

}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	shell_drv_uart_HAL_UART_TxCpltCallback(huart);
}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
