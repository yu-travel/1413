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
#include <stm32f4xx_flash.h>

void Flash_Init(void)
{
    // 使能Flash接口时钟
  //  RCC_APB2PeriphClockCmd(RCC_APB2Periph_FLASH, ENABLE);
}

void Flash_ErasePage(uint32_t PageAddress)
{
    // 解锁Flash编程/擦除控制器
    FLASH_Unlock();
    // 擦除指定页
    FLASH_EraseSector(PageAddress,VoltageRange_4);
	
    // 锁定Flash编程/擦除控制器
    FLASH_Lock();
}

void WriteFloatDirectToFlash(uint32_t WriteAddress, uint32_t floatData)
{
    FLASH_Unlock();
    // 检查Flash是否忙，若忙则等待
    while (FLASH_GetFlagStatus(FLASH_FLAG_BSY) != RESET);
    // 检查是否有写保护错误
    if (FLASH_GetFlagStatus(FLASH_FLAG_WRPERR) != RESET)
    {
        FLASH_ClearFlag(FLASH_FLAG_WRPERR);
    }
    // 检查是否有编程错误
    if (FLASH_GetFlagStatus(FLASH_FLAG_PGAERR) != RESET)
    {
        FLASH_ClearFlag(FLASH_FLAG_PGAERR);
    }
    // 直接写入32位浮点数（float在C语言中通常占4字节）
    FLASH_ProgramWord(WriteAddress, floatData); 
    FLASH_Lock();
}

/**
 * 向Flash写入多个32位数据
 * @param WriteAddress 起始地址（需4字节对齐）
 * @param data 待写入的32位数据数组
 * @param count 数据个数
 */
void WriteMultiple32ToFlash(uint32_t WriteAddress, const uint32_t *data, uint32_t count)
{
    if (data == NULL || count == 0)
        return; // 无效参数检查

    FLASH_Unlock();
    uint32_t currentAddr = WriteAddress;

    for (uint32_t i = 0; i < (count/4); i++)
    {
        // 等待Flash空闲
        while (FLASH_GetFlagStatus(FLASH_FLAG_BSY) != RESET);

        // 清除错误标志
        if (FLASH_GetFlagStatus(FLASH_FLAG_WRPERR) != RESET)
            FLASH_ClearFlag(FLASH_FLAG_WRPERR);
        if (FLASH_GetFlagStatus(FLASH_FLAG_PGAERR) != RESET)
            FLASH_ClearFlag(FLASH_FLAG_PGAERR);

        // 写入单个32位数据
        FLASH_ProgramWord(currentAddr, data[i]);

        // 地址偏移4字节（32位）
        currentAddr += 4;
    }

    FLASH_Lock();
}

/**
 * 从Flash指定地址读取32位数据
 * @param address Flash地址（必须在0x08000000 ~ 0x080FFFFF范围内，针对STM32F407RG）
 * @return 读取到的32位数据
 */
uint32_t Flash_ReadWord(uint32_t address)
{
    // 检查地址是否在Flash范围内（STM32F407RG的Flash最大地址为0x080FFFFF）
    if (address < 0x08000000 || address > 0x080FFFFF)
    {
        return 0xFFFFFFFF; // 地址无效，返回错误值
    }
    
    // 通过指针直接读取32位数据（地址需4字节对齐）
    return *(volatile uint32_t *)address;
}

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


extern uint32_t k_u32[13];
extern int32_t b_u32[13];
uint32_t readBuf[13]={0};
int32_t readBuf_1[13]={0};

int main(void)
{

	uint32_t ret=0,errNum=0;
	uint16_t temp; 
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
    delay_init(168);    //初始化延时函数,lys：通过分析可得系统时钟为168M
    delay_ms(1500);     // 电源管理板延时启动

    TRACE_OUT(DEBUG_OUT, "<DYGLB> system init ......\r\n");
	//todo:
//	i2c_all_init();
//	default_Power_Config();

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
//    IWDG_Init(4, 800);  			// 看门狗配置，超时时间1.6s 	

    TRACE_OUT(DEBUG_OUT, "<DYGLB> System init completed, enter loop ......\r\n");
    while(1)
    {    
      	softtimer_loop();			//软定时器循环
      	Flash_ErasePage(FLASH_Sector_11);
		save_k_b();
		
		WriteMultiple32ToFlash(0x080E0000,k_u32,sizeof(k_u32));
		WriteMultiple32ToFlash(0x080E0000+sizeof(k_u32),b_u32,sizeof(b_u32));

		Flash_ReadBytes(0x080E0000,readBuf,sizeof(k_u32));
		Flash_ReadBytes(0x080E0000+sizeof(k_u32),readBuf_1,sizeof(b_u32));

		TRACE_OUT(DEBUG_OUT,"biao ding END!!!!!!!       \r\n");
		TRACE_OUT(DEBUG_OUT,"you can break off power!!!!!!!     \r\n");
		break;


    }
	while(1)
	{
	    softtimer_loop();			//软定时器循环

	}
}



