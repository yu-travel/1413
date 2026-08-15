#ifndef _TIMER_H
#define _TIMER_H
#include "types_def.h"
#include "sys.h"


#define TIMER_PRESCALER             (84-1)
#define TIM_HZ(hz)                  ((1000000/hz)-1)
#define TIM_1KHZ                    TIM_HZ(1000)
#define TIM_2KHZ                    TIM_HZ(2000)
#define TIM_5KHZ                    TIM_HZ(5000)
#define TIM_10KHZ                   TIM_HZ(10000)
#define TIM_25KHZ                   TIM_HZ(25000)
#define TIM_50KHZ                   TIM_HZ(50000)
#define TIM_100KHZ                  TIM_HZ(100000)
#define TIM_200KHZ                  TIM_HZ(200000)

#define TIM_DEFAULT                 TIM_25KHZ       // 25Khz��PWM���ƱȽ�ϸ��


//ͨ�ö�ʱ��2�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��2!Ԥ���ж�����1ms;
void timer2_int_init(u16 arr,u16 psc);

//ͨ�ö�ʱ��3�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��3!
void TIM3_Int_Init(u16 arr,u16 psc);

u64 timer_getms_count(void);

void pwm_init(u32 arr,u32 psc);
void pwm_config_set(u8 duty);


#endif
