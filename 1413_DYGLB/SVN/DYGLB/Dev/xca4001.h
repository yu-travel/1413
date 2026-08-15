#ifndef __XCA4001_H_
#define __XCA4001_H_

#include "board_map.h"

/*
    @brief      : XCA4001 电流监控芯片模式切换与锁存清除驱动
    @note       : 1. 句柄 xca4001_handle_t 在本文件定义 (board_map.h 无此类型);
                     bsp_board.c 已完成 RESET 推挽输出 (初始高 = 锁存模式)
                     与 ALERT 上拉输入初始化, 本驱动只做电平操作
                  2. RESET=1 锁存模式: 过流 ALERT 拉低后锁存, 需手动清除
                  3. RESET=0 自恢复模式: 过流 ALERT 拉低, 电流回落自动恢复高,
                     适合硬件中断实时监测场景
                  4. ALERT 开漏输出, 低电平有效 (V_OUT > V_LIMIT 过流)
*/
typedef struct {
    GPIO_TypeDef *rst_port;   /* 模式选择/清锁存 RESET   OUT_PP (清锁存低脉冲有效) */
    u16           rst_pin;
    GPIO_TypeDef *alert_port; /* 过流报警 ALERT   IN 上拉 (低 = 过流) */
    u16           alert_pin;
} xca4001_handle_t;

void xca4001_set_latch_mode(xca4001_handle_t *h);
void xca4001_set_auto_mode(xca4001_handle_t *h);
void xca4001_clear_latch(xca4001_handle_t *h);  /* 脉冲结束后 RESET 回到锁存模式 (RST=1), 如需自恢复模式须再调 xca4001_set_auto_mode() */
u8   xca4001_alert_active(xca4001_handle_t *h);

#endif /* __XCA4001_H_ */
