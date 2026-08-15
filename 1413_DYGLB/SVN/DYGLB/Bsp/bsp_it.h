#ifndef __BSP_IT_H_
#define __BSP_IT_H_

#include "stm32f4xx.h"

/* Cortex-M4 异常服务函数 */
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

/* 外设中断服务函数 */
void TIM2_IRQHandler(void);
void TIM3_IRQHandler(void);

#endif /* __BSP_IT_H_ */
