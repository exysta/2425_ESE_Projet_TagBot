/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Left_Motor_FWD_Pin GPIO_PIN_0
#define Left_Motor_FWD_GPIO_Port GPIOA
#define Left_Motor_REV_Pin GPIO_PIN_1
#define Left_Motor_REV_GPIO_Port GPIOA
#define Accelerometer_SCK_Pin GPIO_PIN_5
#define Accelerometer_SCK_GPIO_Port GPIOA
#define Accelerometer_MISO_Pin GPIO_PIN_6
#define Accelerometer_MISO_GPIO_Port GPIOA
#define Accelerometer_MOSI_Pin GPIO_PIN_7
#define Accelerometer_MOSI_GPIO_Port GPIOA
#define Accelerometer_CS_Pin GPIO_PIN_4
#define Accelerometer_CS_GPIO_Port GPIOC
#define LIDAR_Input_Pin GPIO_PIN_10
#define LIDAR_Input_GPIO_Port GPIOC
#define LIDAR_Output_Pin GPIO_PIN_11
#define LIDAR_Output_GPIO_Port GPIOC
#define Right_Motor_FWD_Pin GPIO_PIN_6
#define Right_Motor_FWD_GPIO_Port GPIOB
#define Right_Motor_REV_Pin GPIO_PIN_7
#define Right_Motor_REV_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
