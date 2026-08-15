/*
***********************************************************************************************************************
    @brief          : 板载gx21m15温度传感器
    @author         : xiongjinqi
    @date           : 2024/07/21
***********************************************************************************************************************
*/
#ifndef __GX21M15_H_
#define __GX21M15_H_

#include "periph_i2c.h"
#include "main.h"



typedef struct {
    _i2c_interrpt_t *i2cbus;
    u16 rawval;
    s16 temp;   // 扩大100倍之后的值
}_tempreture_t;


/*
    @brief      : 获取平均温度值
*/
s16 gx21m15_temp_avg(void);



#endif /* __GX21M15_H_ */

