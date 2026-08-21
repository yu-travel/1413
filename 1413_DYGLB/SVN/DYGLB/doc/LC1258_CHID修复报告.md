# LC1258 AIN0~15 采样错位问题修复报告

- **日期**: 2026-08-21
- **作者**: 联调现场 (szy + opencode 协助)
- **状态**: 已上板验证通过
- **涉及芯片**: LC1258 ×4 (U2/U5/U8/U11, ADS1258 兼容型号, STM32F407 GPIO 模拟 SPI 驱动)

---

## 1. 现象

芯片联调诊断输出 (`monitor_diag_dump`) 中:

- U2/U5/U8/U11 四片 ADC 均为 **a00~a07 槽位恒为 0、a08~a15 有正常数据**
- 万用表实测 U2 的 AIN0 与 AIN8 **芯片引脚电压均 3.5V** (28V 母线已上电, 12V 未上电)
- 反向配置实验:
  - `MUXSG0=0x00, MUXSG1=0xFF` → 全部通道无数据
  - `MUXSG0=0xFF, MUXSG1=0x00` → 收不到任何 CHID 0~7
  - `MUXSG0=0xFF, MUXSG1=0xFF` → 仅槽位 8~15 有数据
- MUXSG0/MUXSG1 非默认值 (0x55/0xAA) 写入回读校验通过

## 2. 排查时间线

| 阶段 | 内容 | 结论 |
|---|---|---|
| 1 | RTT 输出割裂/乱码 | `BUFFER_SIZE_UP` 1024→12288 解决; `TRACE_OUT_2` 由 buffer2 改走 buffer0 (buffer2 弃用) |
| 2 | a00~a07 "显示 0" | 现场曾注释 valid 位过滤, 未采样槽位打印了静态零值, 造成假象; 后恢复过滤 |
| 3 | 硬件/寄存器排查 | 万用表排除输入网络; 非默认值回读排除写入失败 |
| 4 | 反向配置实验 | 四组结果指向 "低 8 路收不到 CHID 0~7、高 8 路数据错位" |
| 5 | **厂商参考工程定案** | `文档/芯片手册/ADS1258/103_1258/Middleware/ads1258.h` L107~132 的 CHID 官方编码表 |

## 3. 根因

**状态字节 CHID 编码 ≠ AIN 引脚号**。官方编码 (单端通道 **CHID = AIN + 8**):

| CHID | 通道 | 控制寄存器 |
|---|---|---|
| 0x00~0x07 | DIFF0~DIFF7 差分 | MUXDIF (本板 0x00, 永不出现) |
| **0x08~0x0F** | **AIN0~AIN7** | MUXSG0 |
| **0x10~0x17** | **AIN8~AIN15** | MUXSG1 |
| 0x18~0x1D | SYSRED 内部监测 (OFFSET/VCC/TEMP/GAIN/REF) | SYSRED |
| 0x1F | Fixed 固定通道模式 | - |

`monitor_task()` 原以 CHID 直接作 `s_raw_*[]` 数组下标, 后果:

1. **AIN0~AIN7 的数据 (CHID 8~15) 错存进槽位 8~15**, 顶着错误通道名显示
2. **AIN8~AIN15 (CHID 0x10~0x17) 被 `chid >= 16` 判据静默丢弃**, 高 8 路从未被采到
3. CHID 0~7 属差分通道, 本板 MUXDIF=0 时永不出现 → "低八路收不到 CHID 0~7" 实为正常

## 4. 证据链

1. `FF/FF` 时槽 8~15 "有数据" 实为 AIN0~7 的真实信号
2. `SG0=FF, SG1=00` → 芯片只扫 AIN0~7 → CHID 8~15 → 落槽 8~15
3. `SG0=00, SG1=FF` → 芯片只扫 AIN8~15 → CHID 0x10~0x17 → 全部被丢 → 全零
4. 仅有的两路 12V 设备 **DYGY_V/GSDJ_V** (原理图接 AIN5/AIN7) 数据出现在 a13/a15 且 ≈0.054V (12V 母线未上电), 其余 28V 路全 ≈3.5V —— 错位 +8 完全吻合
5. EFUSE 主动测试中仅 12V_DYGY 报 FLT=1, 与 12V 母线状态吻合
6. 修复后上板验证: DTJ_V 回到 a00≈3.54V, DYGY_V/GSDJ_V 回到 a05/a07, U8 温度整体平移 8 格数值不变, 16 槽位全 valid

## 5. 修复内容

