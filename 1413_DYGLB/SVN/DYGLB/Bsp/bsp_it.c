/*
***********************************************************************************************************************
    @brief          : 中断服务函数 (从 User/stm32f4xx_it.c 与 timer.c 迁移)
                     故障类异常进入死循环, SysTick 为空实现 (delay 为轮询模式)
***********************************************************************************************************************
*/
#include "bsp_it.h"
#include "types_def.h"

extern volatile u64 timer2_count;   /* 定义于 bsp_timer.c */
extern volatile u64 timer3_count;   /* 定义于 bsp_timer.c */

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
    while (1)
    {
    }
}

void MemManage_Handler(void)
{
    while (1)
    {
    }
}

void BusFault_Handler(void)
{
    while (1)
    {
    }
}

void UsageFault_Handler(void)
{
    while (1)
    {
    }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

/*
    @brief      : SysTick 中断服务
    @note       : delay 为轮询模式 (SYSTEM_SUPPORT_OS=0), 无需处理
*/
void SysTick_Handler(void)
{
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/******************************************************************************/

/*
    @brief      : TIM2 更新中断 (1kHz 软定时tick)
*/
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
    {
        timer2_count++;
    }
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}

/*
    @brief      : TIM3 更新中断 (1ms tick)
*/
void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
    {
        timer3_count++;
    }
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
}
