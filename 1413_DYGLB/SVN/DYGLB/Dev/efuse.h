#ifndef __EFUSE_H_
#define __EFUSE_H_

#include "board_map.h"

/*
    @brief      : 电子保险丝 (MAC5048 / HQEF5016) 使能与锁存清除驱动
    @note       : 1. 句柄 efuse_handle_t 在本文件定义 (board_map.h 无此类型);
                     bsp_board.c 已完成 EN 推挽输出 (初始低) 与 GOK/GOC
                     上拉输入初始化, 本驱动只做电平操作
                  2. EN 高电平有效; 故障锁存后拉低 EN 再拉高可清除锁存
                     并重新软启动 (两芯片行为一致)
                  3. MAC5048: 故障由 FAULT 开漏拉低指示, 该引脚由 App 层
                     经 dev_map_t.fault_port[] 直读, 不经过本驱动,
                     其 handle 的 gok_port/goc_port 填 NULL/0
                  4. HQEF5016: GOK 全局故障开漏拉低 (锁存), GOC 稳态过流
                     预警开漏拉低 (自动恢复), 由 efuse_is_fault() 读取
*/
typedef struct {
    GPIO_TypeDef *en_port;   /* 使能输出 EN    OUT_PP (高有效) */
    u16           en_pin;
    GPIO_TypeDef *gok_port;  /* 全局故障 GOK   IN 上拉 (仅 HQEF5016 用, MAC5048 填 NULL/0) */
    u16           gok_pin;
    GPIO_TypeDef *goc_port;  /* 过流预警 GOC   IN 上拉 (仅 HQEF5016 用, MAC5048 填 NULL/0) */
    u16           goc_pin;
} efuse_handle_t;

void efuse_on(efuse_handle_t *h);
void efuse_off(efuse_handle_t *h);
void efuse_clear_latch(efuse_handle_t *h);
u8   efuse_is_fault(efuse_handle_t *h);

#endif /* __EFUSE_H_ */
