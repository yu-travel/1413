# DYGLB 电源管理板固件编写计划

> **For agentic workers:** REQUIRED SUB-SKILL: 执行时使用 superpowers:subagent-driven-development 或 superpowers:executing-plans 逐任务实施。步骤使用 `- [ ]` 复选框跟踪。

> **实施状态:** Task 1~12 已完成（2026-08-16，全部推送 main 分支，Keil Rebuild 0 Error 0 Warning）；Task 13 为硬件联调清单，待实板执行。

**Goal:** 按 App 业务层 → Dev 设备抽象层 → Bsp 板级支撑层 → FWLIB 芯片原厂库四层结构，重写 DYGLB 电源管理板固件（15 路电源监控 + 限流控制 + FPGA SPI 协议）。

**Architecture:** 顶层重建 `App/`、`Dev/`、`Bsp/` 三个目录；`FWLIB`（ST 标准库）、`Core`、`Middleware`、`Utilities` 复用不动；旧代码（AD7606、AD5542、control_Dev、transf_jkkzb 及旧协议）全部移除。GDA6641 与 LC1258 均采用 GPIO 位操作模拟时序，SPI1（10MHz 主机）与 FPGA 通信。

**Tech Stack:** STM32F407IGTx(LQFP176) + ST 标准库 1.8.0、Keil MDK (AC5)、SEGGER RTT、elog、MultiTimer 软定时器、内部 Flash 校准。无 EEPROM（上电默认全关，默认开关状态由 FPGA 下发帧提供）。

### 系统时钟配置（doc/clock_config.md，2026-08-17 实施）

| 配置项 | 值 | 说明 |
|--------|-----|------|
| HSE | 25MHz | 新板外部晶振，`User/stm32f4xx.h` HSE_VALUE 已改 25000000（原 8MHz 适配旧板） |
| PLL | M=25 / N=336 / P=2 / Q=4 | `User/system_stm32f4xx.c` PLL_M 8→25、PLL_Q 7→4（本板不用 USB/RNG/SDIO，Q=4 输出 84MHz） |
| SYSCLK / HCLK | 168MHz | AHB /1 |
| APB1 / APB2 | 42MHz / 84MHz | /4 / /2，定时器倍频后 84MHz / 168MHz；TIM/SPI/USART 时序均不变 |
| CSS | Enable | SetSysClock PLL 切换成功后置 CSSON；HSE 失效自动切 HSI 并触发 NMI（`Bsp/bsp_it.c` NMI_Handler 清 CSSF + RTT 告警） |
| RTC 源 | LSI | 本固件未用 RTC，无代码；LSI 自动驱动 IWDG |

---

## 一、系统资源清单（来自 doc 分析，2026-08-15 定稿）

### 1.1 15 路逻辑电源设备（协议 Device-ID）

| ID | 名称 | EN | 故障信号 | 芯片 | CLREF DAC |
|----|------|----|---------|------|-----------|
| 0x01 | 28V_KF | PD0 | PB4(KF1)+PA15(KF2) | MAC5048×2 (U20=KF1, U89=KF2) | U17.VOUTB |
| 0x02 | 28V_DTJ | PD14 | PB15 | MAC5048 | U14.VOUTD |
| 0x03 | 12V_DYGY | PD9 | PH6(GOK)/PH7(GOC), FAULT→CH3.AIN10 | HQEF5016 | U15.VOUTA |
| 0x04 | 28V_QGSJ | PD1 | PB9 | MAC5048 | U17.VOUTA |
| 0x05 | 28V_SFXJ1 | PD4 | PB12 | MAC5048 | U14.VOUTB |
| 0x06 | 28V_SFXJ2 | PD7 | PB10 | MAC5048 | U14.VOUTC |
| 0x07 | 12V_GSDJ | PD12 | PA2(GOK)/PA3(GOC), FAULT→CH3.AIN11 | HQEF5016 | U15.VOUTC |
| 0x08 | 28V_WAOXJ | PD8 | PB13 | MAC5048 | U16.VOUTD |
| 0x09 | 28V_HWXJ1 | PD13 | PB8 | MAC5048 | U15.VOUTB |
| 0x0A | 28V_HWXJ2 | PD2 | PB7 | MAC5048 | U17.VOUTC |
| 0x0B | 28V_HWXJ3 | PD6 | PB0 | MAC5048 | U16.VOUTB |
| 0x0C | 28V_HJJC1 | PD3 | PB5 | MAC5048 | U16.VOUTC |
| 0x0D | 28V_HJJC2 | PD10 | PB6 | MAC5048 | U15.VOUTD |
| 0x0E | 28V_HJJC3 | PD11 | PB14 | MAC5048 | U14.VOUTA |
| 0x0F | 28V_PD | PD5 | PB3 | MAC5048 | U16.VOUTA |

