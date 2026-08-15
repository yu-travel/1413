#ifndef __ADC_H
#define __ADC_H	
#include "sys.h" 
#include "stm32f4xx.h"

#define Channel_Num  6 //9 个通道
#define Sample_Num  10 //采样10次进行平均

void ADC_DMA_Config(void);
u16 ReadADCAverageValue(uint16_t Channel);
#endif 















