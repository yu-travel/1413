/******************************************************************************
 * @copyright
 * Copyright (c) 2025, Chengdu  UCAS Co., Ltd. All rights reserved.
 
 * @file AC7606.c
 *
 * @brief ac7606采集的使用
 *
 * <pre>
 * 修改记录:
 * 2025-07-03, liangys, First create.
 * </pre>
 ******************************************************************************/
		 
/******************************* Include Files ********************************/

#include "stm32f4xx.h"
#include "AD7606_Driver.h"
#include "main.h"
#include "misc.h"

#define SYSCFG_MEMRMP   (*((volatile uint32_t*) (SYSCFG_BASE + 0x00)))




// 定义AD7606控制引脚
#define AD7606_RESET_PIN    GPIO_Pin_8
#define AD7606_RESET_PORT   GPIOA
#define AD7606_CONVST_PIN   GPIO_Pin_1
#define AD7606_CONVST_PORT  GPIOA
#define AD7606_BUSY_PIN     GPIO_Pin_0
#define AD7606_BUSY_PORT    GPIOA
// 定义片选引脚
#define AD7606_CS_PIN       GPIO_Pin_15
#define AD7606_CS_PORT      GPIOA

// 全局变量，用于存储采集的数据
volatile uint16_t adc_data[ADC_CHANNEL_NUM]={0};
// 全局变量，用于存储转换后的数据，单位mv，mA。
volatile uint16_t analog_data[ADC_CHANNEL_NUM]={0};

// 标记是否收到触发指令
volatile uint8_t trigger_send = 0;



/******************************************************************************
 * 函 数 名：SPI3_GPIO_Init
 *
 * 函数说明: spi3的gpio初始化函数
 * 参数说明: 
 * 输入参数: 
 * 输出参数: 
 * 返 回 值: 
 * 
 * 修改记录：
 * --------------------
 *	 
 ******************************************************************************/
void Spi3_Gpio_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能GPIOA和GPIOC时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC , ENABLE);

	// 关闭JTAG调试功能，释放PA15引脚
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	SYSCFG_MEMRMP |= (0x1 << 25);  // SWJ_CFG = 0b010

    // 配置AD7606控制引脚（包含片选引脚）
    GPIO_InitStructure.GPIO_Pin = AD7606_RESET_PIN | AD7606_CONVST_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(AD7606_RESET_PORT, &GPIO_InitStructure);

    // 配置BUSY引脚为输入
    GPIO_InitStructure.GPIO_Pin = AD7606_BUSY_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(AD7606_BUSY_PORT, &GPIO_InitStructure);

    // 配置SPI3引脚：SCK(PC10), MISO(PC11), MOSI(PC12)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

  
	//配置SPI3片选，当spi为主时cs管脚，配置为输出模式
	GPIO_InitStructure.GPIO_Pin = AD7606_CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  // 片选引脚必须为输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(AD7606_CS_PORT, &GPIO_InitStructure);


    // 将SPI3引脚连接到AF功能
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_SPI3);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_SPI3);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_SPI3);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource15, GPIO_AF_SPI3);

    // 初始化片选引脚为高电平，默认不选中AD7606
    GPIO_SetBits(AD7606_CS_PORT, AD7606_CS_PIN);

}
/******************************************************************************
 * 函 数 名：Spi3_Configuration
 *
 * 函数说明: spi3初始化
 * 参数说明: 
 * 输入参数: 
 * 输出参数: 
 * 返 回 值: 
 * 
 * 修改记录：
 * --------------------
 *	 
 ******************************************************************************/
void Spi3_Configuration(void)
{
    SPI_InitTypeDef SPI_InitStructure;

    // 使能SPI3时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);//APB1时钟为42M

    // 配置SPI3
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;//SPI输出时钟为21M
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI3, &SPI_InitStructure);

    // 使能SPI3
    SPI_Cmd(SPI3, ENABLE);
}



// 初始化AD7606
void AD7606_Init(void) {
    Spi3_Gpio_Init();
    Spi3_Configuration();
    // 复位AD7606
    GPIO_SetBits(AD7606_RESET_PORT, AD7606_RESET_PIN);
    // 保持复位信号一段时间
    for (volatile uint32_t i = 0; i < 1000; i++);
    GPIO_ResetBits(AD7606_RESET_PORT, AD7606_RESET_PIN);
}

