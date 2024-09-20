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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_CHAT_Pin GPIO_PIN_13
#define LED_CHAT_GPIO_Port GPIOC
#define LED_SOURIS_Pin GPIO_PIN_14
#define LED_SOURIS_GPIO_Port GPIOC
#define BTN_CAT_MOUSE_Pin GPIO_PIN_15
#define BTN_CAT_MOUSE_GPIO_Port GPIOC
#define USER_LED_1_Pin GPIO_PIN_0
#define USER_LED_1_GPIO_Port GPIOA
#define USER_LED_2_Pin GPIO_PIN_1
#define USER_LED_2_GPIO_Port GPIOA
#define Accelerometer_INT1_Pin GPIO_PIN_2
#define Accelerometer_INT1_GPIO_Port GPIOA
#define Accelerometer_INT2_Pin GPIO_PIN_3
#define Accelerometer_INT2_GPIO_Port GPIOA
#define Accelerometer_NSS_Pin GPIO_PIN_4
#define Accelerometer_NSS_GPIO_Port GPIOA
#define Accelerometer_SCK_Pin GPIO_PIN_5
#define Accelerometer_SCK_GPIO_Port GPIOA
#define Accelerometer_MISO_Pin GPIO_PIN_6
#define Accelerometer_MISO_GPIO_Port GPIOA
#define Accelerometer_MOSI_Pin GPIO_PIN_7
#define Accelerometer_MOSI_GPIO_Port GPIOA
#define LIDAR_M_EN_Pin GPIO_PIN_4
#define LIDAR_M_EN_GPIO_Port GPIOC
#define LIDAR_DEV_EN_Pin GPIO_PIN_0
#define LIDAR_DEV_EN_GPIO_Port GPIOB
#define LIDAR_M_SCTR_Pin GPIO_PIN_1
#define LIDAR_M_SCTR_GPIO_Port GPIOB
#define CAPTEUR_DIST_S_Pin GPIO_PIN_2
#define CAPTEUR_DIST_S_GPIO_Port GPIOB
#define LIDAR_INPUT_Pin GPIO_PIN_10
#define LIDAR_INPUT_GPIO_Port GPIOB
#define LIDAR_OUTPUT_Pin GPIO_PIN_11
#define LIDAR_OUTPUT_GPIO_Port GPIOB
#define CAPTEUR_DIST_N_Pin GPIO_PIN_12
#define CAPTEUR_DIST_N_GPIO_Port GPIOB
#define CAPTEUR_DIST_W_Pin GPIO_PIN_14
#define CAPTEUR_DIST_W_GPIO_Port GPIOB
#define CAPTEUR_DIST_E_Pin GPIO_PIN_15
#define CAPTEUR_DIST_E_GPIO_Port GPIOB
#define RIGHT_MOTOR_FWD_Pin GPIO_PIN_8
#define RIGHT_MOTOR_FWD_GPIO_Port GPIOA
#define RIGHT_MOTOR_REV_Pin GPIO_PIN_9
#define RIGHT_MOTOR_REV_GPIO_Port GPIOA
#define LEFT_MOTOR_FWD_Pin GPIO_PIN_10
#define LEFT_MOTOR_FWD_GPIO_Port GPIOA
#define LEFT_MOTOR_REV_Pin GPIO_PIN_11
#define LEFT_MOTOR_REV_GPIO_Port GPIOA
#define STLINK_TX_Pin GPIO_PIN_10
#define STLINK_TX_GPIO_Port GPIOC
#define STLINK_RX_Pin GPIO_PIN_11
#define STLINK_RX_GPIO_Port GPIOC
#define ENCODER_RIGHT_A_Pin GPIO_PIN_4
#define ENCODER_RIGHT_A_GPIO_Port GPIOB
#define ENCODER_RIGHT_B_Pin GPIO_PIN_5
#define ENCODER_RIGHT_B_GPIO_Port GPIOB
#define ENCODER_LEFT_A_Pin GPIO_PIN_6
#define ENCODER_LEFT_A_GPIO_Port GPIOB
#define ENCODER_LEFT_B_Pin GPIO_PIN_7
#define ENCODER_LEFT_B_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