- 4 片 GDA6641 = 16 路 DAC，用 15 路（U17.VOUTD 空闲）
- 4 片 LC1258：CH0=V_Monitor / CH1=I_Monitor / CH2=T_Monitor / CH3=XCA4001+恒压源+GSDJ_HAL+DYGY/GSDJ FAULT
- XCA4001×4：RESET(PC0~PC3)、ALERT(PC4/PC5/PC12/PC13)、Monitor→CH3.AIN0~3

### 1.2 监测通道映射表（最终定稿）

**V_Monitor（ADC_CH0, U2）：**

| AIN | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 |
|-----|---|---|---|---|---|---|---|---|---|---|---|----|----|----|----|----|
| 设备 | DTJ | SFXJ1 | SFXJ2 | HWXJ3 | HJJC3 | DYGY | WAOXJ | GSDJ | KF | PD | HJJC1 | QGSJ | HWXJ2 | HWXJ1 | HJJC2 |

**I_Monitor（ADC_CH1, U5）：**

| AIN | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 |
|-----|---|---|---|---|---|---|---|---|---|---|---|----|----|----|----|----|
| 设备 | HWXJ3 | DTJ | SFXJ2 | HJJC3 | SFXJ1 | WAOXJ | DYGY | GSDJ | HJJC2 | HWXJ1 | HWXJ2 | QGSJ | HJJC1 | PD | KF |

**T_Monitor（ADC_CH2, U8）：**

| AIN | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 |
|-----|---|---|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|
| 设备 | WAOXJ | DYGY | SFXJ1 | HJJC3 | SFXJ2 | DTJ | KF2 | HWXJ3 | QGSJ | HJJC1 | HWXJ1 | HWXJ2 | HJJC2 | PD | GSDJ | KF1 |

**ADC_CH3（U11）：**

| AIN | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 |
|-----|---|---|---|---|---|---|---|---|---|---|----|----|
| 信号 | 3V3(U82) | 12V0(U84) | 5V0(U83) | 28V0(U85) | GSDJ_HAL_CH0 | GSDJ_HAL_CH1 | 28V0恒压源 | 12V0恒压源 | 5V0恒压源 | 3V3恒压源 | DYGY_FAULT | GSDJ_FAULT |

### 1.3 换算公式

```
【电压上报值】
  5048: VOUT = V_adc × 16/2             = V_adc × 8
  5016: VOUT = V_adc × 16×(27.4/37.4)   = V_adc × 11.722
【电流上报值】
  5048: IMON_pin = V_adc × (10/28.2)；I_OUT = IMON_pin / (25µA/A × 3.6k) = IMON_pin / 0.09
  5016: IMON_pin = V_adc / 2；          I_OUT = IMON_pin / (10µA/A × 2k)  = IMON_pin / 0.02
【温度（本地监测，协议无温度字段不上报）】
  5048: VTEMP = V_adc × (10/24.7)；T_J = (VTEMP + 0.2V) / 12.1mV/℃
  5016: VTEMP = V_adc / 2；          T_J = (VTEMP − 152.5mV) / 8.7mV/℃
【限流 DAC 换算】
  5048: V_CLREF = I_LIMIT × 3.6k × 25µA/A = I_LIMIT × 0.09
  5016: V_CLREF = I_LIMIT × 2k × 10µA/A   = I_LIMIT × 0.02
  DAC 码值: D = V_CLREF / 2.5 × 65536
【XCA4001 电流】
  I = V_monitor / (100 × R_sense)；R_sense = 0.04Ω(3V3/5V0)、0.08Ω(12V0)、0.16Ω(28V0)
【恒压源】
  28V0: ×(167.8/17.8)；12V0: ×(195/45)；5V0: ×2；3V3: ×1
【FAULT 模拟量译码（DYGY/GSDJ）】
  0.1V 正常 / 0.3V 比较器故障 / 0.6V MOS 损坏 / 0.9V 过温过压 / 1.2V 过流 / 1.5V 短路
【协议】
  SPI 10MHz，MCU 主机；帧头 0x55AA(上传)/0xAA55(下发)、帧长 2B、校验和 16 位累加（含帧头，低 16 位）、帧尾 0xACBC
  上传帧内容 = 15×(ID+V+I) u16 + 默认状态 + 开关状态 + 告警状态；电压×1000(mV)、电流×1000(mA)，×1000 后截断小数（不四舍五入）
  wire 字节序 = 大端（文档前导帧字节列表 55 AA 00 02 AB DE 01 8A AC BC 为证），实现集中于 put16_be/rd16_be
```

