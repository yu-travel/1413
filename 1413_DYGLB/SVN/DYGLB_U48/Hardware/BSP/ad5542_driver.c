/******************************************************************************
 * @copyright
 * Copyright (c) 2025, Chengdu  UCAS Co., Ltd. All rights reserved.
 
 * @file ad5542_driver.c
 *
 * @brief ad5542驱动
 *
 * <pre>
 * 修改记录:
 * 2025-07-04, liangys, First create.
 * </pre>
 ******************************************************************************/
		 
/******************************* Include Files ********************************/
#include "ad5542_driver.h"
#include "stm32f4xx.h"
#include "main.h"
// 定义 CS 和 LDAC 引脚操作宏
#define AD5542_CS_LOW(config)  GPIO_ResetBits((config)->cs_gpio_port, (config)->cs_gpio_pin)
#define AD5542_CS_HIGH(config) GPIO_SetBits((config)->cs_gpio_port, (config)->cs_gpio_pin)
#define AD5542_LDAC_LOW(config)  GPIO_ResetBits((config)->ldac_gpio_port, (config)->ldac_gpio_pin)
#define AD5542_LDAC_HIGH(config) GPIO_SetBits((config)->ldac_gpio_port, (config)->ldac_gpio_pin)


#define AD5542_VREF (2.5f)
// 定义两个 AD5542 芯片的配置
AD5542_ConfigTypeDef ad5542_SFXJ = {
    .cs_gpio_port = GPIOB,
    .cs_gpio_pin = GPIO_Pin_12,
    .ldac_gpio_port = GPIOB,
    .ldac_gpio_pin = GPIO_Pin_11
};

AD5542_ConfigTypeDef ad5542_SFPSD = {
    .cs_gpio_port = GPIOC,
    .cs_gpio_pin = GPIO_Pin_0,
    .ldac_gpio_port = GPIOB,
    .ldac_gpio_pin = GPIO_Pin_10
};

AD5542_ConfigTypeDef ad5542_BQXJ = {
	.cs_gpio_port = GPIOC,
	.cs_gpio_pin = GPIO_Pin_1,
	.ldac_gpio_port = GPIOB,
	.ldac_gpio_pin = GPIO_Pin_9
};

AD5542_ConfigTypeDef ad5542_QGSJ = {
	.cs_gpio_port = GPIOC,
	.cs_gpio_pin = GPIO_Pin_2,
	.ldac_gpio_port = GPIOB,
	.ldac_gpio_pin = GPIO_Pin_8
};
uint16_t limit_Ad5524_Data[DEVNUM]={0};

extern uint16_t limit_Current_Data[DEVNUM];
/**
  * @brief  初始化 SPI2 相关的 GPIO 和 SPI2，这部分对所有 AD5542 芯片是公共的
  * @param  None
  * @retval None
  */
void AD5542_InitCommon(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStructure;

    // 使能GPIOB和SPI2时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE); //APB1:42MHZ

    // 配置SPI2的SCK和MOSI引脚
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 将引脚连接到SPI2的复用功能
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);

    // 配置SPI2
    SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;  // 只使用发送模式
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI2, &SPI_InitStructure);

    // 使能SPI2
    SPI_Cmd(SPI2, ENABLE);
}

/**
  * @brief  初始化单个 AD5542 芯片的 CS 和 LDAC 引脚
  * @param  config: 指向 AD5542 芯片配置结构体的指针
  * @retval None
  */
void AD5542_InitSingle(AD5542_ConfigTypeDef* config)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC , ENABLE);
    // 配置 CS 引脚
    GPIO_InitStructure.GPIO_Pin = config->cs_gpio_pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(config->cs_gpio_port, &GPIO_InitStructure);
    AD5542_CS_HIGH(config);

    // 配置 LDAC 引脚
    GPIO_InitStructure.GPIO_Pin = config->ldac_gpio_pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(config->ldac_gpio_port, &GPIO_InitStructure);
    AD5542_LDAC_HIGH(config);
}