// 初始化外部中断
void EXTI_Configuration(void) {
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 使能SYSCFG时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    // 将BUSY引脚连接到外部中断线
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

    // 配置外部中断线
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // 配置NVIC
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

// 通过SPI3发送和接收数据
uint16_t SPI3_ReadWrite(uint16_t data) {
    // 等待发送缓冲区为空
    while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) == RESET);
    // 将数据写入SPI3的数据寄存器，触发数据发送
    SPI_I2S_SendData(SPI3, data);
    // 等待接收缓冲区非空，即有数据接收到
    while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET);
    // 从SPI3的数据寄存器读取接收到的数据
    return SPI_I2S_ReceiveData(SPI3);

}
// 外部中断服务函数
void EXTI0_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
        // 拉低片选信号，选中AD7606
        GPIO_ResetBits(AD7606_CS_PORT, AD7606_CS_PIN);

        // 读取数据
        for (int i = 0; i < ADC_CHANNEL_NUM; i++) {
            adc_data[i] = SPI3_ReadWrite(0x0000);
        }

        // 拉高片选信号，取消选中AD7606
        GPIO_SetBits(AD7606_CS_PORT, AD7606_CS_PIN);

        // 清除中断标志位
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

#define AVERAGE_N	10
uint16_t AD7606ValueBuf[8][AVERAGE_N]={0};	//原始数据
uint32_t AD7606AverageSum[8]={0};			//原始数据和
uint16_t AD7606AverageValueBuf[8]={0};		//均值数据

/******************************************************************************
 * 函 数 名：data_sliding_average
 *
 * 函数说明: 对一个通道采集到的数据进行滑动均值处理
 * 参数说明: 
 * 输入参数: channel:AD7606的通道号 new_value：对应通道号采集到的数据
 * 输出参数: 
 * 返 回 值: 返回当前通道的均值
 * 
 * 修改记录：
 * --------------------
 *	 
 ******************************************************************************/
 static uint16_t data_sliding_average(uint8_t channel,uint16_t new_value)
{
	uint8_t count;
	AD7606AverageSum[channel] -= AD7606ValueBuf[channel][0];
	for( count =0; count <AVERAGE_N-1; count++)
	{
        AD7606ValueBuf[channel][count] = AD7606ValueBuf[channel][count +1] ;
	}
	AD7606ValueBuf[channel][AVERAGE_N-1] = new_value;
    AD7606AverageSum[channel] += AD7606ValueBuf[channel][AVERAGE_N-1];
    return(AD7606AverageSum[channel]/(AVERAGE_N));
}
 

/******************************************************************************
 * 函 数 名：Alldata_sliding_average
 *
 * 函数说明: 对所有通道进行滑动均值处理
 * 参数说明: 
 * 输入参数: onoff:1打开 0,关闭
 * 输出参数: 
 * 返 回 值: 
 * 
 * 修改记录：
 * --------------------
 *	 
 ******************************************************************************/
void Alldata_sliding_average(bool onoff)
{	
	int i=0;
	if(onoff==1)
	{		
		for(i=0;i<8;i++)
		{
			AD7606AverageValueBuf[i]=data_sliding_average(i,adc_data[i]);
		}
	}
	else
	{
		for(i=0;i<8;i++)
		{
			AD7606AverageValueBuf[i]=adc_data[i];
		}
	}
}


// 启动AD7606转换
void AD7606_StartConv(void) {
    // 启动转换
    GPIO_ResetBits(AD7606_CONVST_PORT, AD7606_CONVST_PIN);

    // 保持CONVST信号一段时间
    for (volatile uint32_t i = 0; i < 100; i++);

	GPIO_SetBits(AD7606_CONVST_PORT, AD7606_CONVST_PIN);
}

