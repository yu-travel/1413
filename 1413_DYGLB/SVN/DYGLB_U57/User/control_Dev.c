/******************************************************************************
 * @copyright
 * Copyright (c) 2025, Chengdu  UCAS Co., Ltd. All rights reserved.
 
 * @file control_Dev.c
 *
 * @brief 初始化控制开关的gpio管脚，和对收到的信息进行处理
 *
 * <pre>
 * 修改记录:
 * 2025-07-03, liangys, First create.
 * </pre>
 ******************************************************************************/
/******************************* Include Files ********************************/
#include "stm32f4xx.h"
#include "control_Dev.h"
#include "main.h"
#include "AD7606_Driver.h"
#include "AD5542_Driver.h"
#include "transf_jkkzb.h"
#include "at24cxx.h"
#include "stdbool.h"
/**************************** Constant Definitions ****************************/


/****************************** Type Definitions ******************************/

/**************************** Variable Declare ****************************/
bool limit_Init_Flag=1;//是否是上电初始化，0为否，1为是
//uint16_t limit_Current_Data[DEVNUM]={3500,3500,1000,1500};//限流值，默认值为初始限流值
uint16_t limit_Current_Data[DEVNUM]={5000,5000,5000,5000};//限流值，默认值为初始限流值,20250919:为了保证默认上电启动，能正常启动，改大限流
/******************* Macros (Inline Functions) Definitions ********************/
#define OPERATION_HJJC1(stu) \
	do{stu?GPIO_SetBits(GPIOA,GPIO_Pin_9):GPIO_ResetBits(GPIOA,GPIO_Pin_9);}while(0)
#define OPERATION_HJJC2(stu) \
	do{stu?GPIO_SetBits(GPIOA,GPIO_Pin_12):GPIO_ResetBits(GPIOA,GPIO_Pin_12);}while(0)
#define OPERATION_BF(stu) \
	do{stu?GPIO_SetBits(GPIOB,GPIO_Pin_3):GPIO_ResetBits(GPIOB,GPIO_Pin_3);}while(0)
#define OPERATION_WAOXJ(stu) \
	do{stu?GPIO_SetBits(GPIOC,GPIO_Pin_3):GPIO_ResetBits(GPIOC,GPIO_Pin_3);}while(0)

/******************* 			Extern	Declare				   ********************/
extern  SEND_MESSAGE sendmessage;
extern  RECIVE_MESSAGE recivemessage;



/******************************************************************************
 * 函 数 名：control_Gpio_Init
 *
 * 函数说明: 开关gpio初始化函数
 * 参数说明: 
 * 输入参数: 
 * 输出参数: 
 * 返 回 值: 
 * 
 * 修改记录：
 * --------------------
 *	 
 ******************************************************************************/
void control_Gpio_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    // 使能GPIOA和GPIOC时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD , ENABLE);

	// pa10: HJJC1_alert pb0:HJJC2_alert pb4:BF_alert pc4:WAOXJ_alert
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_4 ;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    // pa9: HJJC1 pa12:HJJC2 pb3:BF pc7:WAOXJ
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_12 ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_3;
    GPIO_Init(GPIOC, &GPIO_InitStructure);	
    // LED:pd2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	//PA11:HJJC1 PB1:HJJC2 PB5:BF PC5:WAOXJ  reset
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOA, &GPIO_InitStructure)   ;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_5;
	GPIO_Init(GPIOB, &GPIO_InitStructure)   ;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOC, &GPIO_InitStructure)   ;
	
    GPIO_SetBits(GPIOA, GPIO_Pin_11);//4001默认设置为锁存模式
	GPIO_SetBits(GPIOB, GPIO_Pin_1);//4001默认设置为锁存模式
	GPIO_SetBits(GPIOB, GPIO_Pin_5);//4001默认设置为锁存模式
	GPIO_SetBits(GPIOC, GPIO_Pin_5);//4001默认设置为锁存模式


	/*默认关闭设备*/
    GPIO_ResetBits(GPIOA, GPIO_Pin_9);
	GPIO_ResetBits(GPIOA, GPIO_Pin_12);
	GPIO_ResetBits(GPIOB, GPIO_Pin_3);
	GPIO_ResetBits(GPIOC, GPIO_Pin_3);
}

uint32_t dev_First_Open_Flag[DEVNUM]={0};//0:未打开，1：已达开
uint32_t dev_Open_counter_time[DEVNUM]={0};//单位为ms
uint32_t selfRecoveTime_0 =0;
uint32_t selfRecoveTime_1 =0;
uint32_t temp1=0;
/******************************************************************************
 * 函 数 名：operation_dev
 *
 * 函数说明: 根据收到的信息，对设备开关进行操作。
 * 参数说明: 
 * 输入参数: 
 * 输出参数: 
 * 返 回 值: 
 * 
 * 修改记录：
 * --------------------
 *	 
 ******************************************************************************/
