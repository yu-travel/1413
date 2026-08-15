#ifndef _TIMER_H
#define _TIMER_H
#include "types_def.h"
#include "main.h"
#include "sys.h"


#define TIMER_PRESCALER             (84-1)
#define TIM_HZ(hz)                  ((1000000/hz)-1)
#define TIM_1KHZ                    TIM_HZ(1000)
#define TIM_2KHZ                    TIM_HZ(2000)
#define TIM_5KHZ                    TIM_HZ(5000)
#define TIM_10KHZ                   TIM_HZ(10000)
#define TIM_25KHZ                   TIM_HZ(25000)
#define TIM_50KHZ                   TIM_HZ(50000)
#define TIM_DEFAULT                 TIM_25KHZ       // 25Khz下PWM控制比较细腻


//通用定时器2中断初始化
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器2!预计中断周期1ms;
void timer2_int_init(u16 arr,u16 psc);

//通用定时器3中断初始化
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器3!
void timer3_int_init(u16 arr,u16 psc);

u64 timer_getms_count(void);

void pwm_init(u32 arr,u32 psc);
void pwm_config_set(u8 duty);


#endif
