#include  "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "led.h"
void err_init(void)
{
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);

    GPIO_InitTypeDef gpio_init_struct;
    gpio_init_struct.GPIO_Pin=GPIO_Pin_5;
    gpio_init_struct.GPIO_Mode=GPIO_Mode_OUT;
    gpio_init_struct.GPIO_OType=GPIO_OType_PP;
    gpio_init_struct.GPIO_Speed=GPIO_High_Speed ;
    gpio_init_struct.GPIO_PuPd=GPIO_PuPd_NOPULL;
    
    GPIO_Init(GPIOB, &gpio_init_struct);
    
    GPIO_ResetBits(GPIOB, GPIO_Pin_5);
    
    
}
