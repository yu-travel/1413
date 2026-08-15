/*
***********************************************************************************************************************
    @brief          : 板载adc初始化
    @author         : xiongjinqi
    @date           : 2024/07/21
***********************************************************************************************************************
*/
#include "main.h"
#include "adc.h"

#define Channel_Num     7 //9 个通道
#define Sample_Num      10 //采样10次进行平均

u16 ADC_ConvertedValue[Sample_Num][Channel_Num] = {0};

_board_vol_t    brd_adc[] = {
    {.channel = VDDQ_Channel,0,0},
    {.channel = VDD_CORE_Channel,0,0},
    {.channel = VCC_1V8_Channel,0,0},
    {.channel = VCC_2V5_TST_Channel,0,0},
    {.channel = VCC_3V3_TST_Channel,0,0},
    {.channel = VCC_5V0_TST_Channel,0,0},
    {.channel = XCA4001_OUT_Channel,0,0},
};

/*
    @brief      : 单片机ADC采集初始化（采集HALL_GSDJ1,2）
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void adc_init(void)
{
    ADC_InitTypeDef       ADC_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    DMA_InitTypeDef       DMA_InitStructure;
    GPIO_InitTypeDef      GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    //  DMA_DeInit(DMA2_Stream0);
    /* DMA2 Stream0 channe0 configuration **************************************/
    DMA_InitStructure.DMA_Channel = DMA_Channel_0;  
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADC_ConvertedValue;  //存放采集到的AD数据
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    DMA_InitStructure.DMA_BufferSize = Sample_Num*Channel_Num;         //采集6路 ADC数据
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA2_Stream0, &DMA_InitStructure);
    DMA_Cmd(DMA2_Stream0, ENABLE);

    /* Configure ADC1 Channel6 pin as analog input ******************************/
        //先初始化ADC1通道5 IO口
    GPIO_InitStructure.GPIO_Pin = VDDQ_Pin| VDD_CORE_Pin| VCC_1V8_Pin| VCC_2V5_TST_Pin|VCC_3V3_TST_Pin|VCC_5V0_TST_Pin;//PA5 通道5
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//模拟输入
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;//不带上下拉
    GPIO_Init(VDDQ_GPIO_Port, &GPIO_InitStructure);//初始化
    
    GPIO_InitStructure.GPIO_Pin = XCA4001_OUT_Pin;//PA5 通道5
    GPIO_Init(XCA4001_OUT_GPIO_Port, &GPIO_InitStructure);//初始化
    
    /* ADC Common Init **********************************************************/
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);

    //结构体先初始化，否则ADC_ExternalTrigConvEdge异常
    ADC_StructInit(&ADC_InitStructure);
    
    /* ADC1 Init ****************************************************************/
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = Channel_Num;
    ADC_Init(ADC1, &ADC_InitStructure);

    /* ADC1 regular channe6 configuration *************************************/
    ADC_RegularChannelConfig(ADC1, VDDQ_Channel,        1, ADC_SampleTime_3Cycles);
    ADC_RegularChannelConfig(ADC1, VDD_CORE_Channel,    2, ADC_SampleTime_3Cycles);
    ADC_RegularChannelConfig(ADC1, VCC_1V8_Channel,     3, ADC_SampleTime_3Cycles);
    ADC_RegularChannelConfig(ADC1, VCC_2V5_TST_Channel, 4, ADC_SampleTime_3Cycles);
    ADC_RegularChannelConfig(ADC1, VCC_3V3_TST_Channel, 5, ADC_SampleTime_3Cycles);
    ADC_RegularChannelConfig(ADC1, VCC_5V0_TST_Channel, 6, ADC_SampleTime_3Cycles);
    ADC_RegularChannelConfig(ADC1, XCA4001_OUT_Channel, 7, ADC_SampleTime_3Cycles);

    /* Enable DMA request after last transfer (Single-ADC mode) */
    ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

    /* Enable ADC3 DMA */
    ADC_DMACmd(ADC1, ENABLE);

    /* Enable ADC3 */
    ADC_Cmd(ADC1, ENABLE);
    ADC_SoftwareStartConv(ADC1);
}

