#include "app_monitor.h"
#include "bsp_flash.h"
#include "efuse.h"
#include "lc1258.h"
#include "xca4001.h"

/*
    @brief      : 采集与告警 (App 层): 4 片 LC1258 轮询采集 + 物理量换算
                  + Flash 校准 + FAULT 模拟量译码 + 15 路告警判定
    @note       : 1. monitor_task() 每 1ms 调用, 轮询 4 片 ADC DRDY,
                    就绪则读 1 个样本按 CHID (即 AIN) 存入对应原始码数组
                    (CH0=电压 CH1=电流 CH2=温度 CH3=电源/辅助)
                   2. monitor_convert_all() 每 100ms 调用:
                    码 → V_adc → 芯片类型分派换算 → Flash k/b 校准
                    → g_monitor; 再执行 FAULT 译码与告警判定
                    3. 电压/电流经校准后 ×1000 截断取整为 mV/mA 上报协议;
                     温度保留 float 本地监测
                   4. 校准系数索引 = (设备ID-1)*3 + {0=V,1=I,2=T};
                    KF 无 T 槽 (adc_ain_t=0xFF), KF1/KF2 温度共用 KF T 槽系数
                   5. 告警: 阈值比较 (阈值=0 视为未配置不告警)
                    + 硬件故障信号 (低电平=故障): MAC5048 直读 FAULT 引脚,
                    HQEF5016 经 efuse_is_gok_goc() 驱动, 4 轨 XCA4001 ALERT
                    经 xca4001_alert_active() 驱动 (只记 aux_alarm)
                    + FAULT 译码非 NORMAL
                   6. 电压阈值语义: vol_mv/cur_ma 为绝对值, 与 FPGA 下发
                    基准阈值直接比较 (大于即告警)
*/

/* 每片 ADC 16 路单端通道 (AIN0~15) */
#define MON_AIN_NUM     16u

/* 告警状态字 bit 位序: bit1 = ID1 ... bit15 = ID15, bit0 预留 */
#define MON_ALARM_BIT(id)   (u16)(1u << (id))

/* 原始码数组 (索引 = AIN 通道号) + 有效标志位图 (bitN = AIN N 已采到) */
static s32 s_raw_v[MON_AIN_NUM];
static s32 s_raw_i[MON_AIN_NUM];
static s32 s_raw_t[MON_AIN_NUM];
static s32 s_raw_ch3[MON_AIN_NUM];
static u16 s_raw_v_valid;
static u16 s_raw_i_valid;
static u16 s_raw_t_valid;
static u16 s_raw_ch3_valid;

/* Flash 45 组校准系数 (bsp_flash_cal_read 填充, 失败时默认 k=1 b=0) */
static float s_k[CAL_NUM];
static float s_b[CAL_NUM];

/* 基准阈值 (来自协议下发帧), 索引 = 设备ID-1 */
static dev_threshold_t s_thr[PROTO_DEV_NUM];

/* 15 路告警状态字 */
static u16 s_alarm_state;

/* ADC 初始化失败掩码: bit0~3 = ADC_CH0~CH3 (lc1258_init ID 校验失败),
   由 monitor_init 置位; CH0 电压/CH1 电流失效会使对应设备报告假 0 数据,
   告警判定时据此强制置位设备告警 (见 monitor_alarm_eval) */
static u8 s_adc_fault_mask;

/* 全局监测数据实例 */
monitor_data_t g_monitor;

/* FAULT 分段电平中心 (mV), 索引与 fault_type_e 一致 */
static const float s_fault_level_mv[6] = {
    100.0f,     /* FAULT_T_NORMAL  0.1V 正常 */
    300.0f,     /* FAULT_T_COMP    0.3V 比较器故障 */
    600.0f,     /* FAULT_T_MOS     0.6V MOS 损坏 */
    900.0f,     /* FAULT_T_OTP_OVP 0.9V 过温过压 */
    1200.0f,    /* FAULT_T_OC      1.2V 过流 */
    1500.0f     /* FAULT_T_SCP     1.5V 短路 */
};

/* XCA4001 轨电流采样电阻 (Ω), 索引与 rail_cur_a 一致: 0=3V3 1=12V0 2=5V0 3=28V0 */
static const float s_rail_rsense[4] = {
    MON_RSENSE_3V3, MON_RSENSE_12V0, MON_RSENSE_5V0, MON_RSENSE_28V0
};

