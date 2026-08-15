/*
***********************************************************************************************************************
    @brief          : 板载adc初始化
    @author         : xiongjinqi
    @date           : 2024/07/21
***********************************************************************************************************************
*/
#ifndef __ADC_H_
#define __ADC_H_

#include "types_def.h"

#define RAW_TO_VOLTAGE(rawval)      (float)((float)(rawval*3.3f)/4096.0f)

typedef struct {
    u8 channel;
    u16 rawval;
    float voltage;
}_board_vol_t;

typedef struct {
    u8 slot_addr;
    s16 tempreture;
    u16 VOL_VDDQ; // 电压扩大100倍
    u16 VOL_VDD_CORE; // 电压扩大100倍
    u16 VOL_VCC1V8; // 电压扩大100倍
    u16 VOL_VCC2V5;  // 电压扩大100倍
    u16 VOL_VCC3V3;  // 电压扩大100倍
    u16 VOL_VCC5V0;  // 电压扩大100倍
    u16 VOL_VPX12V_CURR;    // VPX12V供电电流扩大100倍
    _board_vol_t voltage;
}_board_monitor_t;


/*
    @brief      : 单片机ADC采集初始化（采集HALL_GSDJ1,2）
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void adc_init(void);

/*
    @brief      : 获取采集HALL_GSDJ1,2的电压值
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void board_voltage_get(_board_monitor_t *sysmon);



#endif /* __ADC_H_ */

