#include "lc1258.h"
#include "delay.h"

/*
    @brief      : LC1258 (ADS1258) 24 位 16 通道 ADC 驱动
                  采用 GPIO 位操作模拟 SPI 通信 (SPI Mode0)
    @note       : 1. 引脚方向与空闲电平由 bsp_board.c 完成初始化
                     (CS/RST=1, SCLK/DIN/START=0, DRDY 上拉输入, DOUT 输入),
                     速度 100MHz 推挽
                  2. Mode0 时序: SCLK 空闲低, DIN 在 SCLK 上升沿被锁存,
                     DOUT 在 SCLK 下降沿输出, 故每个 bit 先写 DIN 再拉高 SCLK,
                     拉低 SCLK 后读 DOUT
                  3. CS 拉低后需等待 2.5tCLK 再发命令:
                     内部 fCLK=16MHz → tCLK=62.5ns → 2.5tCLK≈160ns,
                     保守用 delay_us(1) 实现 (依赖 delay_init 已由 main.c 完成)
                  4. 168MHz 系统时钟下单条 GPIO 读写约 20~40ns,
                     已满足芯片 SPI 时序要求, 按位插入 __NOP() 增加时序裕量
*/

/*
    @brief      : 发送一个字节并接收一个字节 (MSB 先行)
    @note       : Mode0: 每 bit 先写 DIN (SCLK=0 期间建立数据) → SCLK=1
                  上升沿锁存 DIN → SCLK=0 下降沿 DOUT 输出 → 读 DOUT;
                   8 bit 循环完成
    @param[in]  : h    LC1258 实例句柄
    @param[in]  : tx   待发送字节
    @param[out] : none
    @retval     : 同时钟下读回的字节
*/
static u8 lc1258_spi_byte(lc1258_handle_t *h, u8 tx)
{
    u8  rx = 0;
    s32 i;

    for (i = 7; i >= 0; i--) {
        GPIO_WriteBit(h->din.port, h->din.pin,
                      (BitAction)((tx >> i) & 0x1u));   /* 写 DIN 位 (SCLK=0 期间建立) */
        __NOP();
        GPIO_SetBits(h->sclk.port, h->sclk.pin);        /* SCLK=1, 上升沿锁存 DIN */
        __NOP();
        GPIO_ResetBits(h->sclk.port, h->sclk.pin);      /* SCLK=0, 下降沿 DOUT 输出 */
        __NOP();
        if (GPIO_ReadInputDataBit(h->out.port, h->out.pin) != Bit_RESET) {
            rx |= (u8)(1u << i);                        /* 读 DOUT 位 (MSB 先行) */
        }
    }

    return rx;
}

/*
    @brief      : CS 拉低后等待 2.5tCLK 再发命令
    @note       : 内部 fCLK=16MHz → tCLK=62.5ns → 2.5tCLK≈160ns,
                  保守用 delay_us(1) 实现
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
static void lc1258_cs_setup_delay(void)
{
    delay_us(1);                                        /* ≥160ns 时序要求, 1us 保守裕量 */
}