---

## 二、目录结构（四层重建）

```
DYGLB/
├── App/                          # 第一层 APP 业务层（新建）
│   ├── app_main.c/h
│   ├── app_power.c/h             # 15路开关/默认上电/故障恢复
│   ├── app_monitor.c/h           # 采集调度/物理换算/告警
│   ├── app_protocol.c/h          # SPI 组包/解包/校验
│   └── app_config.h              # 设备枚举/周期/参数
├── Dev/                          # 第二层 设备抽象层（新建，与板无关）
│   ├── lc1258.c/h                # ADS1258 驱动（handle+引脚结构体）
│   ├── gda6641.c/h               # GDA6641 DAC 驱动
│   ├── efuse.c/h                 # MAC5048/HQEF5016 EN+故障抽象
│   └── xca4001.c/h               # XCA4001 RESET/ALERT 抽象
├── Bsp/                          # 第三层 BSP 板级支撑层（新建）
│   ├── board_map.h               # 全部引脚宏 + 15路设备表 + DAC/ADC映射表
│   ├── bsp_board.c/h             # 板级 GPIO 初始化 + handle 常量
│   ├── bsp_spi.c/h               # SPI1 FPGA 通信(10MHz主机) + PA4片选
│   ├── bsp_usart.c/h             # USART1 调试(自 System/usart 迁移)
│   ├── bsp_timer.c/h             # TIM2 1kHz + TIM3 1ms(自 Hardware/BSP/timer 迁移)
│   ├── bsp_iwdg.c/h              # 独立看门狗(迁移)
│   ├── bsp_flash.c/h             # 0x080E0000 校准参数读写
│   ├── bsp_it.c/h                # 中断服务(自 stm32f4xx_it 迁移)
│   └── delay.c/h + sys.c/h       # 自 System/ 迁移
├── FWLIB/  Core/  Middleware/  Utilities/  doc/  Project/MDK/   # 不动/复用
└── (删除) User/、Hardware/、System/ 旧代码
```

分层依赖规则：`App → Dev → Bsp → FWLIB`，只允许向下依赖；`board_map.h` 是 Dev 与 Bsp 之间的唯一耦合点（Dev 通过 handle 结构体接收引脚，不直接引用引脚宏）。

---

## 三、任务分解（每任务结束需 Keil 编译 0 Error 0 Warning）

### Task 1: 工程骨架重建 ✅ 已完成

**Files:** 修改 `Project/MDK/DYGLB.uvprojx`；新建 `App/`、`Dev/`、`Bsp/` 目录

- [x] **Step 1:** Keil 工程新建分组 APP/DEV/BSP，移除旧文件（`AD7606_Driver.c`、`ad5542_driver.c`、`control_Dev.c`、`transf_jkkzb.c`、`mac5048.c`、`myiic.c`、`at24cxx.c`、`User/*`、`System/*`、`Hardware/BSP/timer.c`、`iwdg.c` 暂留但移除编译）
- [x] **Step 2:** 将 `System/delay`、`System/sys`、`System/usart`、`Hardware/BSP/timer.c`、`Hardware/BSP/iwdg.c` 迁移到 `Bsp/`，改 include 路径
- [x] **Step 3:** 新建 `App/`、`Dev/`、`Bsp/` 下的空模块骨架（仅头文件 + 空函数）
- [x] **Step 4:** 修改 `main.c` 为最小空壳（仅时钟 + delay_init + LED 闪烁），确保编译通过
- [x] **Step 5:** 编译验证：Keil Rebuild 0 Error；Commit

### Task 2: BSP 层 —— board_map.h（引脚定义总表）✅ 已完成

**Files:** 新建 `Bsp/board_map.h`

