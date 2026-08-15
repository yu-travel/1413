#ifndef __APP_MONITOR_H_
#define __APP_MONITOR_H_

#include "app_config.h"
#include "board_map.h"

/*
    @brief      : 采集与告警 (App 层): 4 片 LC1258 轮询采集 + 物理量换算
                  + Flash 校准 + FAULT 模拟量译码 + 15 路告警判定
    @note       : 1. 原始码 → 电压: V_adc = code / 8388607 × (1.06 × ADC_VREF)
                   2. 物理换算按芯片类型分派 (MAC5048 / HQEF5016),
                      系数集中在 app_config.h (doc 权威值)
                   3. Flash 45 组 k/b 校准: 索引 = (设备ID-1)*3 + {0=V,1=I,2=T},
                      KF1/KF2 温度共用 KF 的 T 槽系数
                   4. 告警状态字 bit1~15 对应设备 ID 1~15, bit0 预留
                   5. FAULT 模拟量 (DYGY/GSDJ) 电压分段译码:
                      0.1V 正常 / 0.3V 比较器 / 0.6V MOS / 0.9V 过温过压
                      / 1.2V 过流 / 1.5V 短路 (带宽 ±FAULT_BAND_MV)
*/

/* FAULT 模拟量分段译码类型 */
typedef enum {
    FAULT_T_NORMAL = 0,    /* 0.1V 正常 */
    FAULT_T_COMP,          /* 0.3V 比较器故障 */
    FAULT_T_MOS,           /* 0.6V MOS 损坏 */
    FAULT_T_OTP_OVP,       /* 0.9V 过温过压 */
    FAULT_T_OC,            /* 1.2V 过流 */
    FAULT_T_SCP            /* 1.5V 短路 */
} fault_type_e;

/* 设备监测数据 (每 100ms 刷新一次) */
typedef struct {
    /* 设备实测值 (索引 = 设备ID, [0] 不用) */
    u16 vol_mv[DEV_NUM];      /* 协议上报电压 mV */
    u16 cur_ma[DEV_NUM];      /* 协议上报电流 mA */
    float temp_c[DEV_NUM];    /* 主温度点 ℃ (本地监测); KF 槽 = KF1 温度 */
    float kf1_temp_c;         /* KF1 温度 (AIN15) */
    float kf2_temp_c;         /* KF2 温度 (AIN6) */
    /* CH3 辅助监测量 */
    float rail_cur_a[4];      /* XCA4001 轨电流: 0=3V3 1=12V0 2=5V0 3=28V0 */
    float rail_vol_v[4];      /* 恒压源: 0=28V0 1=12V0 2=5V0 3=3V3 */
    float hal_ch0_v, hal_ch1_v;    /* GSDJ_HAL_CH0 (AIN4) / CH1 (AIN5), 电压直读 */
    float dygy_fault_v, gsdj_fault_v;    /* FAULT 模拟量电压 (AIN10/AIN11) */
    u8   dygy_fault_type, gsdj_fault_type; /* 0正常 1比较器 2MOS 3过温过压 4过流 5短路 */
    u8   aux_alarm;           /* 4 轨 XCA4001 ALERT 辅助告警 (不进 15 路告警字):
                                 bit0=3V3 bit1=5V0 bit2=28V0 bit3=12V0 */
} monitor_data_t;

extern monitor_data_t g_monitor;      /* 全局实例 */

void monitor_init(void);        /* 4片ADC lc1258_init(校验ID) + start; 读flash校准; 失败TRACE_OUT */
void monitor_task(void);        /* 每周期(1ms)调用: 各ADC DRDY就绪则读1样本存原始码 */
void monitor_convert_all(void); /* 周期(100ms)调用: 码→物理量(换算+校准) + 告警判定 */
void monitor_set_threshold(const dev_threshold_t *thr); /* 更新基准阈值(来自协议下发帧) */
u16  monitor_get_alarm_state(void); /* 返回 15 路告警位 (bit1~15 对应 ID1~15) */
u8   monitor_measure_to_protocol(dev_measure_t *m);    /* 填 15 组 id/vol_mv/cur_ma 供协议组包 */

#endif /* __APP_MONITOR_H_ */
