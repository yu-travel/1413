#include "stm32f4xx.h"
#include "I2C.h"
#include  "delay.h"


/*************************IIC3采集数据函数*****************************************/
void I2C3_Init(void)
{
	/* enable GPIOA|C clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);  //PC9  SDA
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);  //PA8  SCL
    
    GPIO_InitTypeDef gpioc_init_struct;
    gpioc_init_struct.GPIO_Pin=GPIO_Pin_9;
    gpioc_init_struct.GPIO_Mode=GPIO_Mode_OUT;
    gpioc_init_struct.GPIO_OType=GPIO_OType_PP;
    gpioc_init_struct.GPIO_Speed=GPIO_Fast_Speed ;
    gpioc_init_struct.GPIO_PuPd=GPIO_PuPd_UP;
    
    GPIO_Init(GPIOC, &gpioc_init_struct);
    
    GPIO_InitTypeDef gpioa_init_struct;
    gpioa_init_struct.GPIO_Pin=GPIO_Pin_8;
    gpioa_init_struct.GPIO_Mode=GPIO_Mode_OUT;
    gpioa_init_struct.GPIO_OType=GPIO_OType_PP;
    gpioa_init_struct.GPIO_Speed=GPIO_Fast_Speed ;
    gpioa_init_struct.GPIO_PuPd=GPIO_PuPd_UP;
   
    GPIO_Init(GPIOA, &gpioa_init_struct);

	I2C_SCL_H;
	I2C_SDA_H;	
	
}


void I2C_SDA_OUT(void)
{
	GPIO_InitTypeDef gpioc_init_struct;
    gpioc_init_struct.GPIO_Pin=GPIO_Pin_9;
    gpioc_init_struct.GPIO_Mode=GPIO_Mode_OUT;
    gpioc_init_struct.GPIO_OType= GPIO_OType_PP;
    gpioc_init_struct.GPIO_Speed=GPIO_Fast_Speed ;
    
    GPIO_Init(GPIOC, &gpioc_init_struct);

	
}

/*******************************************************************************
* 函 数 名         : I2C_Sensor_SDA_IN
* 函数功能		     : SDA输入配置	   
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
void I2C_SDA_IN(void)
{
	GPIO_InitTypeDef gpioc_init_struct;
    gpioc_init_struct.GPIO_Pin=GPIO_Pin_9;
    gpioc_init_struct.GPIO_Mode=GPIO_Mode_IN;
    gpioc_init_struct.GPIO_PuPd=GPIO_PuPd_NOPULL;
    gpioc_init_struct.GPIO_Speed=GPIO_Fast_Speed ;
    
    GPIO_Init(GPIOC, &gpioc_init_struct);
   
}

//产生起始信号
void I2C_Start(void)
{
  I2C_SDA_OUT();
	
	I2C_SDA_H;
	I2C_SCL_H;
	delay_us(5);
	I2C_SDA_L;
	delay_us(6);
	I2C_SCL_L;
}

//产生停止信号
void I2C_Stop(void)
{
   I2C_SDA_OUT();

   I2C_SCL_L;
   I2C_SDA_L;
   I2C_SCL_H;
   delay_us(6);
   I2C_SDA_H;
   delay_us(6);
}

//主机产生应答信号ACK
void I2C_Ack(void)
{
   I2C_SCL_L;
   I2C_SDA_OUT();
   I2C_SDA_L;
   delay_us(2);
   I2C_SCL_H;
   delay_us(5);
   I2C_SCL_L;
}

//主机不产生应答信号NACK
void I2C_NAck(void)
{
   I2C_SCL_L;
   I2C_SDA_OUT();
   I2C_SDA_H;
   delay_us(2);
   I2C_SCL_H;
   delay_us(5);
   I2C_SCL_L;
}
//等待从机应答信号
//返回值：1 接收应答失败 0 接收应答成功
u8 I2C_Wait_Ack(void)
{
	u8 tempTime=0;

	I2C_SDA_IN();

	I2C_SDA_H;
	delay_us(1);
	I2C_SCL_H;
	delay_us(1);

	while(PCin(9)) {
		tempTime++;
		if(tempTime>250) {
			I2C_Stop();
			return 1;
		}	 
	}

	I2C_SCL_L;
	return 0;
}
//I2C 发送一个字节
void I2C_Send_Byte(u8 txd)
{
	u8 i=0;

	I2C_SDA_OUT();
	I2C_SCL_L;//拉低时钟开始数据传输

	for(i=0;i<8;i++)
	{
		if((txd&0x80)>0) //0x80  1000 0000
			I2C_SDA_H;
		else
			I2C_SDA_L;

		txd<<=1;
		I2C_SCL_H;
		delay_us(2); //发送数据
		I2C_SCL_L;
		delay_us(2);
	}
}

//I2C 读取一个字节

u8 I2C_Read_Byte(u8 ack)
{
   u8 i=0,receive=0;

   I2C_SDA_IN();
   for(i=0;i<8;i++)
   {
   	I2C_SCL_L;
		delay_us(2);
		I2C_SCL_H;
		receive<<=1;
		if(PCin(9))
		   receive++;
		delay_us(1);	
   }
  if(ack==0)
	   I2C_NAck();
	else
		I2C_Ack();

	return receive;
}

