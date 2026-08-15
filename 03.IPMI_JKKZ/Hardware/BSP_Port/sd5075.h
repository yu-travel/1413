/*
*********************************************************************************************************************
    @brief      : SD5075驱动文件
    @author     : xjq
    @date       : 2024/06/12
    @history    : v1.0
*********************************************************************************************************************
*/

#ifndef __SD5075_H_
#define __SD5075_H_

#include "types_def.h"
#include "myiic.h"


#define SD5075_DEV_ID_1         0x48    // 温度传感器1设备地址
#define SD5075_DEV_ID_2         0x49    // 温度传感器2设备地址
#define SD5075_DEV_ID_3         0x4A    // 温度传感器3设备地址


/* 寄存器定义 */
#define SD5075_REG_00           0x00    //温度结果寄存器 0000H
#define SD5075_REG_01           0x01    //配置寄存器 00H
#define SD5075_REG_02           0x02    //迟滞阈值寄存器 4B00H (75℃ )
#define SD5075_REG_03           0x03    //过温阈值寄存器 5000H (80℃ )
#define SD5075_REG_04           0x04    //单次测温寄存器 XXH

#define SD5075_TEMP_CALC(data)  ((s16)((data&(1<<11))?((int)(data-4096)/16):(data/16)))


typedef struct {
    u8 devid[3];
    s16 tempreture[3];
}_sd5075_t;

extern _sd5075_t tmp_sd5075;


/*
    @brief      : sd5075初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void sd5075_init(void);

/*
    @brief      : sd5075获取板载所有温度传感器数据
    @param[in]  : 
        handler        温度传感器句柄
    @param[out] : none
    @retval     : none
*/
int sd5075_tempreture_get(_sd5075_t *handler);

/*
    @brief      : sd5075设置过温报警值
        @note       建议上下限阈值合理设置
    @param[in]  : 
        handler         温度传感器句柄
        alarm_down      设置过温报警恢复的下限温度，芯片默认值75摄氏度
        alarm_upper     设置过温报警恢复的上限温度，芯片默认值80摄氏度
    @param[out] : none
    @retval     : none
*/
int sd5075_alarm_tempreture_set(_sd5075_t *handler, u8 alarm_down, u8 alarm_upper);


#endif /* __SD5075_H_ */

