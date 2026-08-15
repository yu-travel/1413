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
