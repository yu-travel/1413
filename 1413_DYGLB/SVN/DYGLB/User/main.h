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
#include "delay.h"
#include "sys.h"

/* Private defines -----------------------------------------------------------*/

#define DEVNUM  2

#define TSGY_DEV_NUM	(0)
#define KF_DEV_NUM	(1)

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
