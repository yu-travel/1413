#ifndef __APP_POWER_H_
#define __APP_POWER_H_

#include "app_config.h"
#include "board_map.h"

/*
    @brief      : 电源控制 (App 层): 15 路开关状态机 + DAC 限流换算与同步刷新
    @note       : 1. 上电默认全关 (无 EEPROM 记忆), 默认限流 DEFAULT_I_LIMIT_MA
                   = 0mA (关断式最安全), FPGA 须在开电前下发限流值, 待联调确认
                   2. 开关指令由 FPGA 下发驱动: 开 → EN 高 (自动清锁存+软启动),
                   关 → EN 低; 位无变化不动作 (避免反复清锁存脉冲)
                   3. 限流换算 (doc 权威): V_CLREF = I_LIMIT(A) x 0.09 (MAC5048)
                   或 x 0.02 (HQEF5016); DAC 码 D = V_CLREF / 2.5 x 65536
                   4. 故障恢复策略: MCU 只上报故障/告警, 开关与清锁存由 FPGA
                   指令驱动 (待确认#6)
                   5. 开关状态字 bit1~15 对应设备 ID 1~15 (1=开), bit0 预留
*/

void power_init(void);                          /* 4片DAC初始化(POR/CLR/LDAC高) + 全部默认限流写入并刷新 + EN保持全关 */
void power_apply_state(u16 switch_state);       /* 按 FPGA 下发状态配置 15 路 EN; bit1~15=ID1~15, 1=开 */
void power_set_limit(dev_id_e id, u16 i_limit_ma);  /* 单路限流(mA), 写DAC输入寄存器(缓存) */
void power_flush_limits(void);                  /* 4 片 DAC LDAC 脉冲同步刷新 */
u16  power_get_switch_state(void);              /* 当前开关状态字 */
void power_diag_dump(void);                     /* 联调诊断: 15 路 EN/故障引脚电平 + DAC 下发码值/电压/限流 (app_diag.c 调用) */
void power_diag_test_seq(void);                 /* 联调诊断: 15 路逐路主动上电测试 (真实供电, 需确认负载安全, app_diag.c 调用) */

#endif /* __APP_POWER_H_ */
