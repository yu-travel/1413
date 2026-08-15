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
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "SEGGER_RTT.h"
#include "usart.h"
#include "gpio.h"
#include "myiic.h"
#include "delay.h"
#include "sys.h"

/* Private includes ----------------------------------------------------------*/
#define AT24C02_DEV_ID              0xAE        // EEPROM…Ë±∏µÿ÷∑

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum {
	MCU1_28V_TSGY=0x0,
	MCU1_28V_KF=0x1,
	MCU2_12V_GSDJ1=0x2,
	MCU2_12V_GSDJ2,
	MCU2_12V_DYGY=0x4,
	MCU3_28V_SFXJ=0x5,
	MCU3_28V_SFPSD,
	MCU3_28V_BQXJ,
	MCU3_28V_QGSJ=0x8,
	MCU4_28V_HJJC1=0x9,
	MCU4_28V_HJJC2,
	MCU4_28V_BF,
	MCU4_28V_WAOXJ=0xc,	
}DEVICE_ID;

#define GSDJ1_DEV_NUM 	(0)
#define GSDJ2_DEV_NUM 	(1)
#define DYGY_DEV_NUM	(2)
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/


/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

#define DEVNUM  3


#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