void operation_dev(void) 
{	
	uint8_t stu[DEVNUM]={0};
	stu[HJJC1_DEV_NUM]=recivemessage.stuAndwarning.bits.hjjc1_stu;
	stu[HJJC2_DEV_NUM]=recivemessage.stuAndwarning.bits.hjjc2_stu;
	stu[BF_DEV_NUM]=recivemessage.stuAndwarning.bits.bf_stu;
	stu[WAOXJ_DEV_NUM]=recivemessage.stuAndwarning.bits.waoxj_stu;


	if(stu[HJJC1_DEV_NUM]==0)//hjjc1_RESET
	{		
		GPIO_ResetBits(GPIOA, GPIO_Pin_11);
		for(int i=0;i<50;i++);
		GPIO_SetBits(GPIOA, GPIO_Pin_11);
		dev_First_Open_Flag[HJJC1_DEV_NUM]=0;  //设备打开标志清0
		dev_Open_counter_time[HJJC1_DEV_NUM]=0;//设备打开时间清0
	}
	if(stu[HJJC2_DEV_NUM]==0)//hjjc2_RESET
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_1);
		for(int i=0;i<50;i++);
		GPIO_SetBits(GPIOB, GPIO_Pin_1);
		dev_First_Open_Flag[HJJC2_DEV_NUM]=0;  //设备打开标志清0
		dev_Open_counter_time[HJJC2_DEV_NUM]=0;//设备打开时间清0
	}
		if(stu[BF_DEV_NUM]==0)//bf_RESET
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_5);
		for(int i=0;i<50;i++);
		GPIO_SetBits(GPIOB, GPIO_Pin_5);
		dev_First_Open_Flag[BF_DEV_NUM]=0;  //设备打开标志清0
		dev_Open_counter_time[BF_DEV_NUM]=0;//设备打开时间清0
	}
		if(stu[WAOXJ_DEV_NUM]==0)//waoxj_RESET
	{
		GPIO_ResetBits(GPIOC, GPIO_Pin_5);
		for(int i=0;i<50;i++);
		GPIO_SetBits(GPIOC, GPIO_Pin_5);
		dev_First_Open_Flag[WAOXJ_DEV_NUM]=0;  //设备打开标志清0
		dev_Open_counter_time[WAOXJ_DEV_NUM]=0;//设备打开时间清0
	}
/*4001缓启动配置*/
	if((stu[HJJC1_DEV_NUM]==1)&&(dev_First_Open_Flag[HJJC1_DEV_NUM]==0))//收到打开指令，且还未打开过
	{		
		GPIO_ResetBits(GPIOA, GPIO_Pin_11);		//设置4001自恢复模式					
		dev_First_Open_Flag[HJJC1_DEV_NUM]=1;	//设置为已打开过

	}
	if((stu[HJJC2_DEV_NUM]==1)&&(dev_First_Open_Flag[HJJC2_DEV_NUM]==0))//收到打开指令，且还未打开过
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_1);
		dev_First_Open_Flag[HJJC2_DEV_NUM]=1;
	}
	if((stu[BF_DEV_NUM]==1)&&(dev_First_Open_Flag[BF_DEV_NUM]==0))//收到打开指令，且还未打开过
	{		
		GPIO_ResetBits(GPIOB, GPIO_Pin_5);		//设置4001自恢复模式					
		dev_First_Open_Flag[BF_DEV_NUM]=1;	//设置为已打开过

	}
	if((stu[WAOXJ_DEV_NUM]==1)&&(dev_First_Open_Flag[WAOXJ_DEV_NUM]==0))//收到打开指令，且还未打开过
	{
		GPIO_ResetBits(GPIOC, GPIO_Pin_5);
		dev_First_Open_Flag[WAOXJ_DEV_NUM]=1;
		selfRecoveTime_0=timer_getms_count();
	}

	OPERATION_HJJC1(recivemessage.stuAndwarning.bits.hjjc1_stu);
	OPERATION_HJJC2(recivemessage.stuAndwarning.bits.hjjc2_stu);
	OPERATION_BF (recivemessage.stuAndwarning.bits.bf_stu);	
	OPERATION_WAOXJ (recivemessage.stuAndwarning.bits.waoxj_stu);		
}
/******************************************************************************
 * 函 数 名：selfRecoveTime
 *
 * 函数说明: 4001自恢复保持时间，超时自动变为锁存模式
 * 参数说明: overTime：100~500单位ms
 * 输入参数: 
 * 输出参数: 
 * 返 回 值: 
 * 
 * 修改记录：
 * --------------------
 *	 
 ******************************************************************************/