//获得ADC值
//ch: @ref ADC_channels 
//通道值 0~16取值范围为：ADC_Channel_0~ADC_Channel_16
//返回值:转换结果
u16 adcval_get(u8 ch)
{
    //设置指定ADC的规则组通道，一个序列，采样时间
    //ADC1,ADC通道,480个周期,提高采样时间可以提高精确度
    ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_144Cycles );
    ADC_SoftwareStartConv(ADC1);        //使能指定的ADC1的软件转换启动功能

    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束
    return ADC_GetConversionValue(ADC1);    //返回最近一次ADC1规则组的转换结果
}

//adc采样均值滤波
u16 ReadADCAverageValue(uint16_t Channel)
{
    uint8_t i;
    uint32_t sum = 0;
    for(i=0; i<10; i++)
    {
        sum+=ADC_ConvertedValue[i][Channel];
    }
    return (sum/10);
}

/*
    @brief      : 获取采集HALL_GSDJ1,2的电压值
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void board_voltage_get(_board_monitor_t *sysmon)
{
    for(int i=0; i<Channel_Num; i++)
    {
        //brd_adc[i].rawval = adcval_get(brd_adc[i].channel);
        brd_adc[i].rawval = ReadADCAverageValue(i);
    }

    // 计算电压值
    brd_adc[0].voltage = RAW_TO_VOLTAGE(brd_adc[0].rawval);
    brd_adc[1].voltage = RAW_TO_VOLTAGE(brd_adc[1].rawval);
    brd_adc[2].voltage = RAW_TO_VOLTAGE(brd_adc[2].rawval);
    brd_adc[3].voltage = RAW_TO_VOLTAGE(brd_adc[3].rawval)*2;
    brd_adc[4].voltage = RAW_TO_VOLTAGE(brd_adc[4].rawval)*2;
    brd_adc[5].voltage = RAW_TO_VOLTAGE(brd_adc[5].rawval)*1.474;
    brd_adc[6].voltage = RAW_TO_VOLTAGE(brd_adc[6].rawval);     // VPX12V电流监测
    
    sysmon->VOL_VDDQ        = (u16)(brd_adc[0].voltage*BOARD_FACTOR);
    sysmon->VOL_VDD_CORE    = (u16)(brd_adc[1].voltage*BOARD_FACTOR);
    sysmon->VOL_VCC1V8      = (u16)(brd_adc[2].voltage*BOARD_FACTOR);
    sysmon->VOL_VCC2V5      = (u16)(brd_adc[3].voltage*BOARD_FACTOR);
    sysmon->VOL_VCC3V3      = (u16)(brd_adc[4].voltage*BOARD_FACTOR);
    sysmon->VOL_VCC5V0      = (u16)(brd_adc[5].voltage*BOARD_FACTOR);
    sysmon->VOL_VPX12V_CURR = (brd_adc[6].voltage*2*BOARD_FACTOR);
    #if 1
    TRACE_OUT(DEBUG_OUT,"======================================\r\n");
    TRACE_OUT(DEBUG_OUT,"VDDQ   rawval[%04x] voltage[%d]\r\n", brd_adc[0].rawval, sysmon->VOL_VDDQ);
    TRACE_OUT(DEBUG_OUT,"VDD_CORE rawval[%04x] voltage[%d]\r\n", brd_adc[1].rawval, sysmon->VOL_VDD_CORE);
    TRACE_OUT(DEBUG_OUT,"vcc1v8 rawval[%04x] voltage[%d]\r\n", brd_adc[2].rawval, sysmon->VOL_VCC1V8);
    TRACE_OUT(DEBUG_OUT,"vcc2v5 rawval[%04x] voltage[%d]\r\n", brd_adc[3].rawval, sysmon->VOL_VCC2V5);
    TRACE_OUT(DEBUG_OUT,"vcc3v3 rawval[%04x] voltage[%d]\r\n", brd_adc[4].rawval, sysmon->VOL_VCC3V3);
    TRACE_OUT(DEBUG_OUT,"vcc5v0 rawval[%04x] voltage[%d]\r\n", brd_adc[5].rawval, sysmon->VOL_VCC5V0);
    TRACE_OUT(DEBUG_OUT,"vpx12v rawval[%04x] current[%d]*10mA\r\n", brd_adc[6].rawval, sysmon->VOL_VPX12V_CURR);
    #endif
}