/*
    @brief      : 初始化 LC1258
    @note       : 流程: RST 低脉冲复位 (≥2 个系统时钟, 保守 100us) → 释放 →
                  读 ID 寄存器校验 == 0x8B → 写 CONFIG0=0x0A (Auto-Scan + 状态字节),
                   CONFIG1=0x01 (Auto-Scan 每通道 6.168kSPS),
                  MUXSG0/MUXSG1=0xFF (16 路单端通道全部开启);
                  复位后寄存器本为默认值, 重写一次保证确定性
    @param[in]  : h    LC1258 实例句柄
    @param[out] : none
    @retval     : 1 = 成功 (ID 校验通过); 0 = 失败 (句柄无效/ID 不匹配)
*/
u8 lc1258_init(lc1258_handle_t *h)
{
    u8 id = 0;

    if (h == NULL) {
        return 0;
    }

    GPIO_SetBits(h->cs.port, h->cs.pin);                /* CS=1, 总线空闲态 */
    GPIO_ResetBits(h->start.port, h->start.pin);        /* START=0, 复位期间停止转换 */

    GPIO_ResetBits(h->rst.port, h->rst.pin);            /* RST=0, 触发硬件复位 */
    delay_us(100);                                      /* ≥2 个系统时钟 (125ns), 保守 100us */
    GPIO_SetBits(h->rst.port, h->rst.pin);              /* RST=1, 释放复位 */
    delay_us(100);                                      /* 等待芯片加载默认寄存器与滤波缓存复位 */

    if (lc1258_read_reg(h, LC1258_REG_ID, &id) == 0) {
        return 0;
    }
    if (id != LC1258_CHIP_ID) {                         /* 芯片 ID 校验 */
        return 0;
    }

    lc1258_write_reg(h, LC1258_REG_CONFIG0, 0x0A);      /* Auto-Scan + 带状态字节, 内部直连, 不开斩波 */
    lc1258_write_reg(h, LC1258_REG_CONFIG1, 0x01);      /* Standby + 无切换延时 + 无偏置 + DRATE=01 (Auto-Scan 每通道 6.168kSPS, fCLK=16MHz) */
    lc1258_write_reg(h, LC1258_REG_MUXSG0, 0xFF);       /* 单端通道 AIN0~AIN7 全部开启 */
    lc1258_write_reg(h, LC1258_REG_MUXSG1, 0xFF);       /* 单端通道 AIN8~AIN15 全部开启 */

    return 1;
}

/*
    @brief      : 写单个配置寄存器
    @note       : WREG 命令 (011 0 A[3:0], MUL=0), 命令后紧跟 1 字节寄存器数据
    @param[in]  : h     LC1258 实例句柄
    @param[in]  : addr  寄存器地址 0x00~0x09
    @param[in]  : val   待写入寄存器值
    @param[out] : none
    @retval     : 1 = 成功; 0 = 失败 (句柄/地址无效)
*/
u8 lc1258_write_reg(lc1258_handle_t *h, u8 addr, u8 val)
{
    if (h == NULL || addr > LC1258_REG_ID) {
        return 0;
    }

    GPIO_ResetBits(h->cs.port, h->cs.pin);              /* CS=0, 开启 SPI 总线 */
    lc1258_cs_setup_delay();                            /* 等待 2.5tCLK ≈ 160ns */

    lc1258_spi_byte(h, (u8)(LC1258_CMD_WREG | (addr & 0x0F)));  /* 命令 011 0 A[3:0] */
    lc1258_spi_byte(h, val);                            /* 1 字节寄存器数据 */

    GPIO_SetBits(h->cs.port, h->cs.pin);                /* CS=1, 结束通信 */

    return 1;
}

/*
    @brief      : 读单个配置寄存器
    @note       : RREG 命令 (010 0 A[3:0], MUL=0), 第 8 个 SCLK 下降沿开始
                  DOUT 输出寄存器数据
    @param[in]  : h     LC1258 实例句柄
    @param[in]  : addr  寄存器地址 0x00~0x09
    @param[out] : val   读回的寄存器值
    @retval     : 1 = 成功; 0 = 失败 (句柄/地址/出参无效)
*/
u8 lc1258_read_reg(lc1258_handle_t *h, u8 addr, u8 *val)
{
    if (h == NULL || val == NULL || addr > LC1258_REG_ID) {
        return 0;
    }

    GPIO_ResetBits(h->cs.port, h->cs.pin);              /* CS=0, 开启 SPI 总线 */
    lc1258_cs_setup_delay();                            /* 等待 2.5tCLK ≈ 160ns */

    lc1258_spi_byte(h, (u8)(LC1258_CMD_RREG | (addr & 0x0F)));  /* 命令 010 0 A[3:0] */
    *val = lc1258_spi_byte(h, 0x00);                    /* 读回寄存器数据 */

    GPIO_SetBits(h->cs.port, h->cs.pin);                /* CS=1, 结束通信 */

    return 1;
}

/*
    @brief      : 启动连续转换 (Auto-Scan)
    @note       : START 永久拉高, 芯片循环扫描所有已选通道,
                  每通道转换完成拉低 DRDY; START 拉低则停止进入低功耗
    @param[in]  : h    LC1258 实例句柄
    @param[out] : none
    @retval     : none
*/
void lc1258_start(lc1258_handle_t *h)
{
    if (h == NULL) {
        return;
    }

    GPIO_SetBits(h->start.port, h->start.pin);          /* START=1, 进入连续扫描转换 */
}