- [x] **Step 1:** 按 doc 定义全部引脚宏（21 路故障输入、15 路 EN 输出、4 路 XCA4001 RESET/ALERT、DAC_CH0~3 引脚组、ADC_CH0~3 引脚组、SPI/UART AF、SWD 保留引脚），格式遵循现有 mac5048.h 风格，例如：

```c
#define KF_PWR_EN_PORT      GPIOD
#define KF_PWR_EN_PIN       GPIO_Pin_0
#define DTJ_PWR_EN_PORT     GPIOD
#define DTJ_PWR_EN_PIN      GPIO_Pin_14
/* ...15路 EN、21路故障输入... */
#define DAC_CH0_SCLK_PORT   GPIOI
#define DAC_CH0_SCLK_PIN    GPIO_Pin_5
/* ...4组 DAC 共 28 引脚、4组 ADC 共 28 引脚... */
```

- [x] **Step 2:** 定义设备枚举与映射表：

```c
typedef enum {
    DEV_KF = 1, DEV_DTJ, DEV_DYGY, DEV_QGSJ, DEV_SFXJ1, DEV_SFXJ2,
    DEV_GSDJ, DEV_WAOXJ, DEV_HWXJ1, DEV_HWXJ2, DEV_HWXJ3,
    DEV_HJJC1, DEV_HJJC2, DEV_HJJC3, DEV_PD, DEV_NUM = 16
} dev_id_e;

typedef struct {
    dev_id_e    id;
    u8          name[12];
    GPIO_TypeDef *en_port;   u16 en_pin;
    u8          fault_cnt;                      /* 1或2；HQEF5016 填2: [0]=GOK [1]=GOC */
    GPIO_TypeDef *fault_port[2]; u16 fault_pin[2];
    u8          dac_idx;     u8 dac_ch;         /* GDA6641 实例与通道 */
    u8          adc_ain_v;                      /* ADC_CH0 电压 AIN */
    u8          adc_ain_i;                      /* ADC_CH1 电流 AIN */
    u8          adc_ain_t;                      /* ADC_CH2 温度 AIN（KF 用 t 子表, 填0xFF） */
    u8          adc_ain_fault;                  /* CH3 FAULT 模拟量 AIN（仅HQEF5016: DYGY=10 GSDJ=11）, 0xFF=无 */
    u8          chip_type;                      /* 0=MAC5048 1=HQEF5016 */
} dev_map_t;

/* 温度通道子表：AIN → (设备, 子通道)，KF 拆分 KF1/KF2 */
typedef struct { dev_id_e dev; u8 sub; } t_map_t;  /* sub: 0=主 1=KF1 2=KF2 */
extern const t_map_t g_t_map[16];
/* 索引 = 设备ID 直索引，[0] 为 DEV_INVALID 占位槽 */
extern const dev_map_t g_dev_map[DEV_NUM];

/* Dev 层 handle 类型 = board_map.h 引脚表结构体 typedef（Dev 与 Bsp 唯一耦合点，Task 6/7 复用不重复定义） */
typedef gda6641_pin_t gda6641_handle_t;
typedef lc1258_pin_t  lc1258_handle_t;
```

- [x] **Step 3:** 定义 4 片 DAC、4 片 ADC 的 handle 引脚常量表（`DAC_CH0_SCLK_PORT` 等宏组成的结构体常量）
- [x] **Step 4:** 编译验证；Commit

### Task 3: BSP 层 —— bsp_board.c/h（板级初始化）✅ 已完成

**Files:** 新建 `Bsp/bsp_board.c/h`

- [x] **Step 1:** `bsp_board_init()` 实现：GPIOA~PI 时钟、JTAG 禁用保留 SWD（`SWJ_CFG_JTAGDISABLE`）、15 路 EN 推挽输出（初始输出低，全部关断）、21 路故障输入（上拉）、4 路 XCA4001 RESET 输出高（锁存模式）、DAC/ADC 引脚初始化（SCLK/SYNC/DIN/LDAC/POR/CLR 输出、SDO/DOUT 输入）、SPI1/USART1 AF 引脚
- [x] **Step 2:** 声明 4 个 `lc1258_handle_t`、4 个 `gda6641_handle_t` 常量（`const`，用 board_map.h 引脚宏填充）
- [x] **Step 3:** 编译验证；Commit