/* 恒压源分压系数, 索引与 rail_vol_v 一致: 0=28V0 1=12V0 2=5V0 3=3V3 */
static const float s_rail_coef[4] = {
    MON_COEF_RAIL_28V0, MON_COEF_RAIL_12V0, MON_COEF_RAIL_5V0, MON_COEF_RAIL_3V3
};

/*
    @brief      : 24bit 补码原始码 → ADC 输入引脚电压
    @note       : V_adc = code / 8388607 × (1.06 × ADC_VREF)
    @param[in]  : code  24bit 二进制补码 (lc1258_read_channel 返回值)
    @param[out] : none
    @retval     : 引脚电压 V
*/
static float monitor_raw_to_vadc(s32 code)
{
    return (float)code / ADC_FS_CODE * (ADC_FS_SCALE * ADC_VREF);
}

/*
    @brief      : float → u16 截断取整 (负值归 0, 超上限截断, 直接舍去小数)
    @note       : 上报 FPGA 的数据约定 ×1000 后截断小数 (不四舍五入)
    @param[in]  : val  待转换值
    @param[out] : none
    @retval     : u16 结果
*/
static u16 monitor_float_to_u16(float val)
{
    if (val <= 0.0f) {
        return 0u;
    }
    if (val >= 65535.0f) {
        return 65535u;
    }
    return (u16)val;    /* 截断小数 */
}

/*
    @brief      : FAULT 模拟量电压分段译码
    @note       : 分段中点 ± FAULT_BAND_MV 判定, 从高分段向低分段;
                  带外电平按正常处理 (待联调确认#9)
    @param[in]  : vadc  FAULT 引脚电压 V
    @param[out] : none
    @retval     : fault_type_e 译码结果
*/
static u8 monitor_fault_decode(float vadc)
{
    float mv = vadc * 1000.0f;
    s32   i;

    for (i = (s32)(ARRAY_SIZE(s_fault_level_mv) - 1u); i >= 0; i--) {
        float center = s_fault_level_mv[i];
        float half = (float)FAULT_BAND_MV;
        if (mv >= center - half && mv <= center + half) {
            return (u8)i;
        }
    }
    return FAULT_T_NORMAL;
}