void selfRecoveTime(uint32_t overTime) 
{
	if(dev_Open_counter_time[HJJC1_DEV_NUM]>overTime)
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_11);//设置4001锁存模式	

	}
	if((dev_Open_counter_time[HJJC2_DEV_NUM]>overTime)/*&&(temp1==0)*/)
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_1);//设置4001锁存模式	

	}
	if(dev_Open_counter_time[BF_DEV_NUM]>overTime)
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_5);//设置4001锁存模式	

	}
	if((dev_Open_counter_time[WAOXJ_DEV_NUM]>overTime))
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_5);//设置4001锁存模式	
		selfRecoveTime_1=diffTimer(selfRecoveTime_0);
        temp1=1;
		LYSprintf(LYSDEBUG5,"************ selfRecoveTime_1==%d ***************\r\n ",selfRecoveTime_1);
	}
}
/******************************************************************************
 * 函 数 名：limitCurrent_Config
 *
 * 函数说明: 下发限流配置
 * 参数说明: 
 * 输入参数: 
 * 输出参数: 
 * 返 回 值: 
 * 
 * 修改记录：
 * --------------------
 *	 
 ******************************************************************************/
void limitCurrent_Config(void) 
{
	int i=0,j=0;
	/*更新数据*/
	for(i=0;i<DEVNUM;i++)
	{
		j=i*3;
		limit_Current_Data[i]=recivemessage.messageData[j+2];	// 从接收到的消息中提取限流数据	
		//TRACE_OUT(DEBUG_OUT,"limit_Current_Data[%d]=%x\r\n",i,limit_Current_Data[i]);	
	}
	//先将数据转换成5542识别的数据
	AD5542Analog_data_Conversion();
	//向5542发送数据
	Ad5542_Send_Data();
}

/******************************************************************************
 * 函 数 名：Init_limitCurrent_Config
 *
 * 函数说明: 初始限流配置
 * 参数说明: 
 * 输入参数: 
 * 输出参数: 
 * 返 回 值: 
 * 
 * 修改记录：
 * --------------------
 *	 
 ******************************************************************************/
void Init_limitCurrent_Config(void) 
{
	int i=0;
	if(limit_Init_Flag==1)
	{
		for(i=0;i<DEVNUM;i++)
		{
	//		TRACE_OUT(DEBUG_OUT,"limit_Current_Data[%d]=%x\r\n",i,limit_Current_Data[i]);	
		}
		//先将数据转换成5542识别的数据
		AD5542Analog_data_Conversion();
		//向5542发送数据
		Ad5542_Send_Data();
		limit_Init_Flag=0;
	}
}

/******************************************************************************
 * 函 数 名：config_Default_Power
 *
 * 函数说明: 配置默认上电
 * 参数说明: 
 * 输入参数: 
 * 输出参数: 
 * 返 回 值: 
 * 
 * 修改记录：
 * --------------------
 *	 
 ******************************************************************************/
uint32_t config_Default_Power(DEFAULT_CONFIG_POWER config)
{
	uint8_t stu[DEVNUM]={0};
	stu[HJJC1_DEV_NUM]=config.bits.hjjc1_default_save;
	stu[HJJC2_DEV_NUM]=config.bits.hjjc2_default_save;
	stu[BF_DEV_NUM]=config.bits.bf_default_save;
	stu[WAOXJ_DEV_NUM]=config.bits.waoxj_default_save;	
	if(stu[HJJC1_DEV_NUM]==0)//hjjc1_RESET
	{		
		GPIO_ResetBits(GPIOA, GPIO_Pin_11);
		for(int i=0;i<50;i++);
		GPIO_SetBits(GPIOA, GPIO_Pin_11);
		dev_First_Open_Flag[HJJC1_DEV_NUM]=0;  //设备打开标志清0
		dev_Open_counter_time[HJJC1_DEV_NUM]=0;//设备打开时间清0
	}
	if(stu[HJJC2_DEV_NUM]==0)//hjjc2_RESET
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_1);
		for(int i=0;i<50;i++);
		GPIO_SetBits(GPIOB, GPIO_Pin_1);
		dev_First_Open_Flag[HJJC2_DEV_NUM]=0;  //设备打开标志清0
		dev_Open_counter_time[HJJC2_DEV_NUM]=0;//设备打开时间清0
	}
		if(stu[BF_DEV_NUM]==0)//bf_RESET
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_5);
		for(int i=0;i<50;i++);
		GPIO_SetBits(GPIOB, GPIO_Pin_5);
		dev_First_Open_Flag[BF_DEV_NUM]=0;  //设备打开标志清0
		dev_Open_counter_time[BF_DEV_NUM]=0;//设备打开时间清0
	}
		if(stu[WAOXJ_DEV_NUM]==0)//waoxj_RESET
	{
		GPIO_ResetBits(GPIOC, GPIO_Pin_5);
		for(int i=0;i<50;i++);
		GPIO_SetBits(GPIOC, GPIO_Pin_5);
		dev_First_Open_Flag[WAOXJ_DEV_NUM]=0;  //设备打开标志清0
		dev_Open_counter_time[WAOXJ_DEV_NUM]=0;//设备打开时间清0
	}
