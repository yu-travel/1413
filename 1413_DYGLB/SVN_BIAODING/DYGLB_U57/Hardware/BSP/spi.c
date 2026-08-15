/*
***********************************************************************************************************************
    @brief          : 板载spi初始化
    @author         : xiongjinqi
    @date           : 2024/07/21
***********************************************************************************************************************
*/
#include "main.h"
#include "spi.h"

#if 1

//开启一次DMA传输
//DMA_Streamx:DMA数据流,DMA1_Stream0~7/DMA2_Stream0~7 
//ndtr:数据传输量  
void myDMA_enable(DMA_Stream_TypeDef *DMA_Streamx, u16 ndtr)
{
    DMA_Cmd(DMA_Streamx, DISABLE);                      //关闭DMA传输 
    while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE){}  //确保DMA可以被设置  
    DMA_SetCurrDataCounter(DMA_Streamx, ndtr);          //数据传输量  
    DMA_Cmd(DMA_Streamx, ENABLE);                      //开启DMA传输 
}
#if 0
void SPI1_DMA_Init(u32 *txbuff, u16 txlen)
{
    DMA_InitTypeDef DMA_InitStructure;

      /* Enable DMA clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

      /* DMA configuration -------------------------------------------------------*/
    /* Deinitialize DMA Streams */
    DMA_DeInit(DMA2_Stream3);
    while (DMA_GetCmdStatus(DMA2_Stream3) != DISABLE){}//等待DMA可配置 

    /* 配置 DMA Stream */
    DMA_InitStructure.DMA_Channel           = DMA_Channel_3;  //通道选择
    DMA_InitStructure.DMA_PeripheralBaseAddr= (uint32_t) (&(SPI1->DR));//DMA外设地址
    DMA_InitStructure.DMA_Memory0BaseAddr   = (uint32_t)txbuff;//DMA 存储器0地址
    DMA_InitStructure.DMA_BufferSize        = txlen;//数据传输量
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;//存储器到外设模式
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设非增量模式
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//存储器增量模式
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设数据长度:8位
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//存储器数据长度:8位
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;   // 使用普通模式 
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;//中等优先级
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//存储器突发单次传输
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//外设突发单次传输
    DMA_Init(DMA2_Stream3, &DMA_InitStructure);     //初始化DMA Stream

    myDMA_enable(DMA2_Stream3, txlen);  // 启用dma传输
    
    #if 0
    /* Configure RX DMA */
    DMA_InitStructure.DMA_Channel = SPIx_RX_DMA_CHANNEL ;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory ;
    DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)aRxBuffer ; 
    DMA_Init(SPIx_RX_DMA_STREAM, &DMA_InitStructure);
    #endif
}
#endif

//以下是SPI模块的初始化代码，配置成从机模式
//SPI口初始化
//这里针是对SPI1的初始化
void SPI1_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOB时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);//使能SPI1时钟

    
    //GPIOFB3,4,5初始化设置
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;//PB3~5复用功能输出  
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;     //复用功能
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;    //推挽输出
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;     //上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);              //初始化
    /* CS 初始化 */
    GPIO_InitStructure.GPIO_Pin     = SPI1_CS_Pin;//LED0和LED1对应IO口
    GPIO_Init(SPI1_CS_GPIO_Port, &GPIO_InitStructure);     //初始化GPIO
    
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_SPI1); //PA4复用为 SPI1
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1); //PB3复用为 SPI1
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1); //PB4复用为 SPI1
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1); //PB5复用为 SPI1
 
    //这里只针对SPI口初始化
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,ENABLE);//复位SPI1
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,DISABLE);//停止复位SPI1

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
    SPI_InitStructure.SPI_Mode      = SPI_Mode_Slave;    //设置SPI工作模式:设置为从机
    SPI_InitStructure.SPI_DataSize  = SPI_DataSize_8b;   //设置SPI的数据大小:SPI发送接收8位帧结构
    SPI_InitStructure.SPI_CPOL      = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA      = SPI_CPHA_2Edge;
//    SPI_InitStructure.SPI_CPOL      = SPI_CPOL_High;     //串行同步时钟的空闲状态为高电平
//    SPI_InitStructure.SPI_CPHA      = SPI_CPHA_2Edge;    //串行同步时钟的第二个跳变沿（上升或下降）数据被采样
    SPI_InitStructure.SPI_NSS       = SPI_NSS_Hard;      //NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;        //定义波特率预分频的值:波特率预分频值为64
    SPI_InitStructure.SPI_FirstBit  = SPI_FirstBit_MSB;  //指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
    SPI_InitStructure.SPI_CRCPolynomial = 7;    //CRC值计算的多项式
    SPI_Init(SPI1, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
    SPI_Cmd(SPI1, ENABLE); //使能SPI外设
    //SPI_CalculateCRC(SPI1, ENABLE);     // 使能CRC计算
    //SPI1_SetSpeed(SPI_BaudRatePrescaler_64);

    /* Configure the SPI interrupt priority */
    NVIC_InitStructure.NVIC_IRQChannel = SPIx_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_RXNE, ENABLE);    // 启用中断
}   

//SPI1速度设置函数
//SPI速度=fAPB2/分频系数
//@ref SPI_BaudRate_Prescaler:SPI_BaudRatePrescaler_2~SPI_BaudRatePrescaler_256  
//fAPB2时钟一般为84Mhz：
void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));//判断有效性
    SPI1->CR1&=0XFFC7;//位3-5清零，用来设置波特率
    SPI1->CR1|=SPI_BaudRatePrescaler;   //设置SPI1速度
    SPI_Cmd(SPI1,ENABLE); //使能SPI1
}


#endif
