#ifndef AD7606_DRIVER_H
#define AD7606_DRIVER_H

// 定义ADC通道数量
#define ADC_CHANNEL_NUM     8
#include "stdbool.h"



void Spi3_Gpio_Init(void);
void Spi3_Configuration(void);
void Spi1_Gpio_Init(void) ;
void Spi1_Configuration(void);

void AD7606_Init(void);
void EXTI_Configuration(void) ;
uint16_t SPI3_ReadWrite(uint16_t data);
void AD7606Analog_data_Conversion(void);
void Alldata_sliding_average(bool onoff);

void AD7606_StartConv(void);
void AD7606_Display(void);

#endif
