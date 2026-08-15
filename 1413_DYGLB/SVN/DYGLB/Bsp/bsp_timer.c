/*
***********************************************************************************************************************
    @brief          : 基础定时器 TIM2/TIM3 初始化 (软定时tick)
    @author         : xiongjinqi
    @date           : 2024/07/21
***********************************************************************************************************************
*/
#include "bsp_timer.h"

#define TIMER_PSC_1MHZ      (84-1)      /* APB1 84MHz -> 1MHz 计数频率 */
#define TIMER_ARR_1MS       (1000-1)    /* 1MHz 计数下 1ms (1kHz) */

volatile u64 timer2_count = 0;  /* TIM2 软定时tick计数, 单位ms */
volatile u64 timer3_count = 0;  /* TIM3 1ms tick计数 */

/*
    @brief      : TIM2 中断初始化 (1kHz 软定时tick)
    @note       : Tout = ((arr+1)*(psc+1))/Ft us, Ft=84MHz
*/
static void timer2_int_init(u16 arr, u16 psc)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
    TIM_TimeBaseInitStructure.TIM_Period    = arr;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;

    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*
    @brief      : TIM3 中断初始化 (1ms tick)
*/
static void timer3_int_init(u16 arr, u16 psc)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseInitStructure.TIM_Period = arr;
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;

    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);

    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM3, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*
    @brief      : 基础定时器初始化入口
                  TIM2: 1kHz 软定时tick / TIM3: 1ms tick
*/
void bsp_timer_init(void)
{
    timer2_int_init(TIMER_ARR_1MS, TIMER_PSC_1MHZ);
    timer3_int_init(TIMER_ARR_1MS, TIMER_PSC_1MHZ);
}

/*
    @brief      : 获取 TIM2 软定时 tick 计数 (单位ms)
*/
u64 bsp_timer_getms_count(void)
{
    return timer2_count;
}
