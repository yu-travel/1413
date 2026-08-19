#ifndef __LC1258_H_
#define __LC1258_H_

#include "board_map.h"

/*
    @brief      : LC1258 (SIMCHIP 国产) 16 通道 24 位 Δ-Σ ADC 驱动 (GPIO 位操作模拟 SPI)
    @note       : 1. 引脚结构体 lc1258_handle_t (lc1258_pin_t) 由 board_map.h 定义,
                     bsp_board.c 已完成 GPIO 方向与空闲电平初始化
                     (CS/RST=1, SCLK/DIN/START=0, DRDY 上拉输入, DOUT 输入),
                     本驱动只做协议时序操作
                  2. 厂商时序确认: "上升沿时钟输入数据(DIN), 下降沿时钟输出数据(DOUT)",
                     DOUT 数据流比常规 SPI 提前一拍: bit7 在命令字节最后下降沿输出,
                     bit6..bit0 依次在后续 7 个数据时钟下降沿输出, 仅下沿瞬间有效;
                     发送用 lc1258_spi_byte (DIN 上升沿锁存), 数据读取用
                     lc1258_read_byte_mode1 (bit7 预取 + 下降沿瞬间采样, E6 配方实证)
                  3. 寄存器为 6 位地址 (A5A4+A3A2A1A0), 每次 WREG/RREG 前必须先发
                     "高 2 位地址前缀命令" (官方手册表18, 0xB0~0xB3), 否则读写失效;
                     本工程寄存器全部 00h~09h (A5A4=00) → 前缀固定 0xB0;
                     RDATA 通道数据读不需要前缀
                  4. CS 时序: 拉低后 td(SCCS) 与拉高后 td(CSSC) 均 ≥2tCLK
                     (fCLK=16MHz → tCLK=62.5ns → ≈125ns), 两沿各 delay_us(1)
                  5. Auto-Scan 模式: START 永久拉高, 芯片循环扫描所有已选通道,
                     每通道转换完成 DRDY 拉低, SPI 读出第一个 SCLK 下降沿后
                     DRDY 自动恢复高
                  6. RDATA 读: 发命令 0x30 (MUL 必须=1), 随后 32 个 SCLK 读回
                     1 字节状态 (NEW/OVF/SUPPLY/CHID) + 3 字节 24bit 二进制补码数据;
                     默认输出模式 1LSB=VREF/800000h (P23), 解析不再 <<1,
                     上层按 code/8388608×VREF 换算 (满量程 ±VREF)
                  7. 本板无 PWDN/CLKSEL 引脚 (硬件固定), 复位仅用 RST 硬件引脚
*/

/*============================================================
    寄存器高 2 位地址前缀命令 (官方手册 V1.8 表18, 寄存器读写前必发)
    命令字: 8'b101_100_A5A4
============================================================*/
#define LC1258_CMD_ADDR_MSB_00  0xB0u   /* 1011 0000: 寄存器地址高 2 位 A5A4 = 00 (本工程所有寄存器 00h~09h 用此) */
#define LC1258_CMD_ADDR_MSB_01  0xB1u   /* 1011 0001: A5A4 = 01 */
#define LC1258_CMD_ADDR_MSB_10  0xB2u   /* 1011 0010: A5A4 = 10 (REF 20h / CLK 2Ah 用) */
#define LC1258_CMD_ADDR_MSB_11  0xB3u   /* 1011 0011: A5A4 = 11 */

/*============================================================
    命令字节: C[2:0] | MUL | A[3:0] (MSB 先行)
============================================================*/
#define LC1258_CMD_RDATA   0x30u   /* 001 1 xxxx: 寄存器格式读通道数据 (MUL 必须=1, 无需前缀) */
#define LC1258_CMD_RREG    0x40u   /* 010 0 0000: 寄存器读 (MUL=0 单寄存器), 按位或起始地址 A[3:0] */
#define LC1258_CMD_WREG    0x60u   /* 011 0 0000: 寄存器写 (MUL=0 单寄存器), 按位或地址, 后跟 1 字节数据 */
#define LC1258_CMD_PULSE   0x80u   /* 100 x xxxx: 单次脉冲转换 (START 拉低时用) */
#define LC1258_CMD_RESET   0xC0u   /* 110 x xxxx: 软件复位命令 (寄存器恢复默认) */

/*============================================================
    寄存器地址
============================================================*/
#define LC1258_REG_CONFIG0 0x00u   /* 总控: SPIRST/MUXMOD/BYPAS/CLKENB/CHOP/STAT */
#define LC1258_REG_CONFIG1 0x01u   /* 空闲模式/通道切换延时/偏置电流/采样速率 */
#define LC1258_REG_MUXSCH  0x02u   /* Fixed 模式正/负输入通道选择 */
#define LC1258_REG_MUXDIF  0x03u   /* Auto-Scan 差分通道选择 */
#define LC1258_REG_MUXSG0  0x04u   /* Auto-Scan 单端 AIN0~AIN7 选择 */
#define LC1258_REG_MUXSG1  0x05u   /* Auto-Scan 单端 AIN8~AIN15 选择 */
#define LC1258_REG_SYSRED  0x06u   /* Auto-Scan 内部监测通道 (REF/GAIN/TEMP/VCC/OFFSET) */
#define LC1258_REG_GPIOC   0x07u   /* GPIO 方向配置 */
#define LC1258_REG_GPIOD   0x08u   /* GPIO 电平寄存器 */
#define LC1258_REG_ID      0x09u   /* 只读芯片 ID (固定 0x8B) */

#define LC1258_CHIP_ID     0x8Bu   /* 芯片 ID 固定值 (官方手册 P33 表20, 厂商确认), 用于上电校验 */

/* 状态字节位定义 (32bit 读数据首字节) */
#define LC1258_STAT_NEW    0x80u   /* 1 = 数据未读取 */
#define LC1258_STAT_OVF    0x40u   /* 输入超量程溢出标志 */
#define LC1258_STAT_SUPPLY 0x20u   /* 模拟电源低于 4.3V 告警 */
#define LC1258_STAT_CHID   0x1Fu   /* 当前采样通道编号掩码 (Auto-Scan 有效) */

u8  lc1258_init(lc1258_handle_t *h);
u8  lc1258_write_reg(lc1258_handle_t *h, u8 addr, u8 val);
u8  lc1258_read_reg(lc1258_handle_t *h, u8 addr, u8 *val);
void lc1258_start(lc1258_handle_t *h);
void lc1258_stop(lc1258_handle_t *h);
u8  lc1258_data_ready(lc1258_handle_t *h);
s32 lc1258_read_channel(lc1258_handle_t *h, u8 *chid);

#endif /* __LC1258_H_ */
