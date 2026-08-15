#include "main.h"
#include "delay.h"
#include "types_def.h"

uint32_t dev_Open_counter_time[DEVNUM] = {0};//timer.c ��ʱʹ�õ��豸�����Ƽ���,�������������ؽ���Ǩ��

/*
    @brief      : LED ��ʼ��,PD2
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void led_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
}

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ϵͳ�ж����ȼ�����2
    delay_init(168);    //��ʼ����ʱ����,ϵͳʱ��Ϊ168M
    delay_ms(1500);     //��Դ��������ʱ����
    TRACE_OUT(DEBUG_OUT, "<DYGLB> system init ......\r\n");

    led_init();

    TRACE_OUT(DEBUG_OUT, "<DYGLB> System init completed, enter loop ......\r\n");
    while(1)
    {
        GPIO_SetBits(GPIOD, GPIO_Pin_2);
        delay_ms(500);
        GPIO_ResetBits(GPIOD, GPIO_Pin_2);
        delay_ms(500);
        TRACE_OUT(DEBUG_OUT, "<DYGLB> heartbeat ......\r\n");
    }
}
