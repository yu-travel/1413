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
#if 0
#include "stmflash.h"
#include "adc.h"
#include "can.h"
#include "i2c.h"
#include "iwdg.h"
#endif
#include "SEGGER_RTT.h"
#include "usart.h"
#include "gpio.h"
#include "types_def.h"
#include "myiic.h"
#include "delay.h"
#include "sys.h"


//#include "ltc2990.h"
//#include "sd5075.h"
#include "gpio_app.h"
#include "adc.h"
//#include "at8591t.h"
//#include "at9555.h"



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


/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

#define LED1_Pin                    GPIO_Pin_8
#define LED1_GPIO_Port              GPIOC
// #define RUN_LED2_Pin                GPIO_PIN_4
// #define RUN_LED2_GPIO_Port          GPIOC

/* 槽位号GPIO定义 */
#define VPX_GA0_Pin                 GPIO_Pin_0
#define VPX_GA0_GPIO_Port           GPIOB
#define VPX_GA1_Pin                 GPIO_Pin_1
#define VPX_GA1_GPIO_Port           GPIOB
#define VPX_GA2_Pin                 GPIO_Pin_8
#define VPX_GA2_GPIO_Port           GPIOB
#define VPX_GA3_Pin                 GPIO_Pin_9
#define VPX_GA3_GPIO_Port           GPIOB
#define VPX_GA4_Pin                 GPIO_Pin_12
#define VPX_GA4_GPIO_Port           GPIOB
#define VPX_GAP_Pin                 GPIO_Pin_13
#define VPX_GAP_GPIO_Port           GPIOB

/* I2C接口定义 */
#define IPMI_I2C1_EN_Pin            GPIO_Pin_6
#define IPMI_I2C1_EN_GPIO_Port      GPIOC

#define IPMI_I2C2_EN_Pin            GPIO_Pin_7
#define IPMI_I2C2_EN_GPIO_Port      GPIOC

#define IPMI_I2C1_READY_Pin         GPIO_Pin_0
#define IPMI_I2C1_READY_GPIO_Port   GPIOC

#define IPMI_I2C2_READY_Pin         GPIO_Pin_1
#define IPMI_I2C2_READY_GPIO_Port   GPIOC

/* 系统复位监测 */
#define SYS_RESET_Pin               GPIO_Pin_15
#define SYS_RESET_GPIO_Port         GPIOB

#define NVMRO_Pin                   GPIO_Pin_14
#define NVMRO_GPIO_Port             GPIOB

#if 0
/* GPIO模拟I2C */
#define GPIO_IIC1_SCL_Pin           GPIO_Pin_0
#define GPIO_IIC1_SCL_GPIO_Port     GPIOA

#define GPIO_IIC1_SDA_Pin           GPIO_Pin_1
#define GPIO_IIC1_SDA_GPIO_Port     GPIOA

#define ADC_IIC1_SCL_Pin            GPIO_Pin_2
#define ADC_IIC1_SCL_GPIO_Port      GPIOA
#define ADC_IIC1_SDA_Pin            GPIO_Pin_3
#define ADC_IIC1_SDA_GPIO_Port      GPIOA

#define ADC_IIC2_SCL_Pin            GPIO_Pin_4
#define ADC_IIC2_SCL_GPIO_Port      GPIOA
#define ADC_IIC2_SDA_Pin            GPIO_Pin_5
#define ADC_IIC2_SDA_GPIO_Port      GPIOA
#endif

/* 其他引脚定义 */
#define XC388_EN_Pin                GPIO_Pin_7
#define XC388_EN_GPIO_Port          GPIOA


#define XCA4001_OUT_Channel         ADC_Channel_12
#define XCA4001_OUT_Pin             GPIO_Pin_2
#define XCA4001_OUT_GPIO_Port       GPIOC

/* 作为输入引脚监测电流是否过流 */
#define XCA4001_Alert_Pin           GPIO_Pin_4
#define XCA4001_Alert_GPIO_Port     GPIOC

#define XCA4001_RESET_Pin           GPIO_Pin_5
#define XCA4001_RESET_GPIO_Port     GPIOC


/* ADC电压监测 */
#define VCC_1V0_Channel             ADC_Channel_0
#define VCC_1V0_Pin                 GPIO_Pin_0
#define VCC_1V0_GPIO_Port           GPIOA

#define VCC_1V2_Channel             ADC_Channel_1
#define VCC_1V2_Pin                 GPIO_Pin_1
#define VCC_1V2_GPIO_Port           GPIOA

#define VCC_1V5_Channel             ADC_Channel_2
#define VCC_1V5_Pin                 GPIO_Pin_2
#define VCC_1V5_GPIO_Port           GPIOA

#define VCC_1V8_Channel             ADC_Channel_3
#define VCC_1V8_Pin                 GPIO_Pin_3
#define VCC_1V8_GPIO_Port           GPIOA

#define VCC_2V5_TST_Channel         ADC_Channel_5
#define VCC_2V5_TST_Pin             GPIO_Pin_5
#define VCC_2V5_TST_GPIO_Port       GPIOA

