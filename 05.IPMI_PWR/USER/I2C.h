#ifndef _iic_H
#define _iic_H

#include "stm32f4xx.h"


#define NST175_ADDR 0x90 //read

//I2C3
#define I2C_Master_SCL GPIO_Pin_8	  //PA8
#define I2C_Master_SDA GPIO_Pin_9	  //PC9
#define I2C_SCL_H GPIO_SetBits(GPIOA,I2C_Master_SCL)
#define I2C_SCL_L GPIO_ResetBits(GPIOA,I2C_Master_SCL)
#define I2C_SDA_H GPIO_SetBits(GPIOC,I2C_Master_SDA)
#define I2C_SDA_L GPIO_ResetBits(GPIOC,I2C_Master_SDA)


uint8_t I2C_Master_BufferWrite(uint32_t I2Cx, uint8_t* pBuffer, uint32_t NumByteToWrite, uint8_t SlaveAddress);
uint8_t I2C_Master_BufferRead(uint32_t I2Cx, uint8_t* pBuffer, uint32_t NumByteToRead, uint8_t SlaveAddress);


//I2C3
void I2C3_Init(void);
void I2C_SDA_OUT(void);
void I2C_SDA_IN(void);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_Ack(void);
void I2C_NAck(void);
u8 I2C_Wait_Ack(void);
void I2C_Send_Byte(u8 txd);
u8 I2C_Read_Byte(u8 ack);


void I2C_CLK_Test(uint32_t I2Cx);



#endif

