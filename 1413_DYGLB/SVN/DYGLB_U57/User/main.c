#include "main.h"
#include "delay.h"
#include "usart.h"
#include "softtimer.h"
#include "iwdg.h"
#include "timer.h"
#include "AD7606_Driver.h"
#include "AD5542_Driver.h"
#include "control_Dev.h"
#include "transf_jkkzb.h"
#include "types_def.h"
#include <math.h>

uint32_t k_u32[13]={0};
int32_t b_u32[13]={0};
float k[13]={0};
float b[13]={0};
extern DEFAULT_CONFIG_POWER before_default_config_power;//上电默认配置项
/**
 * 从Flash指定地址读取多个4字节到缓冲区
 * @param address 起始地址
 * @param buffer 接收数据的缓冲区
 * @param length 要读取的字节数
 */
void Flash_ReadBytes(uint32_t address, uint32_t *buffer, uint32_t length)
{
    if (buffer == NULL || length == 0)
    {
        return; // 参数无效
    }
    
    // 计算有效读取范围（不超过Flash边界）
    uint32_t maxAddress = 0x080FFFFF; // STM32F407RG的Flash结束地址
    if (address > maxAddress)
    {
        return;
    }
    if (address + length > maxAddress + 1)
    {
        length = maxAddress - address + 1; // 截断超出部分
    }
    
    // 4字节读取
    volatile uint32_t *flashPtr = (volatile uint32_t *)address;
    for (uint32_t i = 0; i < (length/4); i++)
    {
        buffer[i] = flashPtr[i];
    }
}

int main(void)
{

	uint32_t ret=0,errNum=0;
	uint16_t temp; 
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
    delay_init(168);    //初始化延时函数,lys：通过分析可得系统时钟为168M
    delay_ms(1500);     // 电源管理板延时启动
    TRACE_OUT(DEBUG_OUT, "<DYGLB> system init ......\r\n");
	//todo:
	i2c_all_init();					//初始化I2C

	Spi1_Gpio_Init();        		//spi1 gpio初始化
 	Spi1_Configuration();			//spi1 功能配置
	AD5542_Init();					//ad5542 相关功能初始化
    AD7606_Init();					//ad7606 相关功能初始化
	control_Gpio_Init();			//外设使能管脚初始化
	Init_limitCurrent_Config();		//默认限流配置
    EXTI_Configuration();    		//外部中断初始化

    usart1_init(115200);      		// 串口2初始化
    timer2_int_init(TIM_1KHZ, TIMER_PRESCALER); //定时器2初始化
    softtimer_init();				//软定时器初始化，依托于定时器2，所以只能在定时器2后初始化
    
    TIM3_Int_Init(10-1,8400-1);	//定时器时钟84M，分频系数8400，所以84M/8400=10Khz的计数频率，计数10次为1ms，用于4001自恢复时间计数    
    IWDG_Init(4, 800);  			// 看门狗配置，超时时间1.6s 	
	Flash_ReadBytes(0x080E0000,k_u32,sizeof(k_u32));
	Flash_ReadBytes(0x080E0000+sizeof(k_u32),b_u32,sizeof(b_u32));
	for(uint32_t i=0;i<13;i++)
	{
		k[i]=(uint32_t)k_u32[i]*1e-6;
		b[i]=(int32_t)b_u32[i]*1e-6;
	}
	get_Default_Power(&before_default_config_power);//从epprom里获取默认的上电配置信息,到before_default_config_power里
	config_Default_Power(before_default_config_power);//按照before_default_config_power里的配置信息，对外设进行使能

    TRACE_OUT(DEBUG_OUT, "<DYGLB> System init completed, enter loop ......\r\n");
    while(1)
    {    
      	softtimer_loop();			//软定时器循环

/*数据发送*/
		AD7606_StartConv();			//通知7606开始转换		
		Alldata_sliding_average(0);	//todo：滑动均值处理

		AD7606Analog_data_Conversion();//数字量转换为模拟量
		data_Packet_Creat();		//发送数据包创建
		messageData_Send();			//发送数据
		temp=diffTimer(temp);	
		//AD7606_Display();			//打印7606采集到的数据

		
/*数据接收及处理*/
		ret=messageData_Recive();	//消息接收
		temp=timer_getms_count();		
		if(ret==1)					//如果数据接受正确，就下发信息
		{
			operation_dev();			//控制设备开关
			limitCurrent_Config();		//设置限流
			save_Default_Power();		//存储下发的默认上电配置信息
		}
		else
		{
			errNum++;
			TRACE_OUT(DEBUG_OUT, "LIMIT CURENT MESSGE RECIVE ERROR\r\n");

		}
/*设备打开后4001自恢复保持时间设置*/
		selfRecoveTime(1000);//设置保持1000ms

    	//TRACE_OUT(DEBUG_OUT, "temp===**************%d*********************\r\n",temp);	
        //LYSprintf(DEBUG_OUT,"LIMIT CURENT MESSGE RECIVE ERROR,errNum=%d\r\n",errNum);
		
        delay_ms(1); 
    }
}



