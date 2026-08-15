/*
***********************************************************************************************************************
    @brief          : 板载gpio初始化
    @author         : xiongjinqi
    @date           : 2024/07/21
***********************************************************************************************************************
*/
#ifndef __GPIO_H_
#define __GPIO_H_

#include "types_def.h"
#include "main.h"


/*
    @brief      : led灯Gpio初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void led_init(void);


/*
    @brief      : vpx_slotaddr_init
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void vpx_slotaddr_init(void);

/*
    @brief      : i2c_en_ready_init
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void i2c_en_ready_init(void);
/*
    @brief      : sys_reset_init复位引脚初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void sys_reset_init(void);

/*
    @brief      : xc388_en_init
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void xc388_en_init(void);


/*
    @brief      : xc4001_ctrl_init复位及过流检测初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void xc4001_ctrl_init(void);



#endif /* __GPIO_H_ */

