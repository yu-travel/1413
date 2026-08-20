# ADC 诊断输出格式更新记录

## 修改时间
2026-08-20

## 修改目的
将所有 DIAG 诊断输出重构为「原始采样/真实输出」+「校准后物理量/期望输出」两段式结构，便于硬件联调逐项核对。

## 修改文件
- `App/app_config.h` — `TASK_DIAG_MS` 1000ms → 5000ms
- `App/app_monitor.c` — `monitor_diag_dump()` 追加校准物理量段
- `App/app_power.c` — `power_diag_dump()` 每行追加实测值；`power_diag_test_seq()` 增补电压/温度
- `App/app_diag.c` — `diag_task()` 加显式分隔符
- `doc/ADC_Diag_Output.md` — 本文档

---

## 核心变更

| 项目 | 修改前 | 修改后 |
|------|--------|--------|
| 诊断周期 | 1000ms | **5000ms** (5秒) |
| ADC 原始输出 | `a00=码(V_adc)` | `a00(名称)=码(V_adc)` (保留) |
| 校准物理量 | 无独立段 | **新增独立段** `=== 校准后物理量 (g_monitor) ===` |
| Power DAC 输出 | `DAC=码(V_CLREF,限流)` | 追加 `实测: I= V= T=` |
| 主动测试输出 | `I= PASS/FAIL` | 追加 `V= T=` |
| 模块分隔 | 无 | **新增** `===== ADC Raw Sampling =====` 等 |

---

## 输出结构示例 (每 5 秒一次)

```
[DIAG] ===== ADC Raw Sampling =====
[DIAG] U2(ADC_CH0(V)): a00(DTJ_V)=12345(1.234V) a01(SFXJ1_V)=23456(2.345V) ...
[DIAG] U5(ADC_CH1(I)): a00(HWXJ3_I)=... a01(DTJ_I)=... ...
[DIAG] U8(ADC_CH2(T)): a00(WAOXJ_TEMP)=... a01(DYGY_TEMP)=... a06(KF2_TEMP)=... a15(KF1_TEMP)=... ...
[DIAG] U11(ADC_CH3(AUX)): a00(3V3_CUR)=... a01(12V0_CUR)=... a10(DYGY_FLT)=... a11(GSDJ_FLT)=... ...
[DIAG] 15设备物理量: (原有格式保留)
[DIAG] rails A/V: ...
[DIAG] HAL/FAULT: ...
[DIAG] alarm=...

[DIAG] === 校准后物理量 (g_monitor) ===
[DIAG] 设备实测:
[DIAG]   01 28V_KF   V=28.123V I=1.234A T=25.0C
[DIAG]   02 28V_DTJ  V=28.001V I=0.500A T=24.5C
[DIAG]   ...
[DIAG]   KF1=25.1C KF2=24.8C
[DIAG] 辅助量:
[DIAG]   rails A: 3V3=0.100A 12V0=0.500A 5V0=0.200A 28V0=1.000A
[DIAG]   rails V: 28V0=28.1V 12V0=12.0V 5V0=5.0V 3V3=3.3V
[DIAG]   HAL: CH0=1.200V CH1=0.800V
[DIAG]   FAULT: DYGY=0.100V(NORMAL) GSDJ=0.100V(NORMAL)

[DIAG] ===== Power DAC & Pin Status =====
[DIAG] switch_state=0001
[DIAG] 01 28V_KF   EN=1 FLT=0 DAC(U17.B)=32768(1.250V,1000mA) 实测: I=1.000A V=28.000V T=25.0C
[DIAG] 02 28V_DTJ  EN=0 FLT=0 DAC(U14.D)=0(0.000V,0mA) 实测: I=0.000A V=0.000V T=24.5C
...

[DIAG] ===== XCA4001 Alert =====
[DIAG] XCA4001 ALERT: 3V3=ok 12V0=ok 5V0=ok 28V0=ok
```

---

## 4 片 ADC 完整通道映射表 (不变)

