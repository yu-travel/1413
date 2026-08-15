#include "types_def.h"
#include "stm32f4xx.h"
#include "usart.h"
#include "delay.h"
#include "I2C.h"
#include "GX21.h"
#include "adc.h"
#include "led.h"
#include "iwdg.h"
#include "gpio.h"
#include "timer.h"
#include "softtimer.h"
#include "ipmi_protocol.h"
#include "main.h"

_board_monitor_t sysmon;
_pwr_abnormal_t pwr_abnormal = {
    .AB_ID_PWR_EEPROM       = AB_PWR_EEPROM,
    .AB_ID_PWR_TEMPRETURE   = ID_TEMPRETURE,
    .AB_ID_PWR_VCC12V_1     = ID_VCC12V_1,
    .AB_ID_PWR_VCC12V_2     = ID_VCC12V_2,
    .AB_ID_PWR_VCC3V3       = ID_VCC3V3,
    .AB_PWR_EEPROM          = AB_STA_DEV_OK,
    .AB_PWR_TEMPRETURE      = AB_STA_DEV_OK,
    .AB_PWR_VCC12V_1        = AB_STA_DEV_OK,
    .AB_PWR_VCC12V_2        = AB_STA_DEV_OK,
    .AB_PWR_VCC3V3          = AB_STA_DEV_OK,
};

u16 vol12_1,vol12_2,vol33,cur12_1,cur12_2,cur33;
float v12_1,v12_2,v33,c12_1,c12_2,c33;


void sysmon_abnormal_set(u16 vol12_1, u16 vol12_2, u16 vol33, _pwr_abnormal_t *abnormal)
{
    
    if(vol12_1 < 1000)// 低于10V
    {
        abnormal->AB_PWR_VCC12V_1 = AB_STA_UNDERVOLTAGE;
    }
    if(vol12_1 > 1600) // 高于16V
    {
        abnormal->AB_PWR_VCC12V_1 = AB_STA_OVERVOLTAGE;
    }
    if(vol12_1 > 1000 && vol12_1 < 1600)
    {
        abnormal->AB_PWR_VCC12V_1 = AB_STA_DEV_OK;
    }

    if(vol12_2 < 1000)
    {
        abnormal->AB_PWR_VCC12V_2 = AB_STA_UNDERVOLTAGE;
    }
    if(vol12_2 > 1600)
    {
        abnormal->AB_PWR_VCC12V_2 = AB_STA_OVERVOLTAGE;
    }
    if(vol12_2 > 1000 && vol12_2 < 1600)
    {
        abnormal->AB_PWR_VCC12V_2 = AB_STA_DEV_OK;
    }

    if(vol33 < 100) // 低于1V
    {
        abnormal->AB_PWR_VCC3V3 = AB_STA_UNDERVOLTAGE;
    }
    if(vol33 > 360) // 高于3.6V
    {
        abnormal->AB_PWR_VCC3V3 = AB_STA_OVERVOLTAGE;
    }
    if(vol33 > 100 && vol33 < 360)
    {
        abnormal->AB_PWR_VCC3V3 = AB_STA_DEV_OK;
    }
}

void sysmon_data_get(_board_monitor_t *sysmon)
{
    u8 err = 0;
    sysmon->tempreture = (s16)(Lm75a_get_temp(0x90)*100);
    
    vol12_1 = ReadADCAverageValue(0);
    vol12_2 = ReadADCAverageValue(1);
    vol33   = ReadADCAverageValue(2);
    cur12_1 = ReadADCAverageValue(3);
    cur12_2 = ReadADCAverageValue(4);
    cur33   = ReadADCAverageValue(5);

    v12_1=(float)((vol12_1 * (3.3 /4096))-1.25)*9.6;
    //v12_1=(v12_1>12) ? 12 : v12_1;
    
    v12_2=(float)vol12_2 * (3.3 /4096)*4.8;
    //v12_2=(v12_2>12) ? 12 : v12_2;
    
    v33=(float)vol33* (3.3 /4096)*1.32;
    //v33=(v33>3.3) ? 3.3 : v33;

    
    c12_1=(float)(cur12_1*(3.3/4096)*15.56-19.694);
    c12_1=(c12_1<0) ? 0 : c12_1;
    //c12_1=(cur12_1<192) ? 0 : c12_1;
    
    

    c12_2=(float)(cur12_2*(3.3/4096)*5.2632 - 0.1053);
    c12_2=(c12_2<0) ? 0 : c12_2;
    //c12_2=(cur12_2<34) ? 0 : c12_2;
    
    c33=(float)(cur33*(3.3/4096)*2.6715-0.01043);
    c33=(c33<0) ? 0 : c33;
    //c33=(cur33<24) ? 0 : c33;

    sysmon->vcc12v_1 = (u16)(v12_1*100);
    sysmon->vcc12v_2 = (u16)(v12_2*100);
    sysmon->vcc12v_1_curr = (u16)(c12_1*100);
    sysmon->vcc12v_2_curr = (u16)(c12_2*100);

    sysmon->vcc3v3 = (u16)(v33*100);
    sysmon->vcc3v3_curr = (u16)(c33*100);
	
    /* 异常判断 add by xjq */
    sysmon_abnormal_set(sysmon->vcc12v_1, sysmon->vcc12v_2, sysmon->vcc3v3, &pwr_abnormal);
    
    #if 1
    TRACE_OUT(DEBUG_OUT, "===================================\r\n");
    TRACE_OUT(DEBUG_OUT, "tempreture : %d\r\n", sysmon->tempreture);

    TRACE_OUT(DEBUG_OUT, "voltage V12_1: %d\r\n", (u16)(v12_1*100));
    TRACE_OUT(DEBUG_OUT, "current V12_1: %d*10mA\r\n", (u16)(c12_1*100));
    
    TRACE_OUT(DEBUG_OUT, "voltage V12_2: %d\r\n",(u16)(v12_2*100));
    TRACE_OUT(DEBUG_OUT, "current V12_2: %d*10mA\r\n",(u16)(c12_2*100));
    
    TRACE_OUT(DEBUG_OUT, "voltage V3.3: %d\r\n", (u16)(v33*100));
    TRACE_OUT(DEBUG_OUT, "current V3.3: %d*10mA\r\n", (u16)(c33*100));
    #endif
    
//    if(v12_1<5||v12_2<5||v33<1)
//    {
//        GPIO_SetBits(GPIOB, GPIO_Pin_5);
//        err=GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_5);
//        TRACE_OUT(DEBUG_OUT, "Error: The Voltage too low!!!\r\n");
//        TRACE_OUT(DEBUG_OUT, "ERR-GPIO-VOL:%d\r\n", err);
//    }
}


int main(void)
{
    float temp=0;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    delay_init(168);
    uart_init(115200);
    TRACE_OUT(DEBUG_OUT, "<PWR> System init ......\r\n");
    
    I2C3_Init();
    ADC_DMA_Config();
    err_init();

    led_init();
    vpx_slotaddr_init();
    i2c_en_ready_init();
    sys_reset_init();
    timer2_int_init(TIM_1KHZ, TIMER_PRESCALER);     //时间分辨率1ms
    
    /* 中间件初始化 */
    softtimer_init();
    ipmi_protocol_init();
    
    //IWDG_Init(4, 800);  // 超时时间1.6s
    
    /* 启用I2C中断 */
    //I2C1_INT_ENABLE();
    //I2C2_INT_ENABLE();

    TRACE_OUT(DEBUG_OUT, "<PWR> System init completed, enter loop ......\r\n");
    while(1)
    {
        softtimer_loop();
        ipmi_loop();
        //periph_i2c_test();
    }

}