#define VCC_3V3_TST_Channel         ADC_Channel_6
#define VCC_3V3_TST_Pin             GPIO_Pin_6
#define VCC_3V3_TST_GPIO_Port       GPIOA

/* USER CODE BEGIN Private defines */

#define SOFTWARE_VERSION            0x0100      // 高8位为小数点前，低8位为小数点后; 当前版本V1.01

#define IPMI_MASTER_I2CA_ID         0x63        // IPMI主机I2CA地址
#define IPMI_MASTER_I2CB_ID         0x64        // IPMI主机I2CB地址

#define IPMI_JKKZ_I2CA_ID           0x25        // IPMI转接板I2CA从机地址
#define IPMI_JKKZ_I2CB_ID           0x26        // IPMI转接板I2CB从机地址

#define GX21M15_DEV_ID1             0x90        // 温度传感器设备地址1
#define GX21M15_DEV_ID2             0x9C        // 温度传感器设备地址2
#define AT24C02_DEV_ID              0xAE        // EEPROM设备地址

#define BOARD_FACTOR                100     // 板载扩大因数

#define LIMIT_VOL_VCC0V8            80
#define LIMIT_VOL_VCC1V0            100
#define LIMIT_VOL_VCC1V2            120
#define LIMIT_VOL_VCC1V5            150
#define LIMIT_VOL_VCC1V8            180
#define LIMIT_VOL_VCC2V0            200
#define LIMIT_VOL_VCC2V8            280
#define LIMIT_VOL_VCC3V0            300
#define LIMIT_VOL_VCC3V6            360


/* 异常ID add by xjq */
typedef enum {
    AB_ZJB_EEPROM = 0x70,
    AB_ZKB_EEPROM,
    AB_JKKZ_EEPROM,
    AB_XHCL_EEPROM,
    AB_HTM_EEPROM,
    AB_PWR_EEPROM,
    AB_ZJB_SUB_COMM,
    AB_JKKZ_SUB_COMM,
    AB_XHCL_SUB_COMM,
    AB_HTM_SUB_COMM,
    AB_PWR_SUB_COMM,
    AB_MAX_NULL,
}_ABNORMAL_ID_T;

/* 异常状态 add by xjq */
typedef enum {
                                    /* 暂无过流异常，过流硬件会保护 */
    AB_STA_DEV_OK         = 0x0F,
    AB_STA_OVERVOLTAGE    = 0x10,   /* 过压 */
    AB_STA_UNDERVOLTAGE   = 0x11,   /* 欠压 */
    AB_STA_OPT_FAIL       = 0x12,   /* 操作失败, 指风扇、电源控制操作 */
    AB_STA_EEPROM         = 0x13,   /* EEPROM设备异常 */
    AB_STA_TEMPRETURE     = 0x14,   /* 温度传感器设备异常 */
    AB_STA_DEV_OVER_TEMP  = 0x15,   /* 设备过温(由主控板监控)，风扇最大，温度超过主控控制范围, 严重等级最高 */
    AB_STA_SUB_COMM       = 0x16,   /* 子卡通信异常(IPMI通信) */
}_ABNORMAL_STATUS_T;

typedef struct {
    //u8 AB_PWR_SUB_COMM;       // 通信异常
    u8 AB_ID_JKKZ_TEMPRETURE;   // 温度设备异常
    u8 AB_ID_JKKZ_EEPROM;       // EEPROM异常      
    u8 AB_ID_JKKZ_VCC1V2;       // 电压异常
    u8 AB_ID_JKKZ_VCC1V5;       // 同上
    u8 AB_ID_JKKZ_VCC1V8;       // 同上
    u8 AB_ID_JKKZ_VCC1V0;       // 同上
    u8 AB_ID_JKKZ_VCC2V5;       // 同上
    u8 AB_ID_JKKZ_VCC3V3;       // 同上
    u8 AB_JKKZ_TEMPRETURE;      // 温度设备异常码
    u8 AB_JKKZ_EEPROM;          // EEPROM异常码
    u8 AB_JKKZ_VCC1V2;         // 电压异常码
    u8 AB_JKKZ_VCC1V5;          // 同上
    u8 AB_JKKZ_VCC1V8;          // 同上
    u8 AB_JKKZ_VCC1V0;          // 同上
    u8 AB_JKKZ_VCC2V5;          // 同上
    u8 AB_JKKZ_VCC3V3;          // 同上
}_jkkz_abnormal_t;



typedef enum {
    ID_TEMPRETURE = 0x30,
    ID_VCC1V2,
    ID_VCC1V5,
    ID_VCC1V8,
    ID_VCC1V0,
    ID_VCC2V5,
    ID_VCC3V3,
    ID_VPX12V_CURR,
}_sensor_id_t;


extern _board_monitor_t sysmon;
extern _jkkz_abnormal_t jkkz_abnormal;

void jkkz_abnormal_set(u16 value, u8 adnormal_id);

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