/*
    @brief      : 单设备原始码 → 物理量 (换算 + 校准)
    @note       : 电压/电流由 g_dev_map[].adc_ain_v/i 取码;
                  温度: 普通设备用 adc_ain_t, KF 遍历 g_t_map 子表
                  (sub==1=KF1, sub==2=KF2), KF1/KF2 共用 KF T 槽系数
    @param[in]  : id  设备 ID (1~15)
    @param[out] : none
    @retval     : none
*/
static void monitor_convert_device(u16 id)
{
    const dev_map_t *dev = &g_dev_map[id];
    u8    ain;
    s32   code;
    float vadc;
    float physical;
    u16   cal_idx = (u16)((id - 1u) * CAL_NUM_CH);

    /* ---- 电压 ---- */
    ain = dev->adc_ain_v;
    if (ain < MON_AIN_NUM && (s_raw_v_valid & MON_ALARM_BIT(ain)) != 0u) {
        code = s_raw_v[ain];
        vadc = monitor_raw_to_vadc(code);
        if (dev->chip_type == CHIP_TYPE_HQEF5016) {
            physical = vadc * MON_COEF_V_5016;
        } else {
            physical = vadc * MON_COEF_V_5048;
        }
        physical = s_k[cal_idx + 0u] * physical + s_b[cal_idx + 0u];  /* Flash 校准 */
        g_monitor.vol_mv[id] = monitor_float_to_u16(physical * 1000.0f);
    }

    /* ---- 电流 ---- */
    ain = dev->adc_ain_i;
    if (ain < MON_AIN_NUM && (s_raw_i_valid & MON_ALARM_BIT(ain)) != 0u) {
        code = s_raw_i[ain];
        vadc = monitor_raw_to_vadc(code);
        if (dev->chip_type == CHIP_TYPE_HQEF5016) {
            physical = vadc * MON_COEF_IMON_5016 / MON_COEF_I_R_5016;
        } else {
            physical = vadc * MON_COEF_IMON_5048 / MON_COEF_I_R_5048;
        }
        physical = s_k[cal_idx + 1u] * physical + s_b[cal_idx + 1u];  /* Flash 校准 */
        g_monitor.cur_ma[id] = monitor_float_to_u16(physical * 1000.0f);
    }

    /* ---- 温度 ---- */
    if (dev->adc_ain_t < MON_AIN_NUM) {
        /* 普通设备: 直接取 adc_ain_t 槽 */
        ain = dev->adc_ain_t;
        if ((s_raw_t_valid & MON_ALARM_BIT(ain)) != 0u) {
            code = s_raw_t[ain];
            vadc = monitor_raw_to_vadc(code);
            if (dev->chip_type == CHIP_TYPE_HQEF5016) {
                physical = (vadc * MON_COEF_TEMP_5016 - MON_T_OFFSET_5016) / MON_T_SLOPE_5016;
            } else {
                physical = (vadc * MON_COEF_TEMP_5048 + MON_T_OFFSET_5048) / MON_T_SLOPE_5048;
            }
            physical = s_k[cal_idx + 2u] * physical + s_b[cal_idx + 2u];  /* Flash 校准 */
            g_monitor.temp_c[id] = physical;
        }
    } else if (id == DEV_KF) {
        /* KF 双温度点: 遍历 g_t_map 找 sub==1 (KF1) / sub==2 (KF2) 的 AIN 取码
           (不硬编码通道号, 通道由 bsp_board.c 的 g_t_map 表唯一指定),
           KF 无独立 T 槽 (adc_ain_t=0xFF), KF1/KF2 共用 KF 的 T 槽校准系数:
           KF1 → temp_c[DEV_KF] 主槽 + kf1_temp_c; KF2 → kf2_temp_c */
        u8 i2;

        for (i2 = 0u; i2 < (u8)ARRAY_SIZE(g_t_map); i2++) {
            if (g_t_map[i2].dev != DEV_KF || g_t_map[i2].sub == 0u) {
                continue;
            }
            if ((s_raw_t_valid & MON_ALARM_BIT(i2)) == 0u) {
                continue;
            }
            vadc = monitor_raw_to_vadc(s_raw_t[i2]);
            physical = (vadc * MON_COEF_TEMP_5048 + MON_T_OFFSET_5048) / MON_T_SLOPE_5048;
            physical = s_k[cal_idx + 2u] * physical + s_b[cal_idx + 2u];
            if (g_t_map[i2].sub == 1u) {
                g_monitor.temp_c[id] = physical;    /* KF1 主槽 */
                g_monitor.kf1_temp_c = physical;
            } else {
                g_monitor.kf2_temp_c = physical;
            }
        }
    }
}

/*
    @brief      : CH3 (U11) 电源/辅助通道原始码 → 物理量
    @note       : AIN0~3 XCA4001 轨电流 (3V3/12V0/5V0/28V0),
                  AIN4/5 GSDJ_HAL_CH0/CH1 电压直读,
                  AIN6~9 恒压源 (28V0/12V0/5V0/3V3),
                  AIN10/11 DYGY/GSDJ FAULT 模拟量电压 + 分段译码
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
static void monitor_convert_aux(void)
{
    u8    i;
    float vadc;

    /* AIN0~3 XCA4001 轨电流: I = V_adc / (100 × R_sense) */
    for (i = 0u; i < 4u; i++) {
        if ((s_raw_ch3_valid & MON_ALARM_BIT(i)) != 0u) {
            vadc = monitor_raw_to_vadc(s_raw_ch3[i]);
            g_monitor.rail_cur_a[i] = vadc / (100.0f * s_rail_rsense[i]);
        }
    }

    /* AIN4/5 GSDJ_HAL_CH0/CH1 (电压直读) */
    if ((s_raw_ch3_valid & MON_ALARM_BIT(4u)) != 0u) {
        g_monitor.hal_ch0_v = monitor_raw_to_vadc(s_raw_ch3[4]);
    }
    if ((s_raw_ch3_valid & MON_ALARM_BIT(5u)) != 0u) {
        g_monitor.hal_ch1_v = monitor_raw_to_vadc(s_raw_ch3[5]);
    }

    /* AIN6~9 恒压源 */
    for (i = 0u; i < 4u; i++) {
        u8 ain = (u8)(6u + i);
        if ((s_raw_ch3_valid & MON_ALARM_BIT(ain)) != 0u) {
            vadc = monitor_raw_to_vadc(s_raw_ch3[ain]);
            g_monitor.rail_vol_v[i] = vadc * s_rail_coef[i];
        }
    }

    /* AIN10/11 FAULT 模拟量: 电压 + 分段译码 */
    if ((s_raw_ch3_valid & MON_ALARM_BIT(10u)) != 0u) {
        g_monitor.dygy_fault_v = monitor_raw_to_vadc(s_raw_ch3[10]);
        g_monitor.dygy_fault_type = monitor_fault_decode(g_monitor.dygy_fault_v);
    }
    if ((s_raw_ch3_valid & MON_ALARM_BIT(11u)) != 0u) {
        g_monitor.gsdj_fault_v = monitor_raw_to_vadc(s_raw_ch3[11]);
        g_monitor.gsdj_fault_type = monitor_fault_decode(g_monitor.gsdj_fault_v);
    }
}

