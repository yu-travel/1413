/*
*********************************************************************************************************************
    @brief      : LTC2990驱动文件
    @author     : xjq
    @date       : 2024/06/12
    @history    : v1.0
*********************************************************************************************************************
*/

#ifndef __LTC2990_H_
#define __LTC2990_H_

#include "types_def.h"
#include "myiic.h"

#define LTC2990_DEV_ID         0x98    // 电流传感器1设备地址

/* 寄存器定义 */
#define LTC2990_REG_00         0x00    //STATUS R Indicates BUSY State, Conversion Status
#define LTC2990_REG_01         0x01    //CONTROL R/W Controls Mode, Single/Repeat, Celsius/Kelvin
#define LTC2990_REG_02         0x02    //TRIGGER** R/W Triggers an Conversion
#define LTC2990_REG_03         0x03    //N/A Unused Address
#define LTC2990_REG_04         0x04    //TINT (MSB) R Internal Temperature MSB
#define LTC2990_REG_05         0x05    //TINT (LSB) R Internal Temperature LSB
#define LTC2990_REG_06         0x06    //V1 (MSB) R V1, V1 – V2 or TR1 MSB
#define LTC2990_REG_07         0x07    //V1 (LSB) R V1, V1 – V2 or TR1 LSB
#define LTC2990_REG_08         0x08    //V2 (MSB) R V2, V1 – V2 or TR1 MSB
#define LTC2990_REG_09         0x09    //V2 (LSB) R V2, V1 – V2 or TR1 LSB
#define LTC2990_REG_0A         0x0A    //V3 (MSB) R V3, V3 – V4 or TR2 MSB
#define LTC2990_REG_0B         0x0B    //V3 (LSB) R V3, V3 – V4 or TR2 LSB
#define LTC2990_REG_0C         0x0C    //V4 (MSB) R V4, V3 – V4 or TR2 MSB
#define LTC2990_REG_0D         0x0D    //V4 (LSB) R V4, V3 – V4 or TR2 LSB
#define LTC2990_REG_0E         0x0E    //VCC (MSB) R VCC MSB
#define LTC2990_REG_0F         0x0F    //VCC (LSB) R VCC LSB

/*
    @brief      : 获取电流值
*/
s8 ltc2990_current_get(void);

/*
    @brief      : ltc2990初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void ltc2990_init(void);



#endif /* __LTC2990_H_ */