/**
  * @brief  向指定的 AD5542 芯片写入数据
  * @param  config: 指向 AD5542 芯片配置结构体的指针
  * @param  data: 要写入的16位数据
  * @retval None
  */
void AD5542_WriteData(AD5542_ConfigTypeDef* config, uint16_t data)
{
    AD5542_CS_LOW(config);  // 使能片选

    // 等待SPI发送缓冲区为空
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI2, data);

    // 等待SPI传输完成
    while ((SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)||(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET));
	delay_us(5);

    AD5542_CS_HIGH(config);  // 禁用片选
}

/**
  * @brief  触发指定 AD5542 芯片将输入寄存器数据加载到 DAC 寄存器
  * @param  config: 指向 AD5542 芯片配置结构体的指针
  * @retval None
  */
void AD5542_TriggerLDAC(AD5542_ConfigTypeDef* config)
{
    for (volatile uint32_t i = 0; i < 10; i++);
    AD5542_LDAC_LOW(config);
    // 短暂延时确保信号有效
    for (volatile uint32_t i = 0; i < 100; i++);
    AD5542_LDAC_HIGH(config);
}

/**
  * @brief  将目标输出电压转换为AD5542所需的16位数据
  * @param  voltage: 目标输出电压，单位：V
  * @retval 转换后的16位数据
  */
  uint16_t AD5542_VoltageToData(float voltage)
  {
      // 限制电压范围，避免超出AD5542输出能力
      if (voltage < 0) {
          voltage = 0;
      } 
      else if (voltage > AD5542_VREF)
      {
          voltage = AD5542_VREF;
      }
      return (uint16_t)((voltage * 65536.0f) / AD5542_VREF);
  }

/******************************************************************************
 * 函 数 名：AD5542_Init
 *
 * 函数说明: AD5542设备初始化
 * 参数说明: 
 * 输入参数: 
 * 输出参数: 
 * 返 回 值: 
 * 
 * 修改记录：
 * --------------------
 *	 
 ******************************************************************************/
void AD5542_Init()
{
	// 初始化公共部分，即 SPI2 相关配置
	AD5542_InitCommon();
	// 初始化单个 AD5542 芯片
	AD5542_InitSingle(&ad5542_SFXJ);
	AD5542_InitSingle(&ad5542_SFPSD);
	AD5542_InitSingle(&ad5542_BQXJ);
	AD5542_InitSingle(&ad5542_QGSJ);
}


/******************************************************************************
 * 函 数 名：AD5542Analog_data_Conversion
 *
 * 函数说明: 收到的限流电流值，转换成发送给5542的数据值。limit_Current_Data单位mA
 * 参数说明: 
 * 输入参数: limit_Current_Data:为fpga传过来的数据，单位mA
 * 输出参数: 
 * 返 回 值: 
 * 
 * 修改记录：
 * --------------------
 *20250722:limit_Current_Data单位由A改为mA,limit_Current_Data[i]*0.125->limit_Current_Data[i]*0.125/1000
 *注意：temp为浮点型计算时，整数后面要带小数点，否则会按整数计算。
 ******************************************************************************/
void AD5542Analog_data_Conversion()
{
	int i =0; 
	double temp;
	for (i = 0;i < DEVNUM; i++)
    {
		switch (i)
		{
			case 0:
				temp = (limit_Current_Data[i]/1000.0)*0.5;  //temp：为4001芯片限流limit_Current_Data，所需要提供的电压，单位V
				limit_Ad5524_Data[SFXJ_DEV_NUM]=temp*26214.0;
				LYSprintf(LYSDEBUG3, "********	limit_Ad5524_Data[0]	=	%d	**************\r\n",limit_Ad5524_Data[i]);
				break;
			case 1:
				temp = (limit_Current_Data[i]/1000.0)*0.5;
				limit_Ad5524_Data[SFPSD_DEV_NUM]=temp*26214;
				LYSprintf(LYSDEBUG3, "********	limit_Ad5524_Data[1]	=	%d	**************\r\n",limit_Ad5524_Data[i]);
				break;		
			case 2:
				temp = (limit_Current_Data[i]/1000.0)*0.5;
				limit_Ad5524_Data[BQXJ_DEV_NUM]=temp*26214;
				LYSprintf(LYSDEBUG3, "********	limit_Ad5524_Data[2]	=	%d	**************\r\n",limit_Ad5524_Data[i]);				
				break;	
			case 3:
				temp = (limit_Current_Data[i]/1000.0)*0.5;
				limit_Ad5524_Data[QGSJ_DEV_NUM]=temp*26214;
				LYSprintf(LYSDEBUG3, "********	limit_Ad5524_Data[3]	=	%d	**************\r\n",limit_Ad5524_Data[i]);
				break;	
			default:
				LYSprintf(LYSDEBUG4, "WARNING!!!!!!!  FATAL ERROE,PLEASE CHECK CODE\r\n");				
				break;				
		}
    }
	
}