/*
    @brief      : 停止转换
    @note       : START 拉低后, 当前通道转换完成后停止,
                  进入 Standby/Sleep 低功耗
    @param[in]  : h    LC1258 实例句柄
    @param[out] : none
    @retval     : none
*/
void lc1258_stop(lc1258_handle_t *h)
{
    if (h == NULL) {
        return;
    }

    GPIO_ResetBits(h->start.port, h->start.pin);        /* START=0, 停止扫描转换 */
}

/*
    @brief      : 查询数据就绪状态
    @note       : DRDY=0 表示当前通道 24bit 采样数据已稳定可读;
                  数据被 SPI 读出 (第一个 SCLK 下降沿) 后 DRDY 自动恢复高
    @param[in]  : h    LC1258 实例句柄
    @param[out] : none
    @retval     : 1 = 数据就绪 (DRDY=0); 0 = 未就绪或句柄无效
*/
u8 lc1258_data_ready(lc1258_handle_t *h)
{
    if (h == NULL) {
        return 0;
    }

    return (GPIO_ReadInputDataBit(h->drdy.port, h->drdy.pin) == Bit_RESET) ? 1u : 0u;
}

/*
    @brief      : 读取当前通道转换结果 (RDATA 寄存器格式读)
    @note       : 调用前须由上层轮询 lc1258_data_ready() 确认 DRDY 拉低;
                  CS=0 → 发命令 0x30 (MUL 必须=1) → 随后 32 个 SCLK 读回
                  [1 字节状态][3 字节 24bit ADC 数据]; 数据为二进制补码 MSB 先行;
                  读取操作不影响进行中的转换 (数据手册 RDATA 缓冲语义);
                  状态字节含 NEW/OVF/SUPPLY/CHID, 此处仅取 CHID 返回, 其余位丢弃,
                  NEW/OVF 监测由上层通过轮询 DRDY 保证数据新鲜;
                  RDATA 为缓冲读, 缓冲数据不会被新转换覆盖,
                  但上层超时未读将丢失本轮数据 (随 DRDY 轮询节奏读取即可保证新鲜)
                  24bit → s32 符号扩展: (b1<<24|b2<<16|b3<<8) 算术右移 8 位
    @param[in]  : h     LC1258 实例句柄
    @param[out] : chid  状态字节低 5 位 = 当前采样通道编号 (Auto-Scan 有效), 可传 NULL
    @retval     : 24bit 补码符号扩展后的有符号采样值 (0x7FFFFF=正满量程, 0x800000=负满量程);
                  句柄无效时返回 0
*/
s32 lc1258_read_channel(lc1258_handle_t *h, u8 *chid)
{
    u8  status;
    u8  b1, b2, b3;
    s32 raw;

    if (h == NULL) {
        return 0;
    }

    GPIO_ResetBits(h->cs.port, h->cs.pin);              /* CS=0, 开启 SPI 总线 */
    lc1258_cs_setup_delay();                            /* 等待 2.5tCLK ≈ 160ns */

    lc1258_spi_byte(h, LC1258_CMD_RDATA);               /* 命令 001 1 xxxx, 返回值丢弃 */
    status = lc1258_spi_byte(h, 0x00);                  /* 状态字节: NEW/OVF/SUPPLY/CHID */
    b1 = lc1258_spi_byte(h, 0x00);                      /* 24bit ADC 数据高字节 */
    b2 = lc1258_spi_byte(h, 0x00);                      /* 中字节 */
    b3 = lc1258_spi_byte(h, 0x00);                      /* 低字节 */

    GPIO_SetBits(h->cs.port, h->cs.pin);                /* CS=1, 结束通信 */

    if (chid != NULL) {
        *chid = status & LC1258_STAT_CHID;              /* 当前通道编号 (低 5 位) */
    }

    raw = (s32)(((u32)b1 << 24) | ((u32)b2 << 16) | ((u32)b3 << 8)) >> 8;   /* 24bit 补码符号扩展 */

    return raw;
}
