#ifndef __MAC_5048_H__
#define __MAC_5048_H__

#include "stm32f4xx.h"

//==== MAC5048 FAULT 引脚宏定义 ====
#define KF2_FAULT_GPIO_PORT     GPIOA
#define KF2_FAULT_GPIO_PIN      GPIO_Pin_15

#define HWXJ3_FAULT_GPIO_PORT   GPIOB
#define HWXJ3_FAULT_GPIO_PIN    GPIO_Pin_0

#define PD_FAULT_GPIO_PORT      GPIOB
#define PD_FAULT_GPIO_PIN       GPIO_Pin_3

#define KF1_FAULT_GPIO_PORT     GPIOB
#define KF1_FAULT_GPIO_PIN      GPIO_Pin_4

#define HJJC1_FAULT_GPIO_PORT   GPIOB
#define HJJC1_FAULT_GPIO_PIN    GPIO_Pin_5

#define HJJC2_FAULT_GPIO_PORT   GPIOB
#define HJJC2_FAULT_GPIO_PIN    GPIO_Pin_6

#define HWXJ2_FAULT_GPIO_PORT   GPIOB
#define HWXJ2_FAULT_GPIO_PIN    GPIO_Pin_7

#define HWXJ1_FAULT_GPIO_PORT   GPIOB
#define HWXJ1_FAULT_GPIO_PIN    GPIO_Pin_8

#define QGSJ_FAULT_GPIO_PORT    GPIOB
#define QGSJ_FAULT_GPIO_PIN     GPIO_Pin_9

#define SFXJ2_FAULT_GPIO_PORT   GPIOB
#define SFXJ2_FAULT_GPIO_PIN    GPIO_Pin_10

#define SFXJ1_FAULT_GPIO_PORT   GPIOB
#define SFXJ1_FAULT_GPIO_PIN    GPIO_Pin_12

#define WAOXJ_FAULT_GPIO_PORT   GPIOB
#define WAOXJ_FAULT_GPIO_PIN    GPIO_Pin_13

#define HJJC3_FAULT_GPIO_PORT   GPIOB
#define HJJC3_FAULT_GPIO_PIN    GPIO_Pin_14

#define DTJ_FAULT_GPIO_PORT     GPIOB
#define DTJ_FAULT_GPIO_PIN      GPIO_Pin_15

// 读取引脚电平简易宏
#define READ_KF2_FAULT()        GPIO_ReadInputDataBit(KF2_FAULT_GPIO_PORT,    KF2_FAULT_GPIO_PIN)
#define READ_HWXJ3_FAULT()      GPIO_ReadInputDataBit(HWXJ3_FAULT_GPIO_PORT,  HWXJ3_FAULT_GPIO_PIN)
#define READ_PD_FAULT()         GPIO_ReadInputDataBit(PD_FAULT_GPIO_PORT,     PD_FAULT_GPIO_PIN)
#define READ_KF1_FAULT()        GPIO_ReadInputDataBit(KF1_FAULT_GPIO_PORT,    KF1_FAULT_GPIO_PIN)
#define READ_HJJC1_FAULT()      GPIO_ReadInputDataBit(HJJC1_FAULT_GPIO_PORT,  HJJC1_FAULT_GPIO_PIN)
#define READ_HJJC2_FAULT()      GPIO_ReadInputDataBit(HJJC2_FAULT_GPIO_PORT,  HJJC2_FAULT_GPIO_PIN)
#define READ_HWXJ2_FAULT()      GPIO_ReadInputDataBit(HWXJ2_FAULT_GPIO_PORT,  HWXJ2_FAULT_GPIO_PIN)
#define READ_HWXJ1_FAULT()      GPIO_ReadInputDataBit(HWXJ1_FAULT_GPIO_PORT,  HWXJ1_FAULT_GPIO_PIN)
#define READ_QGSJ_FAULT()       GPIO_ReadInputDataBit(QGSJ_FAULT_GPIO_PORT,   QGSJ_FAULT_GPIO_PIN)
#define READ_SFXJ2_FAULT()      GPIO_ReadInputDataBit(SFXJ2_FAULT_GPIO_PORT,  SFXJ2_FAULT_GPIO_PIN)
#define READ_SFXJ1_FAULT()      GPIO_ReadInputDataBit(SFXJ1_FAULT_GPIO_PORT,  SFXJ1_FAULT_GPIO_PIN)
#define READ_WAOXJ_FAULT()      GPIO_ReadInputDataBit(WAOXJ_FAULT_GPIO_PORT,  WAOXJ_FAULT_GPIO_PIN)
#define READ_HJJC3_FAULT()      GPIO_ReadInputDataBit(HJJC3_FAULT_GPIO_PORT,  HJJC3_FAULT_GPIO_PIN)
#define READ_DTJ_FAULT()        GPIO_ReadInputDataBit(DTJ_FAULT_GPIO_PORT,    DTJ_FAULT_GPIO_PIN)