### Task 4: BSP 层 —— bsp_spi.c（FPGA SPI 主机）✅ 已完成

**Files:** 新建 `Bsp/bsp_spi.c/h`（复用 FWLIB spi）

- [x] **Step 1:** SPI1 初始化：主机、CPOL=0/CPHA=0、预分频 /8 → 10.5MHz（10MHz 配置宏，待确认后调）、8bit、MSB；PA4 作 GPIO 片选
- [x] **Step 2:** 接口：

```c
void bsp_spi_init(void);
void bsp_spi_cs(u8 level);
u8   bsp_spi_write_byte(u8 data);   /* 全双工收发1字节 */
void bsp_spi_transfer(u8 *tx, u8 *rx, u16 len);
```

- [x] **Step 3:** 编译验证；Commit

### Task 5: BSP 层 —— bsp_timer/iwdg/eeprom/flash/it ✅ 已完成

**Files:** 迁移+改造 `Bsp/bsp_timer.c`、`bsp_iwdg.c`、`bsp_flash.c`、`bsp_it.c`（EEPROM 已取消，见待确认#5）

- [x] **Step 1:** TIM2 1kHz 软定时 tick（供 softtimer），TIM3 1ms；timer.c/h 改名 bsp_timer.c/h，接口收敛为 `bsp_timer_init()`
- [x] **Step 2:** IWDG_Init(4,800) ~1.6s，主循环喂狗；iwdg.c/h 改名 bsp_iwdg.c/h
- [x] **Step 3:** Flash 0x080E0000 校准区：`flash_cal_read(k,b)` / `flash_cal_write(k,b)`（45 组 k/b float，带魔数校验）
- [x] **Step 4:** 中断服务迁移到 `bsp_it.c`（SysTick/TIM2/TIM3 IRQ），stm32f4xx_it.c 移出编译
- [x] **Step 5:** Keil Device 由 STM32F407VGTx 改为 STM32F407IGTx（实际封装 LQFP176，已确认）
- [x] **Step 6:** 编译验证；Commit

### Task 6: Dev 层 —— gda6641.c/h（DAC 驱动）✅ 已完成

**Files:** 新建 `Dev/gda6641.c/h`

- [x] **Step 1:** 接口（handle 类型直接复用 `board_map.h` 的 `gda6641_handle_t` typedef，不再重复定义）：

```c
/* handle = board_map.h 的 gda6641_handle_t (gda6641_pin_t 的 typedef)，不重复定义 */

void gda6641_init(gda6641_handle_t *h);                 /* POR拉高、CLR/LDAC拉高 */
void gda6641_write(gda6641_handle_t *h, u8 ch, u16 d);  /* 命令0010 写+同步刷新 */
void gda6641_write_input(gda6641_handle_t *h, u8 ch, u16 d); /* 命令0000 仅缓存 */
void gda6641_update_all(gda6641_handle_t *h);           /* LDAC≥20ns低脉冲 同步刷新 */
void gda6641_clear(gda6641_handle_t *h);                /* CLR 低脉冲 硬件清零 */
void gda6641_reset(gda6641_handle_t *h);                /* POR 低脉冲 软复位 */
```

- [x] **Step 2:** 实现 32bit 帧移位（MSB first）：`u32 frame = ((u32)cmd<<24)|((u32)ch<<20)|((u32)d<<4)`；SYNC 拉低 → 32 次 SCLK 位操作（SCLK 拉高写 DIN → 拉低，空闲低电平即 Mode1/CPHA=1 边沿在下降沿发送）→ SYNC 拉高。通道地址编码：A/B/C/D = 0/1/2/3
- [x] **Step 3:** 编译验证；Commit

### Task 7: Dev 层 —— lc1258.c/h（ADC 驱动）✅ 已完成

**Files:** 新建 `Dev/lc1258.c/h`

- [x] **Step 1:** 接口：

```c
/* handle = board_map.h 的 lc1258_handle_t (lc1258_pin_t 的 typedef)，不重复定义 */

u8  lc1258_init(lc1258_handle_t *h);              /* RST复位+读ID==0x8B校验 */
u8  lc1258_write_reg(lc1258_handle_t *h, u8 addr, u8 val);
u8  lc1258_read_reg(lc1258_handle_t *h, u8 addr, u8 *val);
void lc1258_start(lc1258_handle_t *h);            /* START拉高 连续转换 */
void lc1258_stop(lc1258_handle_t *h);
u8  lc1258_data_ready(lc1258_handle_t *h);        /* DRDY==0 */
s32 lc1258_read_channel(lc1258_handle_t *h, u8 *chid); /* RDATA(0x30) 读32bit, 返回24bit有符号 */
```