/******************************************************************************
 * 函 数 名：Ad5542_Send_Data
 *
 * 函数说明: 向5542发送数据
 * 参数说明: 
 * 输入参数: 
 * 输出参数: 
 * 返 回 值: 
 * 
 * 修改记录：
 * --------------------
 *	 
 ******************************************************************************/
void Ad5542_Send_Data()
{
#if 1
	static u32 i=0;
	u32 temp=0;
	i++;
	temp=i%DEVNUM;
	switch(temp)
	{
		case 0:
			//SFXJ发送数据
			AD5542_WriteData(&ad5542_SFXJ, limit_Ad5524_Data[0]);
			AD5542_TriggerLDAC(&ad5542_SFXJ);
			break;
		case 1:
			//向SFPSD的5542发送数据
			AD5542_WriteData(&ad5542_SFPSD, limit_Ad5524_Data[1]);
			AD5542_TriggerLDAC(&ad5542_SFPSD);
			break;
		case 2:
			//向BQXJ的5542发送数据
			AD5542_WriteData(&ad5542_BQXJ, limit_Ad5524_Data[2]);
			AD5542_TriggerLDAC(&ad5542_BQXJ);
			break;
		case 3:
			//向QGSJ的5542发送数据
			AD5542_WriteData(&ad5542_QGSJ, limit_Ad5524_Data[3]);
		    AD5542_TriggerLDAC(&ad5542_QGSJ);			
			break;
		default :
			LYSprintf(LYSDEBUG4, "WARNING!!!!!!!  FATAL ERROE,PLEASE CHECK CODE\r\n");
			break;
	}
#else
    static float a=1.0f;
	static u32 i=0;
	i++;
	if(i%3==0)
	{
		a=1.5;
	}
	else if(i%3==1)
	{
		a=1.0;
	}
	else if(i%3==2)
	{
		a=3;
	}
	//向SFXJ的5542发送数据
	AD5542_WriteData(&ad5542_SFXJ, a*0.5*26214;
    AD5542_TriggerLDAC(&ad5542_SFXJ);
	//向SFPSD的5542发送数据
	AD5542_WriteData(&ad5542_SFPSD, a*1*26214);
    AD5542_TriggerLDAC(&ad5542_SFPSD);
	//向BQXJ的5542发送数据
	AD5542_WriteData(&ad5542_BQXJ, a*1.5*26214);
    AD5542_TriggerLDAC(&ad5542_BQXJ);
	//向QGSJ的5542发送数据
	AD5542_WriteData(&ad5542_QGSJ, a*2*26214);
    AD5542_TriggerLDAC(&ad5542_QGSJ);

#endif
}

#ifdef SELFTEST
    // 向第一个 AD5542 芯片写入数据并触发 LDAC
    Analog_data_Conversion();
    uint16_t output_data_1 = 0x8000;
    AD5542_WriteData(&ad5542_KF, limit_Ad5524_Data[0]);
    AD5542_TriggerLDAC(&ad5542_KF);

    // 向第二个 AD5542 芯片写入数据并触发 LDAC
    uint16_t output_data_2 = 0x4000;
    AD5542_WriteData(&ad5542_TSGY, limit_Ad5524_Data[1]);
    AD5542_TriggerLDAC(&ad5542_TSGY);
#endif

