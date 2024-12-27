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
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32g4xx_hal.h"
#include "semphr.h"
#include "X4LIDAR_driver.h"
#include "DCMotor_driver.h"
#include "SSD1306.h"
#include "SSD1306_fonts.h"
#include "shell.h"
#include "RobotStrategy.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DISTANCE_SENSOR_ADC_BUFFER_SIZE 10
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
X4LIDAR_handle_t X4LIDAR_handle;
DualDrive_handle_t DualDrive_handle;
__TARGET_HandleTypeDef Target_Handle;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch)
{
	// Send the character using HAL_UART_Transmit
	HAL_UART_Transmit(&huart4, (uint8_t*) &ch, 1, HAL_MAX_DELAY);

	return ch;  // Return the character back to the caller
}

void print_lidar_distances(h_shell_t *h_shell, int argc, char **argv)
{
	for (int i = MIN_ANGLE; i < MAX_ANGLE; i++)
	{
		printf("%s %d: %f \r\n",
				"angle ", i,  X4LIDAR_handle.scan_data.distances[i]);
	}
}

void print_motor_speed(h_shell_t *h_shell, int argc, char **argv)
{
	uint32_t speed_left = DualDrive_handle.motor_left.encoder.measured_rpm;
	uint32_t speed_right = DualDrive_handle.motor_right.encoder.measured_rpm;

	printf("speed left = %lu \r\n", speed_left);
	printf("speed right = %lu \r\n", speed_right);
}



void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
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
	//to disable timer in debug mode

	//to disable timer in debug mode

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
  MX_UART4_Init();
  MX_SPI1_Init();
  MX_TIM4_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_USART3_UART_Init();
  MX_I2C1_Init();
  MX_TIM6_Init();
  MX_TIM8_Init();
  /* USER CODE BEGIN 2 */

	//**********************************************************
	//For distance sensors
//	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
//	HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
//	uint32_t distance_sensor_adc1_buffer [DISTANCE_SENSOR_ADC_BUFFER_SIZE];
//	uint32_t distance_sensor_adc2_buffer [DISTANCE_SENSOR_ADC_BUFFER_SIZE];
//
//	HAL_ADC_Start_DMA(&hadc1, &distance_sensor_adc1_buffer, DISTANCE_SENSOR_ADC_BUFFER_SIZE);
//	HAL_ADC_Start_DMA(&hadc1, &distance_sensor_adc2_buffer, DISTANCE_SENSOR_ADC_BUFFER_SIZE);
//
//	HAL_TIM_Base_Start(&htim6); // trigger pour lancer conversion sharp sensors
	//**********************************************************
	//for motors
//	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
//	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
//	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,
//			2000);
//	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2,
//			0);
//	HAL_Delay(500);
//	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,
//			3000);
//	HAL_Delay(500);
//	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,
//			4000);
//	HAL_Delay(500);
//	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,
//			5000);
//	HAL_Delay(500);
//	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,
//			6000);
//	DCMotor_Init(&DualDrive_handle);
//	DCMotor_SetSpeed(&DualDrive_handle.motor_right, 60, POSITIVE_ROTATION);

	//**********************************************************
	DCMotor_CreateTask(&DualDrive_handle);

	//**********************************************************
	// Init SCREEN OLED
//	if(HAL_OK == SCREEN_SSD1306_Init(&hscreen1, &hi2c1))
//	{
//		SCREEN_SSD1306_DrawBitmap(&hscreen1, Nyan_115x64px, 115, 64, White);
//		//SCREEN_SSD1306_DrawBitmap(&hscreen1, Jerry_50x64px, 120, 64, White);
//		SCREEN_SSD1306_Update_Screen(&hscreen1);
//	}
//	//**********************************************************
	//LIDAR

	X4LIDAR_create_task(&X4LIDAR_handle);

	//**********************************************************
	printf("test \r\n");
	shell_init(&h_shell);
	shell_add(&h_shell, "print_dist", print_lidar_distances,
			"print lidar buffer containing scanned distances");
	shell_add(&h_shell, "print_motor_speed", print_motor_speed,
			"print_motor_speed");
	shell_createShellTask(&h_shell);
	//**********************************************************
	RobotStrategy_CreateTask();
	//HAL_Delay(20000);

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

	/* Code init l'accélérometre*/

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
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
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

	X4LIDAR_HAL_UART_RxHalfCpltCallback(huart,&X4LIDAR_handle);

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

	shell_drv_uart_HAL_UART_RxCpltCallback(huart);
	X4LIDAR_HAL_UART_RxCpltCallback(huart,&X4LIDAR_handle);

}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	shell_drv_uart_HAL_UART_TxCpltCallback(huart);
}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM7 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM7) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
	  DCMotor_EncoderCallback(htim ,&DualDrive_handle);


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