### ADC_CH0 (U2, 电压监测) — GPIOH
| AIN | 通道名 | 对应设备 | 备注 |
|-----|--------|----------|------|
| 0   | DTJ_V  | DEV_DTJ (28V_DTJ) | |
| 1   | SFXJ1_V | DEV_SFXJ1 (28V_SFXJ1) | |
| 2   | SFXJ2_V | DEV_SFXJ2 (28V_SFXJ2) | |
| 3   | HWXJ3_V | DEV_HWXJ3 (28V_HWXJ3) | |
| 4   | HJJC3_V | DEV_HJJC3 (28V_HJJC3) | |
| 5   | DYGY_V  | DEV_DYGY (12V_DYGY) | HQEF5016 |
| 6   | WAOXJ_V | DEV_WAOXJ (28V_WAOXJ) | |
| 7   | GSDJ_V  | DEV_GSDJ (12V_GSDJ) | HQEF5016 |
| 8   | KF_V    | DEV_KF (28V_KF) | 双 MAC5048 共用电压通道 |
| 9   | PD_V    | DEV_PD (28V_PD) | |
| 10  | HJJC1_V | DEV_HJJC1 (28V_HJJC1) | |
| 11  | QGSJ_V  | DEV_QGSJ (28V_QGSJ) | |
| 12  | HWXJ2_V | DEV_HWXJ2 (28V_HWXJ2) | |
| 13  | HWXJ1_V | DEV_HWXJ1 (28V_HWXJ1) | |
| 14  | HJJC2_V | DEV_HJJC2 (28V_HJJC2) | |
| 15  | ---     | 未使用 | 保留 |

### ADC_CH1 (U5, 电流监测) — GPIOE
| AIN | 通道名 | 对应设备 | 备注 |
|-----|--------|----------|------|
| 0   | HWXJ3_I | DEV_HWXJ3 (28V_HWXJ3) | |
| 1   | DTJ_I   | DEV_DTJ (28V_DTJ) | |
| 2   | SFXJ2_I | DEV_SFXJ2 (28V_SFXJ2) | |
| 3   | HJJC3_I | DEV_HJJC3 (28V_HJJC3) | |
| 4   | SFXJ1_I | DEV_SFXJ1 (28V_SFXJ1) | |
| 5   | WAOXJ_I | DEV_WAOXJ (28V_WAOXJ) | |
| 6   | DYGY_I  | DEV_DYGY (12V_DYGY) | HQEF5016 |
| 7   | GSDJ_I  | DEV_GSDJ (12V_GSDJ) | HQEF5016 |
| 8   | HJJC2_I | DEV_HJJC2 (28V_HJJC2) | |
| 9   | HWXJ1_I | DEV_HWXJ1 (28V_HWXJ1) | |
| 10  | HWXJ2_I | DEV_HWXJ2 (28V_HWXJ2) | |
| 11  | QGSJ_I  | DEV_QGSJ (28V_QGSJ) | |
| 12  | HJJC1_I | DEV_HJJC1 (28V_HJJC1) | |
| 13  | PD_I    | DEV_PD (28V_PD) | |
| 14  | KF_I    | DEV_KF (28V_KF) | 双 MAC5048 共用电流通道 |
| 15  | ---     | 未使用 | 保留 |

### ADC_CH2 (U8, 温度监测) — GPIOF
对应 `g_t_map[]` 表，索引 = AIN，KF 拆分 KF1/KF2

| AIN | 通道名 | 对应设备 | 备注 |
|-----|--------|----------|------|
| 0   | WAOXJ_TEMP | DEV_WAOXJ | |
| 1   | DYGY_TEMP  | DEV_DYGY | |
| 2   | SFXJ1_TEMP | DEV_SFXJ1 | |
| 3   | HJJC3_TEMP | DEV_HJJC3 | |
| 4   | SFXJ2_TEMP | DEV_SFXJ2 | |
| 5   | DTJ_TEMP   | DEV_DTJ | |
| 6   | KF2_TEMP   | DEV_KF | KF 第 2 路温度 |
| 7   | HWXJ3_TEMP | DEV_HWXJ3 | |
| 8   | QGSJ_TEMP  | DEV_QGSJ | |
| 9   | HJJC1_TEMP | DEV_HJJC1 | |
| 10  | HWXJ1_TEMP | DEV_HWXJ1 | |
| 11  | HWXJ2_TEMP | DEV_HWXJ2 | |
| 12  | HJJC2_TEMP | DEV_HJJC2 | |
| 13  | PD_TEMP    | DEV_PD | |
| 14  | GSDJ_TEMP  | DEV_GSDJ | |
| 15  | KF1_TEMP   | DEV_KF | KF 第 1 路温度 (主槽) |