| # | 文件 | 改动 |
|---|---|---|
| 1 | `Dev/lc1258.h` | 新增 `LC1258_CHID_AIN_BASE/LAST` 宏 + CHID 编码表注释; `lc1258_chid_to_ain()` 声明 |
| 2 | `Dev/lc1258.c` | 新增 `lc1258_chid_to_ain()`: CHID 0x08~0x17 → AIN 0~15, 其余返回 0xFF |
| 3 | `App/app_monitor.c` `monitor_task()` | 读样本后 `ain = lc1258_chid_to_ain(chid)`, 越界(0xFF)丢弃, switch 内下标/valid 位图改用 `ain` |
| 4 | `App/app_monitor.c` `monitor_diag_dump()` | 恢复被注释的 valid 位过滤 (映射修复后 valid 重新可信) |

## 6. 扩展寄存器访问修复 (同批)

现场调试需要 dump 高阶寄存器 (0x14/0x1B/0x1E/0x20 REF/0x2A CLK/0x0B 等):

- 原 `lc1258_read_reg/write_reg` 限制 `addr > 0x09` 即拒绝, 且前缀命令**写死 0xB0** (仅覆盖 00h~0Fh)
- 修复: 地址上限放开至 **0x3F** (6 位地址空间); 前缀按 `addr[5:4]` 动态计算 `LC1258_CMD_ADDR_MSB(addr)` = `0xB0 | (addr>>4)`, 手册表18 的 0xB1/0xB2 分段自动匹配
- 上电 rst/set 两段全量寄存器 dump 收进 `#if ADC_REG_DUMP_TEST` 开关 (app_config.h, 当前保持 1 供调试, 联调收尾建议改 0)

## 7. 同批联调现场调整 (一并入档)

| 项 | 原值 | 现值 | 理由 |
|---|---|---|---|
| `ADC_VREF` | 2.5f | 4.096f | 与实测显示电压自洽 (待最终校准确认) |
| RST 复位脉宽 | delay_us(100) | delay_ms(5) | 上电时序更稳 (ADC0/1 曾出现寄存器读全零) |
| `BUFFER_SIZE_UP` | 1024 | 12288 | 修 RTT 输出割裂 (NO_BLOCK_SKIP 整条丢弃) |
| `TRACE_OUT_2` | myprintf_2 (buffer2) | myprintf (buffer0) | buffer2 实测不稳, 弃用 |
| `diag_init()` buffer2 配置 | 启用 | 注释 | 同上, 保留待再调试 |
| `GPIOD=0x00` 写入 | 启用 | 注释 | GPIO 引脚外接高电平, 输入模式下写电平寄存器无意义 |
| `s_adc_func` 中文名数组 | 存在 | 保留(未引用警告) | 后续调试用 |

## 8. 遗留待办 (联调收尾清单)

- [ ] TASK 周期恢复设计值: `TASK_MONITOR_MS` 100→1, `TASK_CONVERT/UPLOAD/PROTO` 1000→100/100/200, `TASK_DIAG_MS` 10000→5000
- [ ] 恢复 `bsp_iwdg_init()` 看门狗
- [ ] 恢复 upload/proto/heartbeat 任务注册 (app_main.c 现注释)
- [ ] 移除 `power_apply_state(0xFFFF)` 上电全开调试语句
- [ ] `ADC_REG_DUMP_TEST` 验证完改 0
- [ ] `ADC_VREF` 4.096 与分压系数联调校准确认
- [ ] U11 a14/a15 (未接通道) 曾出现的 `ÀG/ÀK` 显示乱码: buffer 扩容后待复查, 若仍在单独排查
- [ ] `lc1258_dump_all_reg()` 的 `reg_addr_list` 调试完毕后可裁剪

## 9. 经验总结 (驱动级硬知识, 继 E6 配方之后)

1. **CHID 是扫描顺序编号不是引脚号**: ADS1258 系把 DIFF0~7 排在 0~7, 单端 AIN0 从 8 开始。任何以 CHID 作数组下标的代码都必须先减 8
2. **寄存器前缀命令与地址段绑定**: 0xB0/0xB1/0xB2/0xB3 分别覆盖 16 字节窗口, 访问高阶寄存器必须换前缀
3. **诊断输出必须区分 "未采样" 与 "采样为 0"**: valid 位过滤被注释时, 静态零值造成了最大误导
4. **验证通道映射用 "母线差异法" 最快**: 让不同母线的设备电压不同, 一眼看出数据错位
