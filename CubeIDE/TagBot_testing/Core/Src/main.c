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
#include "dma.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "X4_driver.h"
#include "ydlidar_x4.h"

#include "ssd1306.h"
#include "ssd1306_tests.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//#define SSD1306TEST
#define ANGLE_MIN 120
#define ANGLE_MAX 240
#define DISTANCE_MIN 200
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
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
#endif
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	float min_distance = 10000;
	int idx_angle_min_distance;
	uint8_t object_detected = 1;
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
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
	//  X4_SendCommand(X4_CMD_GET_INFO); /**< Command to get device information */
	//  HAL_UART_Receive(&huart3, pData, Size, Timeout)

//	X4_handle_t X4_handle;
//
//	uint8_t data[2] = {X4_CMD_START, X4_CMD_GET_HEALTH}; /**< Array holding the command data */
//	uint8_t raw_data[X4_MAX_RESPONSE_SIZE + X4_RESPONSE_HEADER_SIZE]; /**< Buffer for raw received data */
//	memset(raw_data, 0, sizeof(raw_data));

//	X4_Init(&X4_handle);
//	X4_GetDeviceInfo(&X4_handle);
//	X4_StartScan(&X4_handle);
	YDLIDAR_X4_Init(&hlidar, &huart3);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
		if(hlidar.newData){
			YDLIDAR_X4_Compute_Payload(&hlidar);
			min_distance = 10000;
			for(int idx_angle=ANGLE_MIN; idx_angle<ANGLE_MAX; idx_angle++){
				if((10 < hlidar.scan_response.distance[idx_angle]) &&
						(hlidar.scan_response.distance[idx_angle] < min_distance)){
					idx_angle_min_distance = idx_angle;
					min_distance = hlidar.scan_response.distance[idx_angle];
				}
			}
			if(min_distance < DISTANCE_MIN){
				object_detected = 1;

			}
			else{
				object_detected = 0;
			}

			hlidar.newData = 0;
		}
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

/* USER CODE END 4 */

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