### ADC_CH3 (U11, 电源/辅助监测) — GPIOF
与 `monitor_convert_aux()` 硬编码功能一致

| AIN | 通道名 | 信号含义 | 备注 |
|-----|--------|----------|------|
| 0   | 3V3_CUR  | 3V3 轨电流 | XCA4001 采样电阻 |
| 1   | 12V0_CUR | 12V0 轨电流 | |
| 2   | 5V0_CUR  | 5V0 轨电流 | |
| 3   | 28V0_CUR | 28V0 轨电流 | |
| 4   | HAL0_V   | GSDJ HAL CH0 电压 | 直读电压 |
| 5   | HAL1_V   | GSDJ HAL CH1 电压 | 直读电压 |
| 6   | 28V0_V   | 28V0 恒压源电压 | 分压后直读 |
| 7   | 12V0_V   | 12V0 恒压源电压 | |
| 8   | 5V0_V    | 5V0 恒压源电压 | |
| 9   | 3V3_V    | 3V3 恒压源电压 | |
| 10  | DYGY_FLT | DYGY FAULT 模拟量 | 分段译码见 `monitor_fault_decode` |
| 11  | GSDJ_FLT | GSDJ FAULT 模拟量 | 分段译码见 `monitor_fault_decode` |
| 12-15 | ---      | 未使用 | 保留 |

---

## 关键代码片段

### monitor_diag_dump() - 新增校准段
```c
TRACE_OUT_2(DEBUG_OUT, "[DIAG] === 校准后物理量 (g_monitor) ===\r\n");
TRACE_OUT_2(DEBUG_OUT, "[DIAG] 设备实测:\r\n");
for (id = 1u; id < DEV_NUM; id++) {
    TRACE_OUT_2(DEBUG_OUT, "[DIAG]   %02u %-8s V=%u.%03uV I=%u.%03uA T=%.1fC\r\n", ...);
}
TRACE_OUT_2(DEBUG_OUT, "[DIAG]   KF1=%.1fC KF2=%.1fC\r\n", g_monitor.kf1_temp_c, g_monitor.kf2_temp_c);

TRACE_OUT_2(DEBUG_OUT, "[DIAG] 辅助量:\r\n");
TRACE_OUT_2(DEBUG_OUT, "[DIAG]   rails A: 3V3=%.3fA 12V0=%.3fA 5V0=%.3fA 28V0=%.3fA\r\n", ...);
TRACE_OUT_2(DEBUG_OUT, "[DIAG]   rails V: 28V0=%.3fV 12V0=%.3fV 5V0=%.3fV 3V3=%.3fV\r\n", ...);
TRACE_OUT_2(DEBUG_OUT, "[DIAG]   HAL: CH0=%.3fV CH1=%.3fV\r\n", g_monitor.hal_ch0_v, g_monitor.hal_ch1_v);
TRACE_OUT_2(DEBUG_OUT, "[DIAG]   FAULT: DYGY=%.3fV(%s) GSDJ=%.3fV(%s)\r\n", ...);
```

### power_diag_dump() - 每行追加实测值
```c
TRACE_OUT_2(DEBUG_OUT,
    "[DIAG] %02u %-8s EN=%u FLT=%u DAC(U%u.%c)=%u(%d.%03dV,%umA) 实测: I=%u.%03uA V=%u.%03uV T=%.1fC\r\n",
    id, dev->name, en_lvl, fault,
    dev->dac_idx + 14u, 'A' + dev->dac_ch,
    code, mv/1000, mv%1000, s_limit_ma[id],
    g_monitor.cur_ma[id]/1000, g_monitor.cur_ma[id]%1000,
    g_monitor.vol_mv[id]/1000, g_monitor.vol_mv[id]%1000,
    g_monitor.temp_c[id]);
```