/*
    @brief      : 15 路告警判定
    @note       : 1. 阈值比较: vol_mv > ref_vol_mv 或 cur_ma > ref_cur_ma
                    (阈值为 0 视为未配置不告警)
                   2. 硬件信号 (低电平=故障): MAC5048 直读 g_dev_map[].fault_port[]
                    (FAULT 引脚, 设计约定); HQEF5016 (DYGY/GSDJ) 经
                    efuse_is_gok_goc() 驱动 (handle 由 dev_map 行构造,
                    fault_port[0]=GOK fault_port[1]=GOC, 见 board_map.h)
                   3. FAULT 译码: DYGY/GSDJ 类型非 NORMAL → 置对应告警位
                   4. ADC 采集通道失效: CH0/CH1 ID 校验失败置
                    s_adc_fault_mask, 全 15 路对应物理量为假 0,
                    置告警位防 FPGA 误信 (CH2 温度/CH3 辅助失效不置)
                   5. VCC_*_ALERT 4 路不映射到 15 路 (协议 bit0 预留),
                    经 xca4001_alert_active() 驱动读取, 记录到 aux_alarm 位0~3
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
static void monitor_alarm_eval(void)
{
    u16 id;
    u16 alarm = 0u;

    for (id = 1u; id < DEV_NUM; id++) {
        const dev_map_t *dev = &g_dev_map[id];
        const dev_threshold_t *thr = &s_thr[id - 1u];
        u8 fault = 0u;
        u8 j;

        /* 阈值比较 (阈值=0 未配置) */
        if (thr->ref_vol_mv != 0u && g_monitor.vol_mv[id] > thr->ref_vol_mv) {
            fault = 1u;
        }
        if (thr->ref_cur_ma != 0u && g_monitor.cur_ma[id] > thr->ref_cur_ma) {
            fault = 1u;
        }

        /* 硬件故障信号 (低电平=故障) */
        if (dev->chip_type == CHIP_TYPE_HQEF5016) {
            /* HQEF5016 (DYGY/GSDJ): GOK/GOC 经 Dev/efuse 驱动读取,
               按 board_map.h 约定 handle 由 dev_map 行局部构造:
               fault_port[0]/fault_pin[0]=GOK, fault_port[1]/fault_pin[1]=GOC */
            efuse_handle_t efuse_h;

            efuse_h.en_port  = dev->en_port;
            efuse_h.en_pin   = dev->en_pin;
            efuse_h.gok_port = dev->fault_port[0];
            efuse_h.gok_pin  = dev->fault_pin[0];
            efuse_h.goc_port = dev->fault_port[1];
            efuse_h.goc_pin  = dev->fault_pin[1];
            if (efuse_is_gok_goc(&efuse_h) != 0u) {
                fault = 1u;
            }
        } else {
            /* MAC5048: 直读 FAULT 引脚 (设计约定, 不经过 efuse 驱动) */
            for (j = 0u; j < dev->fault_cnt; j++) {
                if (dev->fault_port[j] != 0 &&
                    GPIO_ReadInputDataBit(dev->fault_port[j], dev->fault_pin[j]) == Bit_RESET) {
                    fault = 1u;
                }
            }
        }

        /* FAULT 模拟量译码 (DYGY: AIN10, GSDJ: AIN11) */
        if (dev->adc_ain_fault == 10u && g_monitor.dygy_fault_type != FAULT_T_NORMAL) {
            fault = 1u;
        }
        if (dev->adc_ain_fault == 11u && g_monitor.gsdj_fault_type != FAULT_T_NORMAL) {
            fault = 1u;
        }

        /* ADC 采集通道失效视为故障: 电压恒由 CH0 (U2) 采集、电流恒由
           CH1 (U5) 采集, 对应片 ID 校验失败时全 15 路该物理量为假 0,
           置告警位防止 FPGA 把假零数据当有效测量;
           CH2 (温度) 失效仅本地温度监测缺失, 不置告警;
           CH3 (辅助量) 失效仅影响辅助量, 不进 15 路告警字 */
        if ((s_adc_fault_mask & (u8)MON_ALARM_BIT(ADC_IDX_CH0)) != 0u ||
            (s_adc_fault_mask & (u8)MON_ALARM_BIT(ADC_IDX_CH1)) != 0u) {
            fault = 1u;
        }

        if (fault != 0u) {
            alarm |= MON_ALARM_BIT(id);
        }
    }

    s_alarm_state = alarm;

    /* 4 轨 XCA4001 ALERT 辅助告警 (低电平=告警, 待联调确认极性),
       不进 15 路告警字, 位序与 rail_cur_a 一致 (CH3 AIN0~3 = 3V3/12V0/5V0/28V0):
       bit0=3V3 bit1=12V0 bit2=5V0 bit3=28V0
       经 Dev/xca4001 驱动读取, handle 用 board_map.h 引脚宏局部构造
       (RST 引脚传对应 RST 宏仅作 handle 完整性, 此处只读 ALERT) */
    xca4001_handle_t xca_h[4] = {
        { VCC_3V3_RST_PORT,  VCC_3V3_RST_PIN,  VCC_3V3_ALERT_PORT,  VCC_3V3_ALERT_PIN  },  /* bit0 3V3  */
        { VCC_12V0_RST_PORT, VCC_12V0_RST_PIN, VCC_12V0_ALERT_PORT, VCC_12V0_ALERT_PIN },  /* bit1 12V0 */
        { VCC_5V0_RST_PORT,  VCC_5V0_RST_PIN,  VCC_5V0_ALERT_PORT,  VCC_5V0_ALERT_PIN  },  /* bit2 5V0  */
        { VCC_28V0_RST_PORT, VCC_28V0_RST_PIN, VCC_28V0_ALERT_PORT, VCC_28V0_ALERT_PIN },  /* bit3 28V0 */
    };
    u8 i2;

    g_monitor.aux_alarm = 0u;
    for (i2 = 0u; i2 < 4u; i2++) {
        if (xca4001_alert_active(&xca_h[i2]) != 0u) {
            g_monitor.aux_alarm |= (u8)MON_ALARM_BIT(i2);
        }
    }
}

