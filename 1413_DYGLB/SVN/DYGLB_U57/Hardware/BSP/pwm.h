#ifndef _PWM_H
#define _PWM_H
#include "sys.h"


//TIM14 PWM部分初始化 
//PWM输出初始化
//arr：自动重装值
//psc：时钟预分频数
void pwm_init(u32 arr,u32 psc);
void pwm_config_set(u8 duty);


#endif
