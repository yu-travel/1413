#include "app_power.h"
#include "gda6641.h"
#include "efuse.h"
#include "app_monitor.h"
#include "delay.h"

/*
    @brief      : 电源控制 (App 层): 15 路开关状态机 + DAC 限流换算与同步刷新
    @note       : 1. 上电默认全关 (无 EEPROM 记忆), 默认限流 DEFAULT_I_LIMIT_MA
                   = 0mA (关断式最安全), FPGA 须在开电前下发限流值, 待联调确认
                   2. 开关状态机: 开 → efuse_clear_latch (EN 低 100us 脉冲
                   清锁存, 结束时 EN=1 即已开电并软启动, MAC5048/5016 通用);
                   关 → efuse_off; 位无变化不动作 (避免反复清锁存脉冲)
                   3. 限流换算 (doc 权威): V_CLREF = I_LIMIT(A) x 0.09 (MAC5048)
                   或 x 0.02 (HQEF5016); DAC 码 D = V_CLREF / 2.5 x 65536
                   (GDA6641: VOUT = VREFIN x D / 65536, VREFIN = 2.5V);
                   D > 65535 时钳位 65535 (I_LIMIT 超芯片可表达范围饱和)
                   4. 故障恢复策略: MCU 只上报故障/告警, 开关与清锁存由 FPGA
                   指令驱动 (待确认#6)
                   5. 开关状态字 bit1~15 对应设备 ID 1~15 (1=开), bit0 预留
                   6. g_dac_pin_map 为 const 只读表, Dev 层接口未声明 const,
                   此处强制转换仅去除 const 限定 (驱动不写句柄内容)
*/

/* CLREF 电压换算系数 (doc 权威) */
#define PWR_CLREF_K_5048    0.09f   /* MAC5048: V_CLREF = I_LIMIT(A) x 0.09 */
#define PWR_CLREF_K_5016    0.02f   /* HQEF5016: V_CLREF = I_LIMIT(A) x 0.02 */

/* GDA6641 外部基准与满量程码 */
#define PWR_DAC_VREF        2.5f    /* VREFIN (V) */
#define PWR_DAC_FULL        65536.0f    /* D = V / VREFIN x 65536 */

/* 开关状态字掩码: 仅 bit1~15 有效, bit0 预留清零 */
#define PWR_STATE_MASK      0xFFFEu

/* 当前 15 路开关状态 (bit1~15 = ID1~15, 1=开) */
static u16 s_switch_state;

/* 每路最新 DAC 码值 / 限流 mA 缓存 (power_set_limit 填充, 供诊断打印) */
static u16 s_dac_code[DEV_NUM];
static u16 s_limit_ma[DEV_NUM];

/*
    @brief      : 由设备映射构造电子保险丝句柄
    @note       : MAC5048 无 GOK/GOC, 句柄填 NULL/0;
                  仅 efuse_on/off/clear_latch 消费 en_port/en_pin
    @param[in]  : id  设备 ID (1~15)
    @param[out] : h   句柄输出
    @retval     : none
*/
static void power_build_efuse_handle(dev_id_e id, efuse_handle_t *h)
{
    const dev_map_t *dev = &g_dev_map[id];

    if (h == NULL) {
        return;
    }

    h->en_port = dev->en_port;
    h->en_pin  = dev->en_pin;

    if (dev->chip_type == CHIP_TYPE_HQEF5016) {
        h->gok_port = dev->fault_port[0];   /* GOK (全局故障, 锁存) */
        h->gok_pin  = dev->fault_pin[0];
        h->goc_port = dev->fault_port[1];   /* GOC (稳态过流预警) */
        h->goc_pin  = dev->fault_pin[1];
    } else {
        h->gok_port = NULL;
        h->gok_pin  = 0u;
        h->goc_port = NULL;
        h->goc_pin  = 0u;
    }
}

