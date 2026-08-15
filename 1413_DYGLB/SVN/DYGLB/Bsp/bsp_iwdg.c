/*
***********************************************************************************************************************
    @brief          : 独立看门狗 (IWDG) 初始化与喂狗
                     Tout = ((4*2^prer)*rlr)/32 ms
***********************************************************************************************************************
*/
#include "bsp_iwdg.h"

/*
    @brief      : 初始化独立看门狗
    @note       : prer=4, rlr=800 -> 超时时间 ((4*2^4)*800)/32 = 1600ms (~1.6s)
*/
void bsp_iwdg_init(void)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(4);
    IWDG_SetReload(800);
    IWDG_ReloadCounter();
    IWDG_Enable();
}

/*
    @brief      : 喂狗
*/
void bsp_iwdg_feed(void)
{
    IWDG_ReloadCounter();
}
