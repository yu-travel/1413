#ifndef __APP_CONFIG_H_
#define __APP_CONFIG_H_

#include "types_def.h"

/*
    @brief      : App 层共享类型与协议参数定义
    @note       : 1. 协议参数权威来源: doc/xieyi_2026.8.3.docx
                  2. 帧格式: [帧头 2B][帧长度 2B][帧内容][校验和 2B][帧尾 2B]
                  3. 字节序: 文档前导帧字节列表 (55 AA / 00 02 / AB DE /
                     01 8A / AC BC) 表明帧内 u16 字段按大端组包
                     (高字节在前), app_protocol.c 的 put16_be/rd16_be
                     按大端实现; 待联调确认#3: FPGA 是否严格按文档发送
*/

/* ---------------- 协议参数 ---------------- */
#define PROTO_DEV_NUM        15              /* 协议设备数量 (15 路) */
#define PROTO_HEAD_UP        0x55AAu         /* 上传帧头 (MCU->FPGA) */
#define PROTO_HEAD_DOWN      0xAA55u         /* 下发帧头 (FPGA->MCU) */
#define PROTO_TAIL           0xACBCu         /* 帧尾 (上下行一致) */
#define PROTO_PREAMBLE_WORD  0xABDEu         /* 前导帧读命令字 */
#define PROTO_PREAMBLE_SUM   0x018Au         /* 前导帧校验和 (样例已验证) */
#define FRAME_LEN_CONTENT    0x0060u         /* 帧内容长度 96B; 待确认#1: 文档写 0x5F(95B), 实际内容 90+6=96B, 以 0x60 实现 */
#define PROTO_RX_BUF_LEN     128             /* 接收缓冲, >= 帧总长 104B + 前导 10B */
#define PROTO_TX_BUF_LEN     128             /* 发送缓冲, >= 上传帧总长 104B */

/* ---------------- 采集换算系数 (doc 权威值, 联调可用 Flash k/b 校准修正) ---------------- */
#define ADC_VREF            2.5f        /* LC1258 外部基准电压, 待确认 (联调校准) */
#define ADC_FS_CODE         8388607.0f  /* 2^23-1 满量程码 */
#define ADC_FS_SCALE        1.06f       /* ±1.06VREF 满量程 */
#define MON_COEF_V_5048     8.0f        /* MAC5048 VOUT 分压系数 */
#define MON_COEF_V_5016     11.722f     /* HQEF5016 VOUT: 16*(27.4/37.4) */
#define MON_COEF_IMON_5048  (10.0f/28.2f)   /* MAC5048 IMON 引脚电压系数 */
#define MON_COEF_IMON_5016  0.5f        /* HQEF5016 IMON: 1/2 */
#define MON_COEF_I_R_5048   0.09f       /* MAC5048: 25uA/A * 3.6k */
#define MON_COEF_I_R_5016   0.02f       /* HQEF5016: 10uA/A * 2k */
#define MON_COEF_TEMP_5048  (10.0f/24.7f)   /* MAC5048 VTEMP 引脚电压系数 */
#define MON_COEF_TEMP_5016  0.5f        /* HQEF5016 VTEMP: 1/2 */
#define MON_T_OFFSET_5048   0.2f        /* MAC5048 VTEMP 电压偏置 V */
#define MON_T_SLOPE_5048    0.0121f     /* MAC5048 温敏斜率 V/℃ */
#define MON_T_OFFSET_5016   0.1525f     /* HQEF5016 VTEMP 电压偏置 V */
#define MON_T_SLOPE_5016    0.0087f     /* HQEF5016 温敏斜率 V/℃ */
#define MON_RSENSE_3V3      0.04f       /* XCA4001 3V3 轨采样电阻 Ω */
#define MON_RSENSE_5V0      0.04f       /* XCA4001 5V0 轨采样电阻 Ω */
#define MON_RSENSE_12V0     0.08f       /* XCA4001 12V0 轨采样电阻 Ω */
#define MON_RSENSE_28V0     0.16f       /* XCA4001 28V0 轨采样电阻 Ω */
#define MON_COEF_RAIL_28V0  (167.8f/17.8f)  /* 28V0 恒压源分压系数 */
#define MON_COEF_RAIL_12V0  (195.0f/45.0f)  /* 12V0 恒压源分压系数 */
#define MON_COEF_RAIL_5V0   2.0f        /* 5V0 恒压源分压系数 */
#define MON_COEF_RAIL_3V3   1.0f        /* 3V3 恒压源直读 */
#define FAULT_BAND_MV       150         /* FAULT 分段判定半带宽 mV, 待确认#9 */
#define CAL_NUM_CH          3           /* 每设备 3 组校准: V/I/T */

/* ---------------- 电源控制参数 ---------------- */
#define DEFAULT_I_LIMIT_MA   0           /* 上电默认限流 0mA=关断式, FPGA 须先下发限流再开电, 待联调确认 */

/* ---------------- 周期任务参数 (app_main.c 注册 MultiTimer 周期, 单位 ms) ---------------- */
#define TASK_MONITOR_MS      1           /* 1ms 采集轮询 (DRDY 就绪读样本) */
#define TASK_CONVERT_MS      100         /* 100ms 码值换算+校准 + 告警判定 */
#define TASK_UPLOAD_MS       100         /* 100ms 上传帧组包发送 (MCU->FPGA) */
#define TASK_PROTO_MS        200         /* 200ms 下发帧收发与指令消费 (FPGA->MCU) */
#define TASK_HEARTBEAT_MS    1000        /* 1000ms RTT 心跳日志 (新板无调试 LED) */

/* ---------------- App 共享类型 ---------------- */

/* 设备测量值 (上传帧内容, MCU 采集上传 FPGA) */
typedef struct {
    u16 id;          /* 设备ID 0x0001~0x000F */
    u16 vol_mv;      /* 电压 mV (实际值 x 1000) */
    u16 cur_ma;      /* 电流 mA (实际值 x 1000) */
} dev_measure_t;

/* 设备基准阈值 (下发帧内容, FPGA 下发 MCU) */
typedef struct {
    u16 id;          /* 设备ID 0x0001~0x000F */
    u16 ref_vol_mv;  /* 基准电压 mV */
    u16 ref_cur_ma;  /* 基准电流 mA */
} dev_threshold_t;

/* 电源状态字 (上传/下发帧末三字) */
typedef struct {
    u16 default_state;  /* 电源开关默认状态, bit1~15 对应设备ID 1~15, bit0 预留 */
    u16 switch_state;   /* 电源开关状态, 1=打开 0=关闭 */
    u16 alarm_state;    /* 电源告警状态, 1=告警 0=正常 */
} power_state_t;

#endif /* __APP_CONFIG_H_ */