/*
    @brief      : 限流 mA → V_CLREF → 16 位 DAC 码
    @note       : 上限钳位 65535 (I_LIMIT 超芯片可表达范围时饱和),
                  下限钳 0 (负值防御)
    @param[in]  : chip_type  CHIP_TYPE_xxx
    @param[in]  : i_limit_ma 限流 mA
    @param[out] : none
    @retval     : 16 位 DAC 码
*/
static u16 power_limit_to_dac(u8 chip_type, u16 i_limit_ma)
{
    float k;
    float v_clref;
    float dac_code;

    k = (chip_type == CHIP_TYPE_HQEF5016) ? PWR_CLREF_K_5016 : PWR_CLREF_K_5048;
    v_clref = (float)i_limit_ma / 1000.0f * k;
    dac_code = v_clref / PWR_DAC_VREF * PWR_DAC_FULL;

    if (dac_code >= 65535.0f) {
        return 65535u;      /* 饱和: I_LIMIT 超芯片可表达范围 */
    }
    if (dac_code <= 0.0f) {
        return 0u;
    }
    return (u16)(dac_code + 0.5f);  /* 四舍五入 */
}

/*
    @brief      : 电源控制初始化
    @note       : 1. 4 片 GDA6641 init (POR/CLR/LDAC/SYNC 置高)
                   2. 全部 15 路写默认限流 DEFAULT_I_LIMIT_MA (=0mA 关断式)
                   并 LDAC 同步刷新; FPGA 须在开电前下发限流值, 待联调确认
                   3. EN 全部显式置低 (bsp_board 已初始化为低, 此处显式
                   确保上电全关, 无 EEPROM 记忆)
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void power_init(void)
{
    u8 i;
    u16 id;
    efuse_handle_t eh;

    /* 4 片 DAC 控制线初始化 (const 表强制转换, 驱动不写句柄内容) */
    for (i = 0u; i < 4u; i++) {
        gda6641_init((gda6641_handle_t *)&g_dac_pin_map[i]);
    }

    /* 15 路默认限流写入输入寄存器 (缓存) 并同步刷新 */
    for (id = 1u; id < DEV_NUM; id++) {
        power_set_limit((dev_id_e)id, DEFAULT_I_LIMIT_MA);
    }
    power_flush_limits();

    /* EN 全关 (上电默认全关) */
    s_switch_state = 0u;
    for (id = 1u; id < DEV_NUM; id++) {
        power_build_efuse_handle((dev_id_e)id, &eh);
        efuse_off(&eh);
    }
}

/*
    @brief      : 按 FPGA 下发状态字配置 15 路 EN
    @note       : 1. bit1~15 = 设备 ID 1~15, 1=开 0=关, bit0 预留忽略
                   2. 开: efuse_clear_latch (EN 低 100us → 高: 清锁存并软启动,
                      对 MAC5048/HQEF5016 通用, 结束已开电)
                   3. 关: efuse_off (EN 低)
                   4. 位无变化不动作, 避免反复清锁存脉冲
    @param[in]  : switch_state  开关状态字
    @param[out] : none
    @retval     : none
*/
void power_apply_state(u16 switch_state)
{
    u16 id;
    u16 cur;
    u16 target;
    efuse_handle_t eh;

    switch_state &= PWR_STATE_MASK;     /* bit0 预留清零 */

    for (id = 1u; id < DEV_NUM; id++) {
        target = (u16)((switch_state >> id) & 1u);
        cur = (u16)((s_switch_state >> id) & 1u);
        if (target == cur) {
            continue;                   /* 位无变化, 不动作 */
        }

        power_build_efuse_handle((dev_id_e)id, &eh);
        if (target != 0u) {
            efuse_clear_latch(&eh);     /* 开: 清锁存脉冲, 结束 EN=1 已开电 */
        } else {
            efuse_off(&eh);             /* 关: EN 低 */
        }
    }

    s_switch_state = switch_state;
}

/*
    @brief      : 单路限流配置 (mA), 写 DAC 输入寄存器 (缓存)
    @note       : 1. 由 g_dev_map 取 dac_idx/dac_ch/chip_type
                   2. V_CLREF → 16 位码 → gda6641_write_input, 仅缓存,
                      需 power_flush_limits() 同步刷新输出
                   3. id 越界 (0 或 >= DEV_NUM) 直接返回
    @param[in]  : id          设备 ID (1~15)
    @param[in]  : i_limit_ma  限流 mA
    @param[out] : none
    @retval     : none
*/
void power_set_limit(dev_id_e id, u16 i_limit_ma)
{
    const dev_map_t *dev;
    u16 d;

    if (id <= DEV_INVALID || id >= DEV_NUM) {
        return;
    }

    dev = &g_dev_map[id];
    d = power_limit_to_dac(dev->chip_type, i_limit_ma);

    s_dac_code[id]  = d;
    s_limit_ma[id]  = i_limit_ma;

    gda6641_write_input((gda6641_handle_t *)&g_dac_pin_map[dev->dac_idx],
                        dev->dac_ch, d);
}

