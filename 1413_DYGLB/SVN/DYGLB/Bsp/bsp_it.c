/*
***********************************************************************************************************************
    @brief          : 中断服务函数 (替代旧 User/stm32f4xx_it.c, 定时器中断并入)
                     故障类异常死循环 (HardFault 带 RTT 寄存器诊断), SysTick 为空实现 (delay 为轮询模式)
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

/*
    @brief      : HardFault 异常处理
    @note       : 最小 RTT 诊断输出 (RTT 为轮询写入不依赖中断, 异常态下可用):
                  打印 CFSR/HFSR/BFAR/MMFAR 寄存器值辅助定位故障源后停机;
                  TRACE_OUT 宏来自 types_def.h (→ SEGGER_RTT.h),
                  flag 传 1 保证不受 DEBUG_OUT 开关影响
*/
void HardFault_Handler(void)
{
    TRACE_OUT(1, "HardFault: CFSR=0x%08X HFSR=0x%08X BFAR=0x%08X MMFAR=0x%08X\r\n",
              (u32)SCB->CFSR, (u32)SCB->HFSR, (u32)SCB->BFAR, (u32)SCB->MMFAR);
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