### power_diag_test_seq() - 增补电压/温度
```c
TRACE_OUT_2(DEBUG_OUT,
    "[DIAG] %02u %-8s EN=%u FLT=%u I=%u.%03uA V=%u.%03uV T=%.1fC  %s\r\n",
    id, dev->name, en_lvl, fault,
    g_monitor.cur_ma[id]/1000, g_monitor.cur_ma[id]%1000,
    g_monitor.vol_mv[id]/1000, g_monitor.vol_mv[id]%1000,
    g_monitor.temp_c[id],
    (en_lvl && !fault && g_monitor.cur_ma[id]) ? "PASS" : "FAIL");
```

### diag_task() - 模块分隔符
```c
void diag_task(void)
{
    TRACE_OUT_2(DEBUG_OUT, "[DIAG] ===== ADC Raw Sampling =====\r\n");
    monitor_diag_dump();
    TRACE_OUT_2(DEBUG_OUT, "[DIAG] ===== Power DAC & Pin Status =====\r\n");
    power_diag_dump();
    TRACE_OUT_2(DEBUG_OUT, "[DIAG] ===== XCA4001 Alert =====\r\n");
    diag_xca_dump();
}
```

---

## 数据来源对照表

| 需求值 | 来源变量 | 计算方式 |
|--------|----------|----------|
| 设备电压 mV | `g_monitor.vol_mv[id]` | 已含 Flash k/b 校准 |
| 设备电流 mA | `g_monitor.cur_ma[id]` | 已含 Flash k/b 校准 |
| 设备温度 ℃ | `g_monitor.temp_c[id]` | 已含 Flash k/b 校准 |
| KF1/KF2 温度 | `g_monitor.kf1_temp_c` / `kf2_temp_c` | 来自 g_t_map 子表 |
| 轨电流 A | `g_monitor.rail_cur_a[0..3]` | V_adc / (100×R_sense) |
| 恒压源 V | `g_monitor.rail_vol_v[0..3]` | V_adc × 分压系数 |
| HAL 电压 V | `g_monitor.hal_ch0_v` / `hal_ch1_v` | 直读 V_adc |
| FAULT 电压 V | `g_monitor.dygy_fault_v` / `gsdj_fault_v` | 直读 V_adc |
| DAC 码值 | `s_dac_code[id]` | 缓存值 |
| V_CLREF mV | `code/65536*2500` | 现有换算 |
| EN 引脚电平 | `GPIO_ReadInputDataBit(dev->en_port, dev->en_pin)` | 现有 |
| FAULT 引脚 | `power_diag_read_fault(id)` | 现有 |

---

## 使用说明

1. **RTT 查看**：J-Link RTT Viewer 选择 "DIAG" 通道 (up-buffer 2)
2. **输出频率**：`TASK_DIAG_MS = 5000ms` (5 秒)
3. **缓冲区**：RTT buffer 2 分配 2048 字节 (`s_diag_rtt_buf`)，5s 周期下足够
4. **两段式核对**：
   - 原始段：核对 ADC 前端链路 (原始码 → V_adc)、DAC 码值、引脚电平
   - 校准段：核对最终上报值 (mV/mA/℃)、功率路电流电压
5. **KF 双温度**：KF 使用同一片 ADC (U8) 的 AIN6 (KF2_TEMP) 和 AIN15 (KF1_TEMP)

---

## 版本记录

| 版本 | 日期 | 修改人 | 说明 |
|------|------|--------|------|
| 2.0  | 2026-08-20 | - | 两段式输出：原始采样+校准物理量；TASK_DIAG_MS=5s；模块分隔符 |
| 1.1  | 2026-08-20 | - | 增加通道号前缀 `a%02u()`，输出格式为 `a00(DTJ_V)=...` |
| 1.0  | 2026-08-20 | - | 初版：ADC 位号 + 通道名称输出 |