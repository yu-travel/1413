#ifndef __BSP_BOARD_H_
#define __BSP_BOARD_H_

#include "board_map.h"

/*
    @brief      : DYGLB 板级初始化
                  - 15 路电源 EN 输出 (GPIOD, 初始全低断电)
                  - 故障/GOK/GOC/ALERT 状态输入 (上拉)
                  - XCA4001 RESET 输出 (初始高)
                  - DAC (GDA6641 x4) / ADC (LC1258 x4) 通道组引脚
                  - 关闭 JTAG 保留 SWD, 释放 PA15
                  g_dev_map/g_t_map/g_dac_pin_map/g_adc_pin_map
                  实例表定义见 bsp_board.c (声明见 board_map.h)
*/

void bsp_board_init(void);

#endif /* __BSP_BOARD_H_ */
