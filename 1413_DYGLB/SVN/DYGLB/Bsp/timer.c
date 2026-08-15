/*
***********************************************************************************************************************
    @brief          : ��ʱ����PWM��ʼ��������
    @author         : xiongjinqi
    @date           : 2024/07/21
***********************************************************************************************************************
*/
#include "timer.h"
#include "periph_i2c.h"

 
volatile u64 timer2_count = 0;
volatile u64 timer3_count = 0;


//ͨ�ö�ʱ��2�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��2!Ԥ���ж�����1ms;
void timer2_int_init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);  ///ʹ��TIM3ʱ��
    
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;  //��ʱ����Ƶ
    TIM_TimeBaseInitStructure.TIM_Period    = arr;   //�Զ���װ��ֵ
    TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
    TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
    
    TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure);//��ʼ��TIM3
    
    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE); //������ʱ��3�����ж�
    TIM_Cmd(TIM2,ENABLE); //ʹ�ܶ�ʱ��3
    
    NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn; //��ʱ��3�ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x3; //��ռ���ȼ�1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x3;
    NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

//��ʱ��2�жϷ�����
void TIM2_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET) //����ж�
    {
        // �������ڼ���������ߵ�Ƶ��
        timer2_count++;
    }
    TIM_ClearITPendingBit(TIM2,TIM_IT_Update);  //����жϱ�־λ
}


/*
    @brief      : ��ȡ��ʱ������ֵ
*/
u64 timer_getms_count(void)
{
    return timer2_count;
}

/*����ʱ��λms*/
u64 diffTimer(u64 beforeTimer)
{
	return (timer2_count-beforeTimer);
}

//ͨ�ö�ʱ��3�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��3!
void TIM3_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  ///ʹ��TIM3ʱ��
	
  TIM_TimeBaseInitStructure.TIM_Period = arr; 	//�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);//��ʼ��TIM3
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //������ʱ��3�����ж�
	TIM_Cmd(TIM3,ENABLE); //ʹ�ܶ�ʱ��3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //��ʱ��3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}


extern uint32_t dev_Open_counter_time[DEVNUM];//��λΪms
//��ʱ��3�жϷ�����
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //����ж�
	{
		dev_Open_counter_time[TSGY_DEV_NUM]++;
		dev_Open_counter_time[KF_DEV_NUM]++;
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //����жϱ�־λ
}


#if 0
//TIM14 PWM���ֳ�ʼ�� 
//PWM�����ʼ��
//arr���Զ���װֵ
//psc��ʱ��Ԥ��Ƶ��
void pwm_init(u32 arr, u32 psc)
{
    //�˲������ֶ��޸�IO������
    
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);    //TIM14ʱ��ʹ��
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);   //ʹ��PORTFʱ��
    
    GPIO_InitStructure.GPIO_Pin     = PWM_CTRL1_Pin|PWM_CTRL2_Pin|PWM_CTRL3_Pin|PWM_CTRL4_Pin;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;       //���ù���
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;  //�ٶ�100MHz
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;      //���츴�����
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;       //����
    GPIO_Init(GPIOB,&GPIO_InitStructure);              //��ʼ��PF9

    TIM_TimeBaseStructure.TIM_Prescaler = psc;  //��ʱ����Ƶ
    TIM_TimeBaseStructure.TIM_Period    = arr;   //�Զ���װ��ֵ
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);//��ʼ����ʱ��14
    TIM_TimeBaseInit(TIM12,&TIM_TimeBaseStructure);//��ʼ����ʱ��14
    
    //��ʼ��TIM14 Channel1 PWMģʽ
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1; //ѡ��ʱ��ģʽ:TIM������ȵ���ģʽ2
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_Low; //�������:TIM����Ƚϼ��Ե�
    TIM_OCInitStructure.TIM_Pulse       = (arr/2);  // pwm�����ʼռ�ձ�50%
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);  
    TIM_OC1Init(TIM12, &TIM_OCInitStructure);  
    
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);  //ʹ��TIM14��CCR1�ϵ�Ԥװ�ؼĴ���
    TIM_OC1PreloadConfig(TIM12, TIM_OCPreload_Enable);  //ʹ��TIM14��CCR1�ϵ�Ԥװ�ؼĴ���

    TIM_ARRPreloadConfig(TIM3,ENABLE);//ARPEʹ��
    TIM_ARRPreloadConfig(TIM12,ENABLE);//ARPEʹ��
    
    TIM_Cmd(TIM3, ENABLE);  //ʹ��TIM14
    TIM_Cmd(TIM12, ENABLE);  //ʹ��TIM14
}  

void pwm_config_set(u8 duty)
{
    u32 pwm_reload = 0;

    //pwm_reload = duty*N/100;
    TIM_SetCompare1(TIM12, pwm_reload);	//�޸ıȽ�ֵ���޸�ռ�ձ�
    TIM_SetCompare1(TIM3, pwm_reload);	//�޸ıȽ�ֵ���޸�ռ�ձ�
}

#endif



