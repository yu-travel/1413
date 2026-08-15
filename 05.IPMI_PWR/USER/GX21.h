#ifndef __GX21_H
#define __GX21_H
#include "stm32f4xx.h"

void LM75A_WriteOneByte(u8 I2C_Addr,u8 addr,u8 dt);
u8 LM75A_ReadOneByte(u8 I2C_Addr,u8 addr);
u16 LM75A_Read2Byte(u8 I2C_Addr,u8 addr);
float  Lm75a_get_temp(uint8_t I2C_Addr);


#endif

