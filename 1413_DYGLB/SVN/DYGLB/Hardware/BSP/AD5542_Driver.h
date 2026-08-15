#ifndef __AD5542_DRIVER_H
#define __AD5542_DRIVER_H

#include "stm32f4xx.h"

// 定义 AD5542 芯片配置结构体
typedef struct {
    GPIO_TypeDef* cs_gpio_port;
    uint16_t cs_gpio_pin;
    GPIO_TypeDef* ldac_gpio_port;
    uint16_t ldac_gpio_pin;
} AD5542_ConfigTypeDef;

void AD5542_Init(void);
void AD5542_InitCommon(void);
void AD5542_InitSingle(AD5542_ConfigTypeDef* config);
void AD5542_WriteData(AD5542_ConfigTypeDef* config, uint16_t data);
void AD5542_TriggerLDAC(AD5542_ConfigTypeDef* config);
void AD5542Analog_data_Conversion(void);
void Ad5542_Send_Data(void);
#endif /* __AD5542_DRIVER_H */