- [x] **Step 2:** 位操作实现：SCLK 空闲低、上升沿锁存 DIN、下降沿读 DOUT（Mode0）；CS 拉低后延时 2.5 tCLK；WREG=0x60|addr、RREG=0x40|addr（MUL=0 单寄存器读，评审修正）、RDATA=0x30
- [x] **Step 3:** `lc1258_init` 流程：RST 低 2+ 周期 → 高 → 读 ID 验证 0x8B → 写 CONFIG0=0x0A（Auto-Scan+STAT）、CONFIG1=0x01（7.8kSPS）、MUXSG0=0xFF、MUXSG1=0xFF
- [x] **Step 4:** 编译验证；Commit
- [x] **补丁（2026-08-17，按厂商模板程序 `文档/芯片手册/ADS1258/103_1258` 对齐）:**
  1. 数据解析符号扩展后 <<1（×2，模板 readData 实证修正"需要左移一位"），App 换算改 `code/16777215×VREF`（去掉 1.06 系数，同模板 volt_b 公式）
  2. CS 两沿各 delay_us(1)（td(SCCS)/td(CSSC) ≥2tCLK，模板 setCS 实证）
  3. 复位释放后 tWAKE delay_ms(5)（模板 adcStartupRoutine）
  4. 全量写 9 寄存器（CONFIG1=0x41 DLY64µs+DRATE01，模板值）+ 回读 CONFIG0/1/MUXSG0/1 校验
  5. 保留差异（架构设计，非芯片逻辑）：DRDY 轮询替代 EXTI 中断、无 registerMap 缓存、无 DIRECT 读模式、无 PWDN/CLKSEL 引脚（本板硬件固定）、不复制模板 SCBCS 宏错误值
- [x] **补丁（2026-08-18，DOUT 采样时序修正，联调实测）:**
  - 现象：4 片 LC1258 ID 均读回 0x16（=0x8B 左移一位）
  - 根因：`lc1258_spi_byte` 在 SCLK 下降沿后仅 __NOP 裕量即采样 DOUT，芯片 DOUT 含传播延时 t_PD，采样过早读到上一位 → 每字节整体左移 1 bit
  - 修复：SCLK 下降沿后 `delay_us(1)` 再读 DOUT（低位中段采样，每字节 8µs；1ms 采集任务最坏 4 片同时就绪 160µs 占 16%，可接受）
  - 佐证：0x8B<<1=0x116→低8位0x16；同时确认 DOUT 引脚接线通畅（此前"模拟，跳过 GPIO"疑点解除）

### Task 8: Dev 层 —— efuse.c/h + xca4001.c/h ✅ 已完成

**Files:** 新建 `Dev/efuse.c/h`、`Dev/xca4001.c/h`

- [x] **Step 1:** efuse 抽象（handle 传端口引脚）：

```c
typedef struct {
    GPIO_TypeDef *en_port;   u16 en_pin;
    GPIO_TypeDef *gok_port;  u16 gok_pin;   /* 仅HQEF5016用, MAC5048填NULL */
    GPIO_TypeDef *goc_port;  u16 goc_pin;
} efuse_handle_t;

void efuse_on(efuse_handle_t *h);
void efuse_off(efuse_handle_t *h);
void efuse_clear_latch(efuse_handle_t *h);   /* EN低100us→高 软启动并清锁存 */
u8   efuse_is_gok_goc(efuse_handle_t *h);    /* 仅5016 GOK/GOC检测, MAC5048恒返回0勿用于故障判定 */
```

- [x] **Step 2:** xca4001：`xca4001_set_latch_mode(h)`/`xca4001_set_auto_mode(h)`、`xca4001_clear_latch(h)`（≥100ns 低脉冲, 结束后回锁存模式）、`xca4001_alert_active(h)`
- [x] **Step 3:** 编译验证；Commit

### Task 9: App 层 —— app_protocol.c/h（协议）✅ 已完成

**Files:** 新建 `App/app_protocol.c/h`

- [x] **Step 1:** 帧结构定义：