/*
    @brief      : 采集初始化
    @note       : 4 片 LC1258 依次 lc1258_init (含 RST + ID 校验), 成功后
                  lc1258_start 进入 Auto-Scan; 单片校验失败置
                  s_adc_fault_mask 对应位降级运行 (不阻塞系统启动,
                  CH0/CH1 失效设备由告警判定强制置位, 见 monitor_alarm_eval);
                  读取 Flash 45 组校准系数, 失败时 bsp_flash_cal_read
                  内部已填充默认 k=1 b=0;
                  g_adc_pin_map 为 const 只读表, Dev 层接口未声明 const,
                  此处强制转换仅去除 const 限定 (驱动不写句柄内容)
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void monitor_init(void)
{
    u8 i;

    s_adc_fault_mask = 0u;

    for (i = 0u; i < 4u; i++) {
        if (lc1258_init((lc1258_handle_t *)&g_adc_pin_map[i]) != 0u) {
            lc1258_start((lc1258_handle_t *)&g_adc_pin_map[i]);
        } else {
            /* 单片 ID 校验失败: 记录故障掩码降级运行, 对应设备
               告警置位防假零数据 (见 monitor_alarm_eval) */
            TRACE_OUT(DEBUG_OUT, "monitor: ADC%d lc1258_init fail\r\n", i);
            s_adc_fault_mask |= (u8)MON_ALARM_BIT(i);
        }
    }

    if (bsp_flash_cal_read(s_k, s_b, CAL_NUM) == 0u) {
        TRACE_OUT(DEBUG_OUT, "monitor: flash cal read fail, use k=1 b=0\r\n");
    }
}