extern float k[13];
extern float b[13];
/******************************************************************************
 * 函 数 名：AD7606Analog_data_Conversion
 *
 * 函数说明: 将采集到的数字量转换为模拟量，单位为mA，mv
 * 参数说明: 
 * 输入参数: 
 * 输出参数: 
 * 返 回 值: 
 * 
 * 修改记录：
 *			20250724：采集回来的数据顺序和定的协议不一样，调整了下数据顺序
 * --------------------
 *	 
 ******************************************************************************/
void AD7606Analog_data_Conversion()
{
	int i =0; 
	int16_t adc=0,temp;

	for (i = 0;i < 8; i++)
    {
    	adc = AD7606AverageValueBuf[i];	
#if 0
		switch (i)
		{
			case 0:
				analog_data[0] = (adc * 10*1000)*4.1481 / 32768; //HJJC1_电压
				break;
			case 1:
				temp=(adc * 10*1000) / 32768;				//HJJC1_电流
				analog_data[1] = temp /0.5;
				break;
			case 2:					   
				temp=(adc * 10*1000) / 32768;				//QGSJ_电流
				analog_data[5] = temp / 0.5;
				break;
			case 3:				
				analog_data[4] = (adc * 10*1000)*4.1296 / 32768;	//QGSJ_电压
				break;
			case 4:
				temp=(adc * 10*1000) / 32768;				//HJJC2_电流
				analog_data[3] = temp /0.5;
				break;
			case 5:
				analog_data[2] = (adc * 10*1000)*4.1481 / 32768;	//HJJC2_电压
				break;
			case 6:
				temp=(adc * 10*1000) / 32768;				//WAOXJ_电流
				analog_data[7] = temp / 0.5;
				break;
			case 7:
				analog_data[6] = (adc * 10*1000)*4.1298 / 32768;	//WAOXJ_电压
				break;	
			default:
				break;				
		}
#else
		switch (i)
		{
			case 0:
				analog_data[0] = (adc * 10*1000)*4 / 32768; //HJJC1_电压
				analog_data[0] = k[MCU4_28V_HJJC1]*analog_data[0]+b[MCU4_28V_HJJC1];
				break;
			case 1:
				temp=(adc * 10*1000) / 32768;				//HJJC1_电流
				analog_data[1] = temp /0.5;
				break;
			case 2:					   
				temp=(adc * 10*1000) / 32768;				//QGSJ_电流
				analog_data[5] = temp / 0.5;
				break;
			case 3:				
				analog_data[4] = (adc * 10*1000)*4 / 32768;	//QGSJ_电压
				analog_data[4] = k[MCU4_28V_BF]*analog_data[4]+b[MCU4_28V_BF];
				break;
			case 4:
				temp=(adc * 10*1000) / 32768;				//HJJC2_电流
				analog_data[3] = temp /0.5;
				break;
			case 5:
				analog_data[2] = (adc * 10*1000)*4 / 32768;	//HJJC2_电压
				analog_data[2] = k[MCU4_28V_HJJC2]*analog_data[2]+b[MCU4_28V_HJJC2];
				break;
			case 6:
				temp=(adc * 10*1000) / 32768;				//WAOXJ_电流
				analog_data[7] = temp / 0.5;
				break;
			case 7:
				analog_data[6] = (adc * 10*1000)*4 / 32768;	//WAOXJ_电压
				analog_data[6] = k[MCU4_28V_WAOXJ]*analog_data[6]+b[MCU4_28V_WAOXJ];
				break;	
			default:
				break;				
		}
#endif
    }
	
}

void AD7606_Display(void)
{
    uint8_t i;
    int16_t adc=0;
    for (i = 0;i < 8; i++)
    {
        adc = adc_data[i];
        analog_data[i] = (adc * 10*1000) / 32768;
        TRACE_OUT(LYSDEBUG1,"adc_data[%d]=%-8x,analog_data=%dmv\r\n",i,adc_data[i],analog_data[i]);
    }
}
#if 0
// 主函数示例
int main(void)
{
	Spi1_Gpio_Init();
    Spi3_Gpio_Init();
    Spi3_Configuration();
    Spi1_Configuration();
    AD7606_Init();
    EXTI_Configuration();
    while (1) {
        AD7606_StartConv();
        // 可以添加延时，控制采样频率
        for (volatile uint32_t i = 0; i < 1000000; i++);
    }
}
#endif
