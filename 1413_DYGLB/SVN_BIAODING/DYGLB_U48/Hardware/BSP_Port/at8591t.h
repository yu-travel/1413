/*
***********************************************************************************************************************
    @brief          : 板载gpio扩展操作
    @author         : xiongjinqi
    @date           : 2024/07/21
***********************************************************************************************************************
*/
#ifndef __AT8591T_H_
#define __AT8591T_H_

#include "main.h"

/* ADC I2C1 ADDR */
#define AT8591T_DEV_1_ID     0x98
#define AT8591T_DEV_2_ID     0x94
#define AT8591T_DEV_3_ID     0x92
#define AT8591T_DEV_4_ID     0x9C

/* ADC I2C2 ADDR */
#define AT8591T_DEV_5_ID     0x98
#define AT8591T_DEV_6_ID     0x94
#define AT8591T_DEV_7_ID     0x92
#define AT8591T_DEV_8_ID     0x9C


typedef struct {
    _myiic_t *i2cbus;
    u8       devid;
    u8       adcdata[4];
}_at8591t_t;


#define CURR_XCA4001_CONVERT(rawval)        (float)((float)((rawval*3.3)/256)*2.0)    // XCA4001 电流转换
#define CURR_28V_JCXJ_CONVERT(rawval)       (float)((float)((rawval*3.3)/256)*2.0)
#define CURR_12V_WAOXJ_CONVERT(rawval)      (float)((float)((rawval*3.3)/256)*2.0)
#define CURR_28V_BQXJ_CONVERT(rawval)       (float)((float)((rawval*3.3)/256)*2.0)
#define CURR_12V_DYGY_B_CONVERT(rawval)     (float)((float)((rawval*3.3)/256)*2.0)
#define CURR_28V_FFXJ_CONVERT(rawval)       (float)((float)((rawval*3.3)/256)*2.0)
#define CURR_12V28V_BF1_CONVERT(rawval)     (float)((float)((rawval*3.3)/256)*2.0)
#define CURR_12V28V_BF2_CONVERT(rawval)     (float)((float)((rawval*3.3)/256)*2.0)
#define CURR_28V_TSGY_CONVERT(rawval)       (float)((float)((rawval*3.3)/256)*2.0)
#define CURR_28V_QGSJ_CONVERT(rawval)       (float)((float)((rawval*3.3)/256)*2.0)
#define CURR_12V28V_HJJC1_CONVERT(rawval)   (float)((float)((rawval*3.3)/256)*2.0)
#define CURR_12V28V_HJJC2_CONVERT(rawval)   (float)((float)((rawval*3.3)/256)*2.0)
#define CURR_12V28V_HJJC3_CONVERT(rawval)   (float)((float)((rawval*3.3)/256)*2.0)
#define CURR_12V_GSDJ1_CONVERT(rawval)      (float)((float)((rawval*3.3)/256)*2.0)
#define CURR_12V_GSDJ2_CONVERT(rawval)      (float)((float)((rawval*3.3)/256)*2.0)
#define CURR_12V_XGHTM_CONVERT(rawval)      (float)((float)((rawval*3.3)/256)*2.0)
#define CURR_28V_KF_CONVERT(rawval)         (float)((float)((rawval*3.3)/256)*4.0)


#define VOL_28V_JCXJ_CONVERT(rawval)        (float)((float)((rawval*3.3)/256)*28.0)    // 28V_JCXJ电压转换
#define VOL_12V_WAOXJ_CONVERT(rawval)       (float)((float)((rawval*3.3)/256)*12.0)
#define VOL_28V_BQXJ_CONVERT(rawval)        (float)((float)((rawval*3.3)/256)*28.0)
#define VOL_12V_DYGY_B_CONVERT(rawval)      (float)((float)((rawval*3.3)/256)*12.0)
#define VOL_28V_FFXJ_CONVERT(rawval)        (float)((float)((rawval*3.3)/256)*28.0)
#define VOL_12V28V_BF1_CONVERT(rawval)      (float)((float)((rawval*3.3)/256)*12.0)
#define VOL_12V28V_BF2_CONVERT(rawval)      (float)((float)((rawval*3.3)/256)*12.0)
#define VOL_28V_TSGY_CONVERT(rawval)        (float)((float)((rawval*3.3)/256)*28.0)
#define VOL_28V_QGSJ_CONVERT(rawval)        (float)((float)((rawval*3.3)/256)*28.0)
#define VOL_12V28V_HJJC1_CONVERT(rawval)    (float)((float)((rawval*3.3)/256)*12.0)
#define VOL_12V28V_HJJC2_CONVERT(rawval)    (float)((float)((rawval*3.3)/256)*12.0)
#define VOL_12V28V_HJJC3_CONVERT(rawval)    (float)((float)((rawval*3.3)/256)*12.0)
#define VOL_12V_GSDJ1_CONVERT(rawval)       (float)((float)((rawval*3.3)/256)*12.0)
#define VOL_12V_GSDJ2_CONVERT(rawval)       (float)((float)((rawval*3.3)/256)*12.0)
#define VOL_12V_XGHTM_CONVERT(rawval)       (float)((float)((rawval*3.3)/256)*12.0)
#define VOL_28V_KF_CONVERT(rawval)          (float)((float)((rawval*3.3)/256)*28.0)


#define AT8591T_RAW_CONVERT(name, board, currraw, volraw)      \
    do{                                         \
        board->VOL_##name.current = CURR_##name##_CONVERT(currraw);   \
        board->VOL_##name.voltage = VOL_##name##_CONVERT(volraw);     \
    }while(0)

#define AT8591T_PRINT(name, board, currraw, volraw)      \
    do{                                                  \
        TRACE_OUT(DEBUG_OUT, "%-20s currraw[%d] volraw[%d] curr[%d] vol[%d]\r\n", \
                 #name, currraw, volraw, (int)((board->VOL_##name.current)*1000),   \
                (int)((board->VOL_##name.voltage)*10)); \
    }while(0)

typedef struct {
    float voltage;
    float current;
    u8 sensor_id;
}_monitor_t;

typedef struct {
    _monitor_t VOL_28V_JCXJ;
    _monitor_t VOL_12V_WAOXJ;
    _monitor_t VOL_28V_BQXJ;
    _monitor_t VOL_12V_DYGY_B;
    _monitor_t VOL_28V_FFXJ;
    _monitor_t VOL_12V28V_BF1;
    _monitor_t VOL_12V28V_BF2;
    _monitor_t VOL_28V_TSGY;
    _monitor_t VOL_28V_QGSJ;
    _monitor_t VOL_12V28V_HJJC1;
    _monitor_t VOL_12V28V_HJJC2;
    _monitor_t VOL_12V28V_HJJC3;
    _monitor_t VOL_12V_GSDJ1;
    _monitor_t VOL_12V_GSDJ2;
    _monitor_t VOL_12V_XGHTM;
    _monitor_t VOL_28V_KF;
}_board_voltage_t;

extern _board_voltage_t board_adc;


/*
    @brief      : Gpio扩展AT8591T初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void at8591t_init(void);

/*
    @brief      : Gpio扩展AT8591T初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void at8591t_output_config(u16 dev1_data, u16 dev2_data);


#endif /* __AT8591T_H_ */


