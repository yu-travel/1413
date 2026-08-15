/**************************************************************************************************
* Copyright (C)   Beijing UCAS Space Technology Co.,Ltd. All Rights Reserved

* FILENAME:       LM75A.c
* Description:    temperature sample  master I2C interface 
*
* Function list: 
*
* Author:         xu.cheng
* Version:        V1.00
* Data:           2018/07/16
* History:
**************************************************************************************************/

#include "stdio.h"
#include "stm32f4xx.h"
#include "I2C.h"
#include "GX21.h"


void LM75A_WriteOneByte(u8 I2C_Addr,u8 addr,u8 dt)
{
	I2C_Start();
	I2C_Send_Byte(I2C_Addr);
	I2C_Wait_Ack();
	I2C_Send_Byte(addr);	//发送数据地址
	I2C_Wait_Ack();
	I2C_Send_Byte(dt);
	I2C_Wait_Ack();
	I2C_Stop();

}


u8 LM75A_ReadOneByte(u8 I2C_Addr,u8 addr)
{
	u8 temp=0;
	I2C_Start();	
	I2C_Send_Byte(I2C_Addr);//10100000
	I2C_Wait_Ack();
	I2C_Send_Byte(addr);
	I2C_Wait_Ack();
	I2C_Start();
	I2C_Send_Byte(I2C_Addr|0x01);//10100001
	I2C_Wait_Ack();
	temp=I2C_Read_Byte(0); //  0   代表 NACK
	I2C_NAck();
	I2C_Stop();	
	return temp;	
	
}
u16 LM75A_Read2Byte(u8 I2C_Addr,u8 addr)
{
	u8 temph=0;
	u8 templ=0;
	u16 data;
	u8 ret = 0;
	
	I2C_Start();	
	I2C_Send_Byte(I2C_Addr);//10100000
	ret = I2C_Wait_Ack();
	//if(ret) {printf("[%s:%d] wait ack err\r\n",__FUNCTION__,__LINE__);}
	
	
	I2C_Send_Byte(addr);
	ret = I2C_Wait_Ack();
	//if(ret) {printf("[%s:%d] wait ack err\r\n",__FUNCTION__,__LINE__);}
	
	
	I2C_Start();
	I2C_Send_Byte(I2C_Addr|0x01);//10100001
	I2C_Wait_Ack();
	
	temph = I2C_Read_Byte(1);
	//I2C_Ack();
	templ = I2C_Read_Byte(1);
	//I2C_NAck();
	I2C_Stop();	
	data=((uint16_t)temph << 8) | templ ;

	return data;	
	
}
//I2C_Addr是器件地址
float  Lm75a_get_temp(uint8_t I2C_Addr)
{
	uint16_t temp_data = 0;
	float  temp_out=0;
	
	temp_data = LM75A_Read2Byte(I2C_Addr,0x00);//0x00是温度寄存器地址

	temp_data = temp_data >> 7;

	if(temp_data & 0x100)  //负温度
	{
		temp_data = ~temp_data + 1;
		temp_data &= 0x1ff;
		temp_data = -temp_data;

	}

	temp_out = temp_data*0.5;
	
	return temp_out;

}
