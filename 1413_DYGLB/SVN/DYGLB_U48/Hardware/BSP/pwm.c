#include "pwm.h"
#include "led.h"
#include "usart.h"



//TIM14 PWM部分初始化 
//PWM输出初始化
//arr：自动重装值
//psc：时钟预分频数
void pwm_init(u32 arr,u32 psc)
{
    //此部分需手动修改IO口设置
    
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);    //TIM14时钟使能
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);   //使能PORTF时钟
    
    GPIO_InitStructure.GPIO_Pin     = PWM_CTRL1_Pin|PWM_CTRL2_Pin|PWM_CTRL3_Pin|PWM_CTRL4_Pin;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;       //复用功能
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;  //速度100MHz
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;      //推挽复用输出
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;       //上拉
    GPIO_Init(GPIOB,&GPIO_InitStructure);              //初始化PF9

    TIM_TimeBaseStructure.TIM_Prescaler =psc;  //定时器分频
    TIM_TimeBaseStructure.TIM_Period    =arr;   //自动重装载值
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);//初始化定时器14
    TIM_TimeBaseInit(TIM12,&TIM_TimeBaseStructure);//初始化定时器14
    
    //初始化TIM14 Channel1 PWM模式
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1; //选择定时器模式:TIM脉冲宽度调制模式2
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_Low; //输出极性:TIM输出比较极性低
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);  
    TIM_OC1Init(TIM12, &TIM_OCInitStructure);  
    
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);  //使能TIM14在CCR1上的预装载寄存器
    TIM_OC1PreloadConfig(TIM12, TIM_OCPreload_Enable);  //使能TIM14在CCR1上的预装载寄存器

    TIM_ARRPreloadConfig(TIM3,ENABLE);//ARPE使能
    TIM_ARRPreloadConfig(TIM12,ENABLE);//ARPE使能
    
    TIM_Cmd(TIM3, ENABLE);  //使能TIM14
    TIM_Cmd(TIM12, ENABLE);  //使能TIM14
}  

void pwm_config_set(u8 duty)
{
    u32 pwm_reload = 0;

    //pwm_reload = duty*N/100;
    TIM_SetCompare1(TIM12, pwm_reload);	//修改比较值，修改占空比
    TIM_SetCompare1(TIM3, pwm_reload);	//修改比较值，修改占空比
}


