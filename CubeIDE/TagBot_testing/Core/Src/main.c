/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Authors : CHARLOTTE & MARIE
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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "X4_driver.h"
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
#define ADC_DMA
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */


/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int chr)
{
	HAL_UART_Transmit(&huart2, (uint8_t*)&chr, 1, HAL_MAX_DELAY);
	return chr;
}

uint32_t adc_value = 0;
volatile uint8_t adc_ready = 0; // Flag pour indiquer que la valeur est prête à être afficher

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if (ADC2 == hadc->Instance)
	{
		//adc_ready = 1;
		// Si la valeur ADC est supérieure à 1000 (la distance avec le capteur est élevée, mettre le flag à 1
		if (adc_value < 1000)
		{
			adc_ready = 1;
		}
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
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
  MX_ADC2_Init();
  MX_TIM6_Init();
  MX_TIM1_Init();
  MX_TIM7_Init();
  MX_TIM16_Init();
  /* USER CODE BEGIN 2 */

	//printf("hello\r\n");

#ifdef ADC_DMA

	HAL_ADC_Start_DMA(&hadc2, &adc_value, 1);
	HAL_TIM_Base_Start(&htim6);

#endif


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
		/*uint16_t distance = DistanceSensor_GetDistance();

  // Faire quelque chose avec la distance (par exemple, l'envoyer par UART)
  printf("adc_value: %lu \r\n", distance);
  HAL_Delay(1000);  // Délai de 1000 ms*/

#ifdef ADC

	HAL_ADC_Start(&hadc2);
	HAL_ADC_PollForConversion(&hadc2, HAL_MAX_DELAY);
	adc_value = HAL_ADC_GetValue(&hadc2);

	printf("adc_value %lu\r\n", adc_value);
	HAL_Delay(500);
#endif

#ifdef ADC_DMA

	// Vérifier si une nouvelle valeur ADC est prête et que le flag est actif
	if (adc_ready == 1)
	{

		printf("adc_value %lu\r\n", adc_value);
		// Réinitialiser le flag après l'affichage
		adc_ready = 0;
	}

	HAL_Delay(500);
#endif


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
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM8 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM8) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
	/* USER CODE BEGIN Callback 1 */
	if (htim->Instance == TIM7) // calcul de la rampe
	{
		if(pwm_handle.interrupt_counter < RAMP_TIME - 1) //on update la valeur de pulse chaque miliseconde
		{
			pwm_handle.intermediate_pulse1 = pwm_handle.previous_pulse1 + (pwm_handle.pulse1 - pwm_handle.previous_pulse1)  * (pwm_handle.interrupt_counter+1)/RAMP_TIME ;
			pwm_handle.intermediate_pulse2 = pwm_handle.previous_pulse2 + (pwm_handle.pulse2 - pwm_handle.previous_pulse2)  * (pwm_handle.interrupt_counter+1)/RAMP_TIME ;
			pwm_handle.intermediate_pulse3 = pwm_handle.previous_pulse3 + (pwm_handle.pulse3 - pwm_handle.previous_pulse3)  * (pwm_handle.interrupt_counter+1)/RAMP_TIME ;
			pwm_handle.intermediate_pulse4 = pwm_handle.previous_pulse4 + (pwm_handle.pulse4 - pwm_handle.previous_pulse4)  * (pwm_handle.interrupt_counter+1)/RAMP_TIME ;

			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,pwm_handle.intermediate_pulse1);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2,pwm_handle.intermediate_pulse2);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3,pwm_handle.intermediate_pulse3);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4,pwm_handle.intermediate_pulse4);

			pwm_handle.interrupt_counter++;
		}
	}

    if (htim == &htim16) { //on lance le calcul de vitesse toutes les secondes
        calculate_motor_speed();

    }
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
