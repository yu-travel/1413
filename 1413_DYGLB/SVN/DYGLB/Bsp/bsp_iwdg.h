#ifndef __BSP_IWDG_H_
#define __BSP_IWDG_H_

#include "types_def.h"
#include "stm32f4xx_iwdg.h"

/*
    @brief      : 初始化独立看门狗
    @note       : prer=4, rlr=800 -> 超时时间 Tout=((4*2^4)*800)/32 = 1600ms (~1.6s)
                  主循环需在超时前调用 bsp_iwdg_feed() 喂狗
*/
void bsp_iwdg_init(void);

/*
    @brief      : 喂狗 (重载计数器)
*/
void bsp_iwdg_feed(void);

#endif /* __BSP_IWDG_H_ */
