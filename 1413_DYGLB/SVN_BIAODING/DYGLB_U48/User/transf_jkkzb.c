/******************************************************************************
 * @copyright
 * Copyright (c) 2025, Chengdu  UCAS Co., Ltd. All rights reserved.
 
 * @file transf_jkkzb.c
 *
 * @brief 和接口扩展板进行通信
 *
 * <pre>
 * 修改记录:
 * 2025-07-05, liangys, First create.
 * </pre>
 ******************************************************************************/
		 
/******************************* Include Files ********************************/

#include "stm32f4xx.h"
#include "main.h"
#include "AD7606_Driver.h"
#include "transf_jkkzb.h"
/**************************** Constant Definitions ****************************/


/****************************** Type Definitions ******************************/



/**************************** Variable Definitions ****************************/
 SEND_MESSAGE sendmessage;
 RECIVE_MESSAGE recivemessage;

/******************* Macros (Inline Functions) Definitions ********************/
#define DEV_STU_GET()\
	do{	sendmessage.stuAndwarning.bits.sfxj_stu=   GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_9);\
		sendmessage.stuAndwarning.bits.sfpsd_stu=   GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_12);\
		sendmessage.stuAndwarning.bits.bqxj_stu=    GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_3);\
		sendmessage.stuAndwarning.bits.qgsj_stu=    GPIO_ReadOutputDataBit(GPIOC,GPIO_Pin_7);\
		sendmessage.stuAndwarning.bits.sfxj_warning=   	!(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_10));\
		sendmessage.stuAndwarning.bits.sfpsd_warning=   !(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0));\
	    sendmessage.stuAndwarning.bits.bqxj_warning=    !(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4));\
	    sendmessage.stuAndwarning.bits.qgsj_warning=    !(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9));\
	  }while(0)
/******************* 			Extern	Declare				   ********************/
extern volatile uint16_t analog_data[ADC_CHANNEL_NUM];

/******************************************************************************
 * 函 数 名：SPI1_GPIO_Init
 *
 * 函数说明: spi1的gpio初始化函数
 * 参数说明: 
 * 输入参数: 
 * 输出参数: 
 * 返 回 值: 
 * 
 * 修改记录：
 * --------------------
 *	 
 ******************************************************************************/
void Spi1_Gpio_Init(void) 
{
    GPIO_InitTypeDef GPIO_InitStructure;
    // 使能GPIOA和GPIOC时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // 配置SPI1引脚：CS(PA4),SCK(PA5), MISO(PA6), MOSI(PA7),当spi为从时cs管脚，配置为复用模式
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//配置SPI1片选，当spi为主时cs管脚，配置为输出模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  // 片选引脚必须为输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 将SPI1引脚连接到AF功能
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);
	
    // 初始化片选引脚为高电平，默认不选中fpga
    GPIO_SetBits(GPIOA, GPIO_Pin_4);

}

/******************************************************************************
 * 函 数 名：Spi1_Configuration
 *
 * 函数说明: spi1初始化
 * 参数说明: 
 * 输入参数: 
 * 输出参数: 
 * 返 回 值: 
 * 
 * 修改记录：
 * --------------------
 *	 
 ******************************************************************************/
void Spi1_Configuration(void) {
    SPI_InitTypeDef SPI_InitStructure;

    // 使能SPI1时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE); //APB1的时钟为42M，APB2的时钟速率为84M
	
//	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,ENABLE);
//	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,DISABLE);

    // 配置SPI1
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	//SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    //SPI_InitStructure.SPI_NSS = SPI_NSS_Hard;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;	
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);

    // 使能SPI1
    SPI_Cmd(SPI1, ENABLE);
}


// 通过SPI1发送和接收数据
uint16_t SPI1_ReadWrite(uint16_t data) {
    // 等待发送缓冲区为空
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    // 将数据写入SPI3的数据寄存器，触发数据发送
    SPI_I2S_SendData(SPI1, data);
    // 等待接收缓冲区非空，即有数据接收到
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    // 从SPI3的数据寄存器读取接收到的数据
    return SPI_I2S_ReceiveData(SPI1);
}

/******************************************************************************
 * 函 数 名：data_Packet_Creat
 *
 * 函数说明: 生成发送给fpga的数据包
 * 参数说明: 
 * 输入参数: 
 * 输出参数: 
 * 返 回 值: 
 * 
 * 修改记录：
 * --------------------
 *	 
 ******************************************************************************/