// 故障判断宏：返回1=故障(引脚低)，0=正常(引脚高)
#define IS_KF2_FAULT()         ((READ_KF2_FAULT())     ? 0 : 1)
#define IS_HWXJ3_FAULT()       ((READ_HWXJ3_FAULT())   ? 0 : 1)
#define IS_PD_FAULT()          ((READ_PD_FAULT())      ? 0 : 1)
#define IS_KF1_FAULT()         ((READ_KF1_FAULT())     ? 0 : 1)
#define IS_HJJC1_FAULT()       ((READ_HJJC1_FAULT())   ? 0 : 1)
#define IS_HJJC2_FAULT()       ((READ_HJJC2_FAULT())   ? 0 : 1)
#define IS_HWXJ2_FAULT()       ((READ_HWXJ2_FAULT())   ? 0 : 1)
#define IS_HWXJ1_FAULT()       ((READ_HWXJ1_FAULT())   ? 0 : 1)
#define IS_QGSJ_FAULT()        ((READ_QGSJ_FAULT())    ? 0 : 1)
#define IS_SFXJ2_FAULT()       ((READ_SFXJ2_FAULT())   ? 0 : 1)
#define IS_SFXJ1_FAULT()       ((READ_SFXJ1_FAULT())   ? 0 : 1)
#define IS_WAOXJ_FAULT()       ((READ_WAOXJ_FAULT())   ? 0 : 1)
#define IS_HJJC3_FAULT()       ((READ_HJJC3_FAULT())   ? 0 : 1)
#define IS_DTJ_FAULT()         ((READ_DTJ_FAULT())     ? 0 : 1)

/* 电源异常告警结构体 */
typedef struct
{
    /* 12V_1 电源异常 */
    u8 AB_PWR_VCC12V_1;
    /* 12V_2 电源异常 */
    u8 AB_PWR_VCC12V_2;
    /* 3.3V 电源异常 */
    u8 AB_PWR_VCC3V3;

    /*==================== MAC5048 e?fuse硬件FAULT告警 ====================*/
    u8 AB_KF2_FAULT;
    u8 AB_HWXJ3_FAULT;
    u8 AB_PD_FAULT;
    u8 AB_KF1_FAULT;
    u8 AB_HJJC1_FAULT;
    u8 AB_HJJC2_FAULT;
    u8 AB_HWXJ2_FAULT;
    u8 AB_HWXJ1_FAULT;
    u8 AB_QGSJ_FAULT;
    u8 AB_SFXJ2_FAULT;
    u8 AB_SFXJ1_FAULT;
    u8 AB_WAOXJ_FAULT;
    u8 AB_HJJC3_FAULT;
    u8 AB_DTJ_FAULT;

}_pwr_abnormal_t;

/* 告警状态枚举，沿用你原有业务 */
typedef enum
{
    AB_STA_NORMAL       = 0,    /* 正常 */
    AB_STA_UNDERVOLTAGE = 1,    /* 欠压 */
    AB_STA_OVERVOLTAGE  = 2,    /* 过压 */
    AB_STA_FAULT        = 3     /* 硬件故障（MAC5048 FAULT） */
}AB_STATUS_E;

typedef struct
{
    /* 温度 LM75，放大100倍，s16 */
    s16 tempreture;

    /* ADC原始采样值 */
    u16 vol12_1;
    u16 vol12_2;
    u16 vol33;
    u16 cur12_1;
    u16 cur12_2;
    u16 cur33;

    /* 换算后物理量 */
    float v12_1;
    float v12_2;
    float v33;
    float i12_1;
    float i12_2;
    float i33;

    /*==================== 新增：14路 MAC5048 FAULT 故障状态 ====================*/
    /* 0 = 正常；1 = FAULT拉低，MAC5048故障（过流等，经过芯片内部栅极延时） */
    u8 kf2_fault;
    u8 hwxj3_fault;
    u8 pd_fault;
    u8 kf1_fault;
    u8 hjjc1_fault;
    u8 hjjc2_fault;
    u8 hwxj2_fault;
    u8 hwxj1_fault;
    u8 qgsj_fault;
    u8 sfxj2_fault;
    u8 sfxj1_fault;
    u8 waoxj_fault;
    u8 hjjc3_fault;
    u8 dtj_fault;

    /* 电源异常告警结构体，沿用你原来的 _pwr_abnormal_t */
    _pwr_abnormal_t abnormal;

} _board_monitor_t;

void MAC5048_init(void);

void sysmon_data_get(_board_monitor_t *sysmon);




#endif

