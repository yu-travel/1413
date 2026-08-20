# ADC 诊断输出格式更新记录

## 修改时间
2026-08-20

## 修改目的
将 `monitor_diag_dump()` 中 4 片 ADC 的诊断输出从通道号 (`a00`, `a01`...) 改为 **ADC 位号 (U2/U5/U8/U11) + 信号名称**，便于硬件联调核对原理图与实测值。

## 修改文件
- `App/app_monitor.c` — `monitor_diag_dump()` 函数
- `doc/ADC_Diag_Output.md` — 本文档

---

## 修改前后输出对比

### 修改前
```
[DIAG] ADC CH0(V): a00=12345(1.234V) a01=23456(2.345V) a02=34567(3.456V) ...
[DIAG] ADC CH1(I): a00=... a01=... a02=... ...
[DIAG] ADC CH2(T): a00=... a01=... a02=... ...
[DIAG] ADC CH3(AUX): a00=... a01=... a02=... ...
```

### 修改后
```
[DIAG] U2(ADC_CH0(V)): DTJ_V=12345(1.234V) SFXJ1_V=23456(2.345V) SFXJ2_V=34567(3.456V) ...
[DIAG] U5(ADC_CH1(I)): HWXJ3_I=... DTJ_I=... SFXJ2_I=... ...
[DIAG] U8(ADC_CH2(T)): WAOXJ_TEMP=... DYGY_TEMP=... KF2_TEMP=... KF1_TEMP=... ...
[DIAG] U11(ADC_CH3(AUX)): 3V3_CUR=... 12V0_CUR=... 5V0_CUR=... 28V0_CUR=... HAL0_V=... ...
```

---

## 4 片 ADC 完整通道映射表

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
| 12  | ---      | 未使用 | 保留 |
| 13  | ---      | 未使用 | 保留 |
| 14  | ---      | 未使用 | 保留 |
| 15  | ---      | 未使用 | 保留 |

---

## 关键代码片段

```c
static const char *const s_adc_refdes[4] = { "U2", "U5", "U8", "U11" };
static const char *const s_adc_func[4]   = { "电压", "电流", "温度", "辅助" };

static const char *const *name_tbl[4] = { s_ch0_name, s_ch1_name, s_ch2_name, s_ch3_name };

for (i = 0u; i < 4u; i++) {
    TRACE_OUT_2(DEBUG_OUT, "[DIAG] %s(ADC_%s): ", s_adc_refdes[i], s_ain_tag[i]);
    for (a = 0u; a < MON_AIN_NUM; a++) {
        if ((*valid[i] & MON_ALARM_BIT(a)) == 0u) continue;
        s32 mv = (s32)(monitor_raw_to_vadc(raw[i][a]) * 1000.0f);
        TRACE_OUT_2(DEBUG_OUT, "%s=%ld(%d.%03dV) ", name_tbl[i][a], (long)raw[i][a], ...);
    }
    TRACE_OUT_2(DEBUG_OUT, "\r\n");
}
```

---

## 使用说明

1. **RTT 查看**：J-Link RTT Viewer 选择 "DIAG" 通道 (up-buffer 2)
2. **输出频率**：由 `app_diag.c` 的 `TASK_DIAG_MS` 控制 (默认 1000ms)
3. **换行处理**：每片 ADC 单独一行，单行可能较长，RTT Viewer 自动换行
4. **核对方法**：对照原理图管脚连接关系表 (`doc/原理图管脚连接关系.txt`) 与 `board_map.h` 中的 AIN 映射
5. **KF 双温度**：KF 使用同一片 ADC (U8) 的 AIN6 (KF2_TEMP) 和 AIN15 (KF1_TEMP)，输出时已区分

---

## 版本记录

| 版本 | 日期 | 修改人 | 说明 |
|------|------|--------|------|
| 1.0  | 2026-08-20 | - | 初版：ADC 位号 + 通道名称输出 |