void data_Packet_Creat()
{
	uint16_t temp=0;
	int base=0,i=0;
	sendmessage.dataHead=0x55AA;
	sendmessage.data_len=3*DEVNUM*2+2;
	
	/*将设备号添加进数据*/
	for (i = 0; i < DEVNUM; i++) 
	{
	    base = i * 3;
	    sendmessage.messageData[base] = i+MCU3_28V_SFXJ;            			//设备号
	    	ttprintf(LYSDEBUG2,"*********sendmessage.messageData[%x]==%d***************\r\n",base,sendmessage.messageData[base]);
	    sendmessage.messageData[base+1] = analog_data[i*2];     // 电压
	    	ttprintf(LYSDEBUG2,"*********sendmessage.messageData[%x]==%d***************\r\n",base+1,sendmessage.messageData[base+1]);
	    sendmessage.messageData[base+2] = analog_data[i*2+1];   // 电流
	    	ttprintf(LYSDEBUG2,"*********sendmessage.messageData[%x]==%d***************\r\n",base+2,sendmessage.messageData[base+2]);
	}
	
	//开关状态和告警信息获取填充 
	DEV_STU_GET();
	ttprintf(LYSDEBUG2,"sendmessage.stuAndwarning.raw==%x\r\n",sendmessage.stuAndwarning.raw);
	/*和校验*/
	temp=sendmessage.dataHead+sendmessage.data_len;	
	for(i=0;i<3*DEVNUM;i++)
	{
		temp=temp+sendmessage.messageData[i];
	}
	temp=temp+sendmessage.stuAndwarning.raw;
	sendmessage.sum=temp;
	ttprintf(LYSDEBUG2,"*********sendmessage.sum==%x***************\r\n",sendmessage.sum);
	/*帧尾*/
	sendmessage.dataEnd=0xACBC;
}

/******************************************************************************
 * 函 数 名：messageData_Send
 *
 * 函数说明: 消息发送
 * 参数说明: 
 * 输入参数: 
 * 输出参数: 
 * 返 回 值: 
 * 
 * 修改记录：
 * --------------------
 *	 
 ******************************************************************************/
void messageData_Send()
{
	int i=0;
    uint16_t *data=(uint16_t *)&sendmessage;
	// 拉低片选信号
	GPIO_ResetBits(GPIOA, GPIO_Pin_4);
    //SPI1_ReadWrite(sendmessage.dataHead);
	//SPI1_ReadWrite(sendmessage.data_len);
	for(i=0;i<sendmessage.data_len+2;i++)
	{
		SPI1_ReadWrite(data[i]);
	}
	// 禁用片选	
    GPIO_SetBits(GPIOA, GPIO_Pin_4); 
}
/******************************************************************************
 * 函 数 名：messageData_Recive
 *
 * 函数说明: 消息接收,累加和校验
 * 参数说明: 
 * 输入参数: 
 * 输出参数: 
 * 返 回 值: 数据是否接受正确，1：正确，0：错误
 * 
 * 修改记录：
 * --------------------
 *	 
 ******************************************************************************/
