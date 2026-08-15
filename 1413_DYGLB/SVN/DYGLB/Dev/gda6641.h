#ifndef __GDA6641_H_
#define __GDA6641_H_

#include "board_map.h"

/*
    @brief      : GDA6641 四通道 16 位电压输出 DAC 驱动 (GPIO 位操作模拟 SPI)
    @note       : 1. 引脚结构体 gda6641_handle_t (gda6641_pin_t) 由 board_map.h 定义,
                     bsp_board.c 已完成 GPIO 方向与空闲电平初始化
                     (SCLK=0, SYNC/LDAC/POR/CLR=1, DIN=0), 本驱动只做协议时序操作
                  2. SPI Mode1: SCLK 空闲低, DIN 在 SCLK 下降沿被锁存
                  3. 32bit 帧 MSB 先行: DB31~28 无效, DB27~24 命令码,
                     DB23~20 通道地址, DB19~4 16 位 DA 值, DB3~0 辅助配置位
                  4. 命令码: 0000=仅写输入寄存器(缓存), 0001=仅刷新输出,
                     0010/0011=写+同步刷新, 0111=软复位
                  5. 通道地址: 0000=VOUTA, 0001=VOUTB, 0010=VOUTC, 0011=VOUTD,
                     1111=全部通道
*/

void gda6641_init(gda6641_handle_t *h);
void gda6641_write(gda6641_handle_t *h, u8 ch, u16 d);
void gda6641_write_input(gda6641_handle_t *h, u8 ch, u16 d);
void gda6641_update_all(gda6641_handle_t *h);
void gda6641_clear(gda6641_handle_t *h);
void gda6641_reset(gda6641_handle_t *h);

#endif /* __GDA6641_H_ */