/*
    @brief      : 采集周期任务 (每 1ms 调用)
    @note       : 轮询 4 片 ADC DRDY, 就绪则读 1 样本, 按 ADC 实例分派:
                  CH0→s_raw_v, CH1→s_raw_i, CH2→s_raw_t, CH3→s_raw_ch3;
                  chid 即 AIN 通道号, 越界 (≥16) 丢弃
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void monitor_task(void)
{
    u8  i;
    u8  chid;
    s32 raw;

    for (i = 0u; i < 4u; i++) {
        if (lc1258_data_ready((lc1258_handle_t *)&g_adc_pin_map[i]) == 0u) {
            continue;
        }

        raw = lc1258_read_channel((lc1258_handle_t *)&g_adc_pin_map[i], &chid);
        if (chid >= MON_AIN_NUM) {
            continue;                       /* 状态字节 CHID 异常, 丢弃 */
        }

        switch (i) {
        case ADC_IDX_CH0:
            s_raw_v[chid] = raw;
            s_raw_v_valid |= MON_ALARM_BIT(chid);
            break;
        case ADC_IDX_CH1:
            s_raw_i[chid] = raw;
            s_raw_i_valid |= MON_ALARM_BIT(chid);
            break;
        case ADC_IDX_CH2:
            s_raw_t[chid] = raw;
            s_raw_t_valid |= MON_ALARM_BIT(chid);
            break;
        default:                            /* ADC_IDX_CH3 */
            s_raw_ch3[chid] = raw;
            s_raw_ch3_valid |= MON_ALARM_BIT(chid);
            break;
        }
    }
}

/*
    @brief      : 物理量换算与告警判定 (每 100ms 调用)
    @note       : 15 设备码→物理量 (换算 + Flash 校准) → CH3 辅助通道
                  → FAULT 译码 → 告警状态字
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void monitor_convert_all(void)
{
    u16 id;

    for (id = 1u; id < DEV_NUM; id++) {
        monitor_convert_device(id);
    }

    monitor_convert_aux();
    monitor_alarm_eval();
}

/*
    @brief      : 更新基准阈值 (来自协议下发帧)
    @note       : 按 thr[i].id 拷贝到 s_thr[ID-1] 槽
    @param[in]  : thr  15 组基准阈值数组
    @param[out] : none
    @retval     : none
*/
void monitor_set_threshold(const dev_threshold_t *thr)
{
    u8 i;

    if (thr == NULL) {
        return;
    }

    for (i = 0u; i < PROTO_DEV_NUM; i++) {
        if (thr[i].id >= 1u && thr[i].id <= PROTO_DEV_NUM) {
            s_thr[thr[i].id - 1u] = thr[i];
        }
    }
}

/*
    @brief      : 读取 15 路告警状态字
    @param[in]  : none
    @param[out] : none
    @retval     : 告警位 (bit1~15 对应 ID1~15, bit0 预留)
*/
u16 monitor_get_alarm_state(void)
{
    return s_alarm_state;
}

/*
    @brief      : 填 15 组测量值供协议组包
    @param[in]  : none
    @param[out] : m  至少 PROTO_DEV_NUM 长度的 dev_measure_t 数组
    @retval     : 1 成功, 0 失败 (出参无效)
*/
u8 monitor_measure_to_protocol(dev_measure_t *m)
{
    u8 i;

    if (m == NULL) {
        return 0u;
    }

    for (i = 0u; i < PROTO_DEV_NUM; i++) {
        u16 id = (u16)(i + 1u);
        m[i].id = id;
        m[i].vol_mv = g_monitor.vol_mv[id];
        m[i].cur_ma = g_monitor.cur_ma[id];
    }

    return 1u;
}