/*4001缓启动配置*/
	if((stu[HJJC1_DEV_NUM]==1)&&(dev_First_Open_Flag[HJJC1_DEV_NUM]==0))//收到打开指令，且还未打开过
	{		
		GPIO_ResetBits(GPIOA, GPIO_Pin_11);		//设置4001自恢复模式					
		dev_First_Open_Flag[HJJC1_DEV_NUM]=1;	//设置为已打开过

	}
	if((stu[HJJC2_DEV_NUM]==1)&&(dev_First_Open_Flag[HJJC2_DEV_NUM]==0))//收到打开指令，且还未打开过
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_1);
		dev_First_Open_Flag[HJJC2_DEV_NUM]=1;
	}
	if((stu[BF_DEV_NUM]==1)&&(dev_First_Open_Flag[BF_DEV_NUM]==0))//收到打开指令，且还未打开过
	{		
		GPIO_ResetBits(GPIOB, GPIO_Pin_5);		//设置4001自恢复模式					
		dev_First_Open_Flag[BF_DEV_NUM]=1;	//设置为已打开过

	}
	if((stu[WAOXJ_DEV_NUM]==1)&&(dev_First_Open_Flag[WAOXJ_DEV_NUM]==0))//收到打开指令，且还未打开过
	{
		GPIO_ResetBits(GPIOC, GPIO_Pin_5);
		dev_First_Open_Flag[WAOXJ_DEV_NUM]=1;
		selfRecoveTime_0=timer_getms_count();
	}

	OPERATION_HJJC1(recivemessage.stuAndwarning.bits.hjjc1_stu);
	OPERATION_HJJC2(recivemessage.stuAndwarning.bits.hjjc2_stu);
	OPERATION_BF (recivemessage.stuAndwarning.bits.bf_stu);	
	OPERATION_WAOXJ (recivemessage.stuAndwarning.bits.waoxj_stu);		



	return 0;
}

//todo：默认上电配置
#if 0
power_ctrl_t power_ctrl_set[16] = {
    {.senior_id = ID_28V_TSGY , .onoff_ctrl = 0},
    {.senior_id = ID_28V_KF   , .onoff_ctrl = 0},
};

/******************************************************************************
 * 函 数 名：default_Power_Config()
 *
 * 函数说明: 默认上电配置
 * 参数说明: 
 * 输入参数: 
 * 输出参数: 
 * 返 回 值: 
 * 
 * 修改记录：
 * --------------------
 *	 
 ******************************************************************************/
void default_Power_Config()
{
	int ret = 0;
    int index = 0, copy_len = 0;
    u8 rd_buff[32] = {0}, power_id = 0, onoff = 0;

    // 获取上电配置
    copy_len = sizeof(power_ctrl_set);
    ret = I2C_EE_BufferRead(rd_buff, EEP_Firstpage, copy_len);
	if(ret == 1)
    {
        for(index = 0; index < ARRAY_SIZE(power_ctrl_set); index++)
        {
            power_id    = power_ctrl_set[index].senior_id;
            onoff       = power_ctrl_set[index].onoff_ctrl;
            TRACE_OUT(DEBUG_OUT, "Line%d: POWER ID[%02x] set %s\r\n", __LINE__, power_id, (onoff==POWER_ON)?"POWER ON":"POWER OFF");
            switch(power_id)
            {
                case ID_28V_TSGY:
					OPERATION_TSGY(onoff);
                break;
                case ID_28V_KF:
					OPERATION_KF(onoff);
                break;

				default:
				
				break;
            }

        }
	}
	else
	{
		
	}
}
#endif


/***************************/
#if SELFTEST

	ret=messageData_Recive();
	if(ret=1)
	{
		operation_dev();
		limitCurrent_Config();
	}
	
#endif