```c
#define FRAME_HEAD_UP    0x55AAu
#define FRAME_HEAD_DOWN  0xAA55u
#define FRAME_TAIL       0xACBCu
#define FRAME_PREAMBLE   0xABDEu
#define FRAME_LEN_UP     0x0060u   /* 实现值; 文档写0x5F, 待确认#1 */
#define FRAME_LEN_DOWN   0x0060u
```

- [x] **Step 2:** 上传帧组包 `protocol_build_upload(u8 *buf, monitor_data_t *m, power_state_t *p)`：0x55AA + 长度 + 15×(ID+V_mV+I_mA) + 默认状态 + 开关状态 + 告警状态 + 校验和（帧头至内容累加低16位）+ 0xACBC
- [x] **Step 3:** 下发帧解包 `protocol_parse_down(u8 *buf, u16 len, threshold_t *t, power_state_t *p)`：校验帧头 0xAA55、帧长、校验和、帧尾；提取 15 路基准电压/电流与开关指令
- [x] **Step 4:** 读流程状态机 `protocol_read_task()`：周期发前导帧（0x55AA,0x0002,0xABDE,校验0x018A,0xACBC）→ 连续时钟接收下发帧 → 解包（握手时序待确认，按"前导帧+继续时钟读"标准模式实现，预留状态机扩展）
- [x] **Step 5:** 编译验证；Commit

### Task 10: App 层 —— app_monitor.c/h（采集与告警）✅ 已完成

**Files:** 新建 `App/app_monitor.c/h`

- [x] **Step 1:** 周期任务（MultiTimer 注册）：`monitor_task()` 轮询 4 片 LC1258 DRDY → `lc1258_read_channel` 取 (chid, 24bit) → 按三张 AIN 映射表（g_dev_map[].adc_ain_v/i/t + g_t_map[]）写入 `monitor_data_t`（15 路 V/I/T + 4 路 XCA4001 + 4 恒压源）
- [x] **Step 2:** 物理量换算：按芯片类型分派两套换算函数（系数集中在 `app_config.h`，默认值取 §1.3 doc 值，Flash k/b 校准覆盖分压离散误差）；KF 设备双温度点 `kf1_temp`/`kf2_temp`
- [x] **Step 3:** FAULT 模拟量译码：DYGY/GSDJ FAULT（CH3.AIN10/11）按电压分段译码（0.1/0.3/0.6/0.9/1.2/1.5V），并入设备告警位
- [x] **Step 4:** 告警判断：V/I > FPGA 下发基准阈值 → 告警位；FAULT/GOK/GOC/ALERT 硬件信号并入告警位
- [x] **Step 5:** 编译验证；Commit

### Task 11: App 层 —— app_power.c/h（电源控制业务）✅ 已完成

**Files:** 新建 `App/app_power.c/h`

- [x] **Step 1:** `power_init()`：上电默认全关（无 EEPROM）+ 4 片 DAC 初始化 + 15 路默认限流(DEFAULT_I_LIMIT_MA=0 关断式)写入并 LDAC 刷新
- [x] **Step 2:** `power_apply_state(switch_state)`：FPGA 下发开关指令处理（开→先清锁存再 EN 高；关→EN 低；边沿检测，无变化不动作）
- [x] **Step 3:** 限流配置 `power_set_limit(id, i_limit_mA)`：换算 `V_CLREF = I × (0.09 或 0.02)` → `D = V_CLREF/2.5×65536` → `gda6641_write_input` 缓存 → `power_flush_limits()` 4 片 `gda6641_update_all` 同步刷新
- [x] **Step 4:** 故障恢复策略：默认"MCU 只上报告警，开关/清锁存由 FPGA 指令驱动"
- [x] **Step 5:** 编译验证；Commit
- [x] **遗留（Task 12 接线）:** `default_state` 字段消费（FPGA 下发默认状态按帧配置）；先 flush 限流再 apply 开关状态的顺序约定

### Task 12: App 层 —— app_main.c（主流程集成）✅ 已完成

**Files:** 新建 `App/app_main.c/h`；删除 `User/main.c`

