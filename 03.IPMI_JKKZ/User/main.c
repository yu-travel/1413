#include "main.h"
#include "delay.h"
#include "usart.h"
#include "softtimer.h"
// #include "at8591t.h"
#include "timer.h"
#include "adc.h"
#include "periph_i2c.h"
#include "ipmi_protocol.h"
#include "types_def.h"

_board_monitor_t sysmon = {0};
_jkkz_abnormal_t jkkz_abnormal = {
    .AB_ID_JKKZ_TEMPRETURE  = ID_TEMPRETURE,
    .AB_ID_JKKZ_EEPROM      = AB_JKKZ_EEPROM,
    .AB_ID_JKKZ_VCC1V2      = ID_VCC1V2,
    .AB_ID_JKKZ_VCC1V5      = ID_VCC1V5,
    .AB_ID_JKKZ_VCC1V8      = ID_VCC1V8,
    .AB_ID_JKKZ_VCC1V0      = ID_VCC1V0,
    .AB_ID_JKKZ_VCC2V5      = ID_VCC2V5,
    .AB_ID_JKKZ_VCC3V3      = ID_VCC3V3,
    .AB_JKKZ_TEMPRETURE     = AB_STA_DEV_OK,   
    .AB_JKKZ_EEPROM         = AB_STA_DEV_OK,       
    .AB_JKKZ_VCC1V2         = AB_STA_DEV_OK,      
    .AB_JKKZ_VCC1V5         = AB_STA_DEV_OK,       
    .AB_JKKZ_VCC1V8         = AB_STA_DEV_OK,       
    .AB_JKKZ_VCC1V0         = AB_STA_DEV_OK,       
    .AB_JKKZ_VCC2V5         = AB_STA_DEV_OK,       
    .AB_JKKZ_VCC3V3         = AB_STA_DEV_OK,       
};

void jkkz_abnormal_set(u16 value, u8 adnormal_id)
{
    switch(adnormal_id)
    {
        case ID_TEMPRETURE :
            // todo nothing
        break;
        case AB_XHCL_EEPROM:
            // todo nothing
        break;
        case ID_VCC1V2:
            if(value > LIMIT_VOL_VCC1V5) // 1.5V
            {
                jkkz_abnormal.AB_JKKZ_VCC1V2 = AB_STA_OVERVOLTAGE;
            }
            else if(value < LIMIT_VOL_VCC1V0) // 1.0V
            {
                jkkz_abnormal.AB_JKKZ_VCC1V2 = AB_STA_UNDERVOLTAGE;
            }
            else
            {
                jkkz_abnormal.AB_JKKZ_VCC1V2 = AB_STA_DEV_OK;
            }
        break;
        case ID_VCC1V5:
            if(value > LIMIT_VOL_VCC1V8) // 1.8V
            {
                jkkz_abnormal.AB_JKKZ_VCC1V5 = AB_STA_OVERVOLTAGE;
            }
            else if(value < LIMIT_VOL_VCC1V2) // 1.2V
            {
                jkkz_abnormal.AB_JKKZ_VCC1V5 = AB_STA_UNDERVOLTAGE;
            }
            else
            {
                jkkz_abnormal.AB_JKKZ_VCC1V5 = AB_STA_DEV_OK;
            }
        break;
        case ID_VCC1V8:
            if(value > LIMIT_VOL_VCC2V0) // 2.0V
            {
                jkkz_abnormal.AB_JKKZ_VCC1V8 = AB_STA_OVERVOLTAGE;
            }
            else if(value < LIMIT_VOL_VCC1V5) // 1.5V
            {
                jkkz_abnormal.AB_JKKZ_VCC1V8 = AB_STA_UNDERVOLTAGE;
            }
            else
            {
                jkkz_abnormal.AB_JKKZ_VCC1V8 = AB_STA_DEV_OK;
            }
        break;
        case ID_VCC1V0:
            if(value > LIMIT_VOL_VCC1V2) // 1.5V
            {
                jkkz_abnormal.AB_JKKZ_VCC1V0 = AB_STA_OVERVOLTAGE;
            }
            else if(value < LIMIT_VOL_VCC0V8) // 0.8V
            {
                jkkz_abnormal.AB_JKKZ_VCC1V0 = AB_STA_UNDERVOLTAGE;
            }
            else
            {
                jkkz_abnormal.AB_JKKZ_VCC1V0 = AB_STA_DEV_OK;
            }
        break;
        case ID_VCC2V5:
            if(value > LIMIT_VOL_VCC2V8) // 2.8V
            {
                jkkz_abnormal.AB_JKKZ_VCC2V5 = AB_STA_OVERVOLTAGE;
            }
            else if(value < 230) // 1.0V
            {
                jkkz_abnormal.AB_JKKZ_VCC2V5 = AB_STA_UNDERVOLTAGE;
            }
            else
            {
                jkkz_abnormal.AB_JKKZ_VCC2V5 = AB_STA_DEV_OK;
            }
        break;
        case ID_VCC3V3:
            if(value > LIMIT_VOL_VCC3V6) // 3.6V
            {
                jkkz_abnormal.AB_JKKZ_VCC3V3 = AB_STA_OVERVOLTAGE;
            }
            else if(value < LIMIT_VOL_VCC3V0) // 3.0V
            {
                jkkz_abnormal.AB_JKKZ_VCC3V3 = AB_STA_UNDERVOLTAGE;
            }
            else
            {
                jkkz_abnormal.AB_JKKZ_VCC3V3 = AB_STA_DEV_OK;
            }

        break;
    }
}


int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0); // 设置系统中断优先级分组2
    delay_init(168);                                // 初始化延时函数
    uart3_init(115200);                             // debug口
    usart1_init(115200);                            // 联通FPGA
    TRACE_OUT(DEBUG_OUT, "system init ......\r\n");
    led_init();
    vpx_slotaddr_init();
    i2c_en_ready_init();
    sys_reset_init();
    xc388_en_init();
    xc4001_ctrl_init();
    adc_init();
    i2c_all_init(); // i2c初始化

    timer2_int_init(TIM_1KHZ, TIMER_PRESCALER);

    local_address_get(&sysmon.slot_addr);
    softtimer_init();
    ipmi_protocol_init();
    
    while (1)
    {
        softtimer_loop();
        ipmi_loop();
        // periph_i2c_test();
        // at9555_test();
        // eeprom_test();
    }
}