/*
    @brief      : 4 片 DAC LDAC 低脉冲同步刷新全部通道输出
    @note       : 各片写入输入寄存器 (缓存) 后调用, 一次刷新 15 路 CLREF
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void power_flush_limits(void)
{
    u8 i;

    for (i = 0u; i < 4u; i++) {
        gda6641_update_all((gda6641_handle_t *)&g_dac_pin_map[i]);
    }
}

/*
    @brief      : 读取当前开关状态字
    @param[in]  : none
    @param[out] : none
    @retval     : 开关状态 (bit1~15 = ID1~15, 1=开)
*/
u16 power_get_switch_state(void)
{
    return s_switch_state;
}

/*
    @brief      : 读取单路故障状态 (用于诊断打印/主动测试判定)
    @note       : MAC5048: 直读 dev_map.fault_port[] (FAULT 开漏低有效, KF 双路
                  任一拉低即故障); HQEF5016: 经 efuse_is_gok_goc() 读 GOK/GOC
                  (任一拉低即故障/过流预警)
    @param[in]  : id  设备 ID (1~15)
    @param[out] : none
    @retval     : 1 = 故障/过流报警; 0 = 正常或 ID 无效
*/
static u8 power_diag_read_fault(u16 id)
{
    const dev_map_t *dev;
    efuse_handle_t  eh;
    u8 j;

    if (id <= DEV_INVALID || id >= DEV_NUM) {
        return 0u;
    }

    dev = &g_dev_map[id];

    if (dev->chip_type == CHIP_TYPE_HQEF5016) {
        power_build_efuse_handle((dev_id_e)id, &eh);
        return efuse_is_gok_goc(&eh);
    }

    /* MAC5048: 直读全部 FAULT 引脚, 任一低=故障 */
    for (j = 0u; j < dev->fault_cnt && j < 2u; j++) {
        if (dev->fault_port[j] != NULL &&
            GPIO_ReadInputDataBit(dev->fault_port[j], dev->fault_pin[j]) == Bit_RESET) {
            return 1u;
        }
    }
    return 0u;
}

/*
    @brief      : 联调诊断: 打印 15 路 EN 电平 / 故障引脚 / DAC 下发值
    @note       : 1. 供 app_diag.c 周期调用 (CHIP_TEST_LOG=1 时);
                   2. EN 电平读取实际引脚 (GPIO_ReadInputDataBit), 可核对接线;
                   3. DAC: 打印 s_dac_code 缓存码值 + 换算电压 (V=code/65536×2.5)
                      + 对应限流 mA (s_limit_ma 缓存);
                   4. 该函数只读不改状态, 安全
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void power_diag_dump(void)
{
    u16 id;

    TRACE_OUT_2(DEBUG_OUT, "[DIAG] switch_state=%04X\r\n", s_switch_state);
    for (id = 1u; id < DEV_NUM; id++) {
        const dev_map_t *dev = &g_dev_map[id];
        u8 en_lvl = (GPIO_ReadInputDataBit(dev->en_port, dev->en_pin) != Bit_RESET) ? 1u : 0u;
        u8 fault  = power_diag_read_fault(id);
        u16 code  = s_dac_code[id];
        s32 mv    = (s32)((float)code / 65536.0f * 2500.0f);   /* V_CLREF mV */

        {
        s32 t10 = (s32)(g_monitor.temp_c[id] * 10.0f);
        if (t10 < 0) t10 = -t10;
        TRACE_OUT_2(DEBUG_OUT, "[DIAG] %02u %-8s EN=%u FLT=%u DAC(U%u.%c)=%u(%d.%03dV,%umA) Meas: I=%u.%03uA V=%u.%03uV T=%d.%dC\r\n",
                  id, (const char *)dev->name, en_lvl, fault,
                  (unsigned)(dev->dac_idx + 14u),                  /* U14..U17 */
                  'A' + (char)dev->dac_ch,
                  code, (int)(mv / 1000), (int)(mv % 1000), s_limit_ma[id],
                  g_monitor.cur_ma[id] / 1000u, g_monitor.cur_ma[id] % 1000u,
                  g_monitor.vol_mv[id] / 1000u, g_monitor.vol_mv[id] % 1000u,
                  (int)(t10 / 10), (int)(t10 % 10));
    }
}

