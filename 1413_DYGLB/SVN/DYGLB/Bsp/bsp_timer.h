#ifndef __BSP_TIMER_H_
#define __BSP_TIMER_H_

#include "types_def.h"
#include "stm32f4xx.h"

/*
    @brief      : TIM2/TIM3 基础定时器初始化
                  - TIM2: 1kHz 软定时tick, 供 softtimer 获取 ms 计数
                  - TIM3: 1ms 中断tick
                  - 定时器时钟 APB1 84MHz, psc=83 分频至 1MHz
    @note       : 中断服务 TIM2_IRQHandler/TIM3_IRQHandler 见 bsp_it.c
*/
void bsp_timer_init(void);

/*
    @brief      : 获取 TIM2 软定时 tick 计数
    @retval     : 上电累计毫秒数
*/
u64 bsp_timer_getms_count(void);

#endif /* __BSP_TIMER_H_ */