int messageData_Recive()
{
	int i=0,j=0,data_RigtFlag=0,err_cnt=0;
	uint16_t sum=0,timeout=0;
	uint16_t temp=0;
	while(data_RigtFlag==0)
	{
		sum=0;
	/*握手包*/
		// 拉低片选信号
		GPIO_ResetBits(GPIOA, GPIO_Pin_4);
		temp=SPI1_ReadWrite(0x55AA);
		SPI1_ReadWrite(0x0002);
		SPI1_ReadWrite(0xABDE);
		SPI1_ReadWrite(0x018A);
		SPI1_ReadWrite(0xACBC);
		// 拉高片选信号
		GPIO_SetBits(GPIOA, GPIO_Pin_4);	
		delay_us(1);
		// 拉低片选信号
		GPIO_ResetBits(GPIOA, GPIO_Pin_4);
		temp=SPI1_ReadWrite(0x77AA);
	/*发送完和逻辑的握手包后，等待接收包头，超时退出。todo：直接退出接收*/
//		while((temp!=0xAA55)&&(timeout<10000))
//		{
//			temp=SPI1_ReadWrite(0x77AA);
//			TRACE_OUT(DEBUG_OUT,"spi_head[%d]=%x\r\n",i,temp);
//			printf("spi_head[%d]=%x\r\n",i,temp);
//			timeout++;
//			goto DATA_ERROR;
//		}

		TRACE_OUT(LYSDEBUG2,"spi_head[%d]=%x\r\n",i,temp);
		ttprintf(LYSDEBUG2,"spi_head[%d]=%x\r\n",i,temp);

		recivemessage.dataHead=temp;
		recivemessage.data_len=SPI1_ReadWrite(0x00);
		
		TRACE_OUT(LYSDEBUG2,"recivemessage.data_len==%x\r\n",recivemessage.data_len);
		ttprintf(LYSDEBUG2,"recivemessage.data_len==%x\r\n",recivemessage.data_len);

		for(i=0;i<DEVNUM;i++)
		{
			j=i*3;
			recivemessage.messageData[j]=SPI1_ReadWrite(0);		// 设备号
			TRACE_OUT(LYSDEBUG2,"recivemessage.messageData[%d]==%x\r\n",j,recivemessage.messageData[j]);
			ttprintf(LYSDEBUG2,"recivemessage.messageData[%d]==%x\r\n",j,recivemessage.messageData[j]);

			recivemessage.messageData[j+1]=SPI1_ReadWrite(0);	// 限压	
			TRACE_OUT(LYSDEBUG2,"recivemessage.messageData[%d]==%x\r\n",j+1,recivemessage.messageData[j+1]);
			ttprintf(LYSDEBUG2,"recivemessage.messageData[%d]==%x\r\n",j+1,recivemessage.messageData[j+1]);


			recivemessage.messageData[j+2]=SPI1_ReadWrite(0);	// 限流
			TRACE_OUT(LYSDEBUG2,"recivemessage.messageData[%d]==%x\r\n",j+2,recivemessage.messageData[j+2]);
			ttprintf(LYSDEBUG2,"recivemessage.messageData[%d]==%x\r\n",j+2,recivemessage.messageData[j+2]);

		}
		recivemessage.stuAndwarning.raw=SPI1_ReadWrite(0);			// 
		TRACE_OUT(LYSDEBUG2,"recivemessage.stuAndwarning.raw==%x\r\n",recivemessage.stuAndwarning.raw);	
		ttprintf(LYSDEBUG2,"recivemessage.stuAndwarning.raw==%x\r\n",recivemessage.stuAndwarning.raw);

		recivemessage.sum=SPI1_ReadWrite(0);
		TRACE_OUT(LYSDEBUG2,"recivemessage.sum==%x\r\n",recivemessage.sum);	
		ttprintf(LYSDEBUG2,"recivemessage.sum==%x\r\n",recivemessage.sum);

		recivemessage.dataEnd=SPI1_ReadWrite(0);
		TRACE_OUT(LYSDEBUG2,"recivemessage.dataEnd==%x\r\n",recivemessage.dataEnd);	
		ttprintf(LYSDEBUG2,"recivemessage.dataEnd==%x\r\n",recivemessage.dataEnd);

		
		sum =  recivemessage.dataHead+recivemessage.data_len;
		for(i=0;i<DEVNUM*3;i++)
		{
			sum=sum+recivemessage.messageData[i];		
		}	
		sum=sum+recivemessage.stuAndwarning.raw;
		if((sum==recivemessage.sum)&&(sum!=0x0)&&(sum!=0xFFFF))//1、校验和正确2、校验和不等于0，防止fpga没有数据发送过来，程序依然判断校验和通过。
		{
			data_RigtFlag=1;
			ttprintf(LYSDEBUG2,"sum=0x%x,recivemessage.sum==%x,\r\n",sum,recivemessage.sum);
		}
		else
		{
		DATA_ERROR:
			err_cnt++;
		}
       // printf("sum=0x%x,recivemessage.sum==%x,\r\n",sum,recivemessage.sum);
		if(err_cnt==3)//如果3次数据接收错误，就跳出循环。
		{
			GPIO_SetBits(GPIOA, GPIO_Pin_4); 
			break;	
		}
		// 禁用片选	
	    GPIO_SetBits(GPIOA, GPIO_Pin_4); 
	}
	return data_RigtFlag;
}

//// SPI1中断服务函数
//void SPI1_IRQHandler(void)
//{
//    if (SPI_I2S_GetITStatus(SPI1, SPI_I2S_IT_RXNE) != RESET)
//	{
//        uint16_t recv_data = SPI_I2S_ReceiveData(SPI1);
//        if (recv_data == 0x55AA) 
//		{
//            trigger_send = 1;
//    	}
//        // 清除接收缓冲区非空中断标志
//        SPI_I2S_ClearITPendingBit(SPI1, SPI_I2S_IT_RXNE);
//    }	
//    if (trigger_send) {
//        for (int i = 0; i < ADC_CHANNEL_NUM; i++) {
//            while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
//            SPI_I2S_SendData(SPI1, adc_data[i]);
//            while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
//            // 读取无用数据以清除接收缓冲区
//            SPI_I2S_ReceiveData(SPI1); 
//        }
//        trigger_send = 0;
//    }
//}