/*
    @brief      : 联调诊断: 15 路逐路主动上电测试 (真实供电!)
    @note       : 1. 每路: 设测试限流 → 开电(清锁存) → 500ms 观测
                     (期间轮询 monitor_task 保持采集不中断) → 读 EN/故障/实测电流
                     → 关电 → 恢复原限流;
                   2. 警告: 会真实给 28V/12V 输出上电, 确认负载可承受且无短路;
                     测试限流 DIAG_TEST_I_LIMIT_MA 见 app_config.h;
                   3. 建议在 bsp_iwdg_init 之前调用 (watchdog 未启动),
                      或观测循环中喂狗; 由 app_diag.c 的 diag_init() 调用;
                   4. 打印每路 PASS/FAIL: 成功=EN=1 且无故障且电流非 0;
                     最后打印统计
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void power_diag_test_seq(void)
{
    u16 id;
    u16 pass = 0u;
    u16 fail = 0u;

    TRACE_OUT_2(DEBUG_OUT, "[DIAG] === efuse active test begin (limit=%u mA) ===\r\n",
              DIAG_TEST_I_LIMIT_MA);

    for (id = 1u; id < DEV_NUM; id++) {
        const dev_map_t *dev = &g_dev_map[id];
        u16 prev_limit = s_limit_ma[id];
        u8  en_lvl;
        u8  fault;
        u32 i;
        u32 t;

        /* 设测试限流并刷新 */
        power_set_limit((dev_id_e)id, DIAG_TEST_I_LIMIT_MA);
        power_flush_limits();

        /* 开电 (bit 变化触发, efuse_clear_latch 清锁存+软启动) */
        power_apply_state((u16)(1u << id));

        /* 500ms 观测: 轮询 ADC 采集 + 周期换算, 保证电流数据新鲜 */
        for (t = 0u; t < 500u; t++) {
            monitor_task();
            if ((t % 100u) == 99u) {
                monitor_convert_all();
            }
            delay_ms(1);
        }

        en_lvl = (GPIO_ReadInputDataBit(dev->en_port, dev->en_pin) != Bit_RESET) ? 1u : 0u;
        fault  = power_diag_read_fault(id);

        /* 成功判定: EN=1 且无故障且实测电流非 0 */
        if (en_lvl != 0u && fault == 0u && g_monitor.cur_ma[id] != 0u) {
            pass++;
        } else {
            fail++;
        }

        {
        s32 t10 = (s32)(g_monitor.temp_c[id] * 10.0f);
        if (t10 < 0) t10 = -t10;
        TRACE_OUT_2(DEBUG_OUT, "[DIAG] %02u %-8s EN=%u FLT=%u I=%u.%03uA V=%u.%03uV T=%d.%dC  %s\r\n",
                  id, (const char *)dev->name, en_lvl, fault,
                  g_monitor.cur_ma[id] / 1000u, g_monitor.cur_ma[id] % 1000u,
                  g_monitor.vol_mv[id] / 1000u, g_monitor.vol_mv[id] % 1000u,
                  (int)(t10 / 10), (int)(t10 % 10),
                  (en_lvl != 0u && fault == 0u && g_monitor.cur_ma[id] != 0u) ? "PASS" : "FAIL");
    }

        /* 关电并恢复原限流 */
        power_apply_state(0u);
        for (i = 0u; i < 100u; i++) {
            monitor_task();
            delay_ms(1);
        }
        power_set_limit((dev_id_e)id, prev_limit);
        power_flush_limits();
    }

    TRACE_OUT_2(DEBUG_OUT, "[DIAG] === efuse active test done: pass=%u fail=%u ===\r\n", pass, fail);
}