- [x] **Step 1:** 主流程：NVIC 分组 → delay_init → bsp_board_init → usart/RTT → bsp_spi_init → bsp_timer_init → softtimer_init → iwdg_init → power_init（DAC 初始化+默认限流）→ monitor_init（4 ADC ID 校验+start+校准读取）→ 注册 MultiTimer 任务（1ms monitor_task / 100ms monitor_convert_all / 周期协议收发 + FPGA 下发指令消费：先 power_set_limit+flush 再 power_apply_state、default_state 按帧配置）
- [x] **Step 2:** 主循环：`softtimer_loop(); bsp_iwdg_feed();`
- [x] **Step 3:** Keil 全量 Rebuild 0 Error 0 Warning；Commit

### Task 13: 集成联调清单（硬件，按顺序执行）

**驱动级验证：**

- [ ] 上电验证 SYSCLK=168MHz（MCO 引脚输出或 1ms 定时/波特率精度；25MHz 晶振 + PLL M25/N336/P2/Q4，2026-08-17 已按 clock_config.md 配置）
- [ ] 4 片 LC1258 上电读 ID = 0x8B（2026-08-18 实测曾读 0x16=0x8B<<1，DOUT 采样过早已修：下降沿后 delay_us(1) 采样；失败时对应设备告警位应置位，RTT 有日志）
- [ ] 4 片 GDA6641 输出 0~2.5V 任意通道验证（示波器确认 SCLK 位时序与 LDAC ≥20ns 脉冲宽度，Task 6 评审跟进项）
- [ ] 15 路 EN 通断 + 21 路故障输入读取
- [ ] DAC 限流 → 实际电流钳位值校准（5048: I=V_CLREF/0.09; 5016: I=V_CLREF/0.02）

**协议联调（与 FPGA）：**

- [ ] 帧长 0x0060(96B) vs 文档 0x005F —— 待确认#1
- [ ] 握手时序：前导帧后 FPGA 回复时机与 CS 保持要求 —— 待确认#2（调整点: protocol_read_task）
- [ ] wire 大端字节序确认 —— 待确认#3（切换点: put16_be/rd16_be）
- [ ] SPI 时钟 10.5MHz 容差确认 —— 待确认#4
- [ ] default_state 语义（首帧无开关指令时生效）与 threshold=0 语义确认
- [ ] 故障恢复策略确认（MCU 只上报，FPGA 指令驱动）—— 待确认#6

**校准与参数：**

- [ ] LC1258 外部基准电压 ADC_VREF 确认（默认 2.5V）与 DOUT 引脚接线确认（DOUT 数据流已实证通畅，2026-08-18）
- [ ] LC1258 读数与实测电压比对（验证厂商模板 <<1 移位与 VREF 换算；若偏差 2 倍，回退 lc1258_read_channel 移位与 ADC_FS_CODE 各一行，2026-08-17 按模板对齐）
- [ ] FAULT 模拟量分段阈值容差校准（±150mV 带宽）—— 待确认#9
- [ ] 45 组 k/b 校准系数烧写与验证（0x080E0000）—— 待确认#7
- [ ] PE13/PF13 空置确认 —— 待确认#8
- [ ] 上电时序：原 1500ms 上电延时已移除，确认 FPGA/ADC 上电稳定时间是否够
- [ ] 采集停转监测（采样新鲜度看门狗）—— Task 10 评审跟进项，若联调发现 ADC 卡死需补实现

---

## 四、待确认问题清单

**结构性（建议执行期间并行确认）：**

1. 协议帧长 0x005F(95B) vs 实际内容 96B(0x60) —— 配置宏，改一个数字
2. 读帧握手时序（前导帧后 FPGA 回复时机）—— 按标准 SPI 主从模式实现，预留状态机扩展

**参数性（不阻塞编码，联调时确定）：**

3. LC1258 DOUT（PH12/PE14/PF7/PF8）文档标注"模拟，跳过 GPIO"与 SPI 数据输出功能矛盾 —— 代码按 DOUT=输入 处理
4. SPI 时钟 10MHz 无法整除（84/8=10.5MHz）—— 配置宏
5. ~~默认开关状态存储位置~~ —— 已确认：无 EEPROM，上电默认全关，FPGA 帧下发
6. 故障恢复策略 —— 默认"MCU 只上报，FPGA 指令驱动"
7. 校准方案规模 —— 45 组 k/b（15 路×V/I/T）存 Flash 0x080E0000
8. PE13/PF13 空置确认
9. FAULT 模拟量电压分段阈值容差（±多少 mV）
10. ~~MCU 封装~~ —— 已确认：STM32F407IGTx (LQFP176)，Keil Device 已改
