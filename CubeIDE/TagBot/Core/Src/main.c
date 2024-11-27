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
#include "stm32g4xx_it.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "distSensor_driver.h"
#include "ADXL343_driver.h"

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
int test = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int chr)
{
	HAL_UART_Transmit(&huart4, (uint8_t*)&chr, 1, HAL_MAX_DELAY);
	return chr;
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
  /* USER CODE BEGIN 2 */

	printf(" _____________________________\r\n");
	printf("|                             |\r\n");
	printf("|                             |\r\n");
	printf("|  WELCOME ON TAGBOT PROJECT  |\r\n");
	printf("|                             |\r\n");
	printf("|_____________________________|\r\n");

	/* Ce code initialise l'adc en dma*/
	//distSensor_initADC_DMA(&hadc2, ADC_CHANNEL_12);
	//distSensor_initADC_DMA(&hadc2, ADC_CHANNEL_15);

	/* Code init l'accélérometre*/

	while(1 == ADXL343_Init())
	{}
	ADXL343_Configure();
	ADXL343_WriteRegister(ADXL343_REG_INT_MAP, 0x00);			// Enable interruption on pin INT1
	ADXL343_WriteRegister(ADXL343_REG_INT_MAP, 0x40);			// Enable interruption on pin INT1


	calibrateOffsets();
	//ADXL343_DetectTap();


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		// CODE FOR BTN
		int8_t tap_status;
		ADXL343_ReadRegister(ADXL343_REG_INT_SOURCE, &tap_status, 1); //Renvoie la valeur du registre int_source
		HAL_Delay(100);

		///* Ce code permet d allumer la led */
		//		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
		//		HAL_Delay(100);

		///* Ce code n'affiche que les valeurs inférieures à 1000 (la distance est trop élevée) */
		//		uint32_t distance = distSensor_ReadADC_DMA(&hadc2);
		//
		//		if (distance == 1){
		//			printf("error\r\n");
		//			HAL_Delay(100);
		//		}
		//		else{
		//			printf("adv_value : %lu\r\n", distance);
		//			HAL_Delay(100);
		//		}
		//

//		int8_t tap_status;
//		ADXL343_ReadRegister(ADXL343_REG_DUR, &tap_status, 1); //Renvoie la valeur du registre int_source
//		printf(" DUR reg : %i\r\n", tap_status);
//		HAL_Delay(1000);
		/* Code pour l 'accéléromètre*/
//		int16_t x, y, z;
//
//		ADXL343_Read_XYZ(&x, &y, &z);
//		printf("data read x :%i , y: %i, z:%i\r\n", x, y, z);
//		HAL_Delay(100);
//		//ADXL343_DetectTap();
//		//HAL_Delay(1000);
//
//		//HAL_NVIC_SetPendingIRQ(EXTI2_IRQn); // Force l'interruption EXTI2
//		HAL_Delay(1000);

		/* test*/
		//test +=1;


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
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
//	ADXL343_DetectTap();
	if(GPIO_Pin == BNT_CAT_MOUSE_Pin){
		printf("Button pushed\r\n");
	} else if(GPIO_Pin == Accelerometer_INT1_Pin){
		printf("Acc Int1\r\n");
		uint8_t tap_status;
		ADXL343_ReadRegister(ADXL343_REG_INT_SOURCE, &tap_status, 1); //Renvoie la valeur du registre int_source
	} else if(GPIO_Pin == Accelerometer_INT2_Pin){
		printf("Acc Int2\r\n");
		uint8_t tap_status;
		ADXL343_ReadRegister(ADXL343_REG_INT_SOURCE, &tap_status, 1); //Renvoie la valeur du registre int_source
	}
}
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
