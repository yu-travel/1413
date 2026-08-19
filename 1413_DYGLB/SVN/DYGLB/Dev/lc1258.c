#include "lc1258.h"
#include "delay.h"

/*
    @brief      : LC1258 (SIMCHIP 国产) 16 通道 24 位 Δ-Σ ADC 驱动
                  采用 GPIO 位操作模拟 SPI 通信 (厂商确认: DIN 上升沿输入, DOUT 下降沿输出)
    @note       : 1. 引脚方向与空闲电平由 bsp_board.c 完成初始化
                     (CS/RST=1, SCLK/DIN/START=0, DRDY 上拉输入, DOUT 输入),
                     速度 100MHz 推挽
                  2. 厂商时序确认: "上升沿时钟输入数据(DIN), 下降沿时钟输出数据(DOUT)",
                     DOUT 数据流比常规 SPI 提前一拍: bit7 在命令字节最后下降沿输出,
                     bit6..bit0 依次在后续 7 个数据时钟下降沿输出, 且仅下沿瞬间有效;
                     发送用 lc1258_spi_byte (DIN 上升沿锁存), 数据读取用
                     lc1258_read_byte_mode1 (bit7 预取 + 下降沿瞬间采样, E6 配方实证)
                  3. 寄存器 6 位地址前缀: 每次 WREG/RREG 前必须先发 0xB0 前缀
                     (官方手册 V1.8 表18, 本工程寄存器全部 A5A4=00);
                     RDATA 通道数据读无需前缀
                  4. CS 时序: 拉低后 td(SCCS) 与拉高后 td(CSSC) 均 ≥2tCLK
                     (fCLK=16MHz → tCLK=62.5ns → ≈125ns), 两沿各 delay_us(1)
                  5. 数据解析: 24bit 补码符号扩展 (不再 <<1, 厂商模板 <<1 是其
                     CPHA_1Edge 上升沿采样早一拍的补偿, E6 下降沿采样读回完整位流);
                     上层按 code/8388608×VREF 换算 (手册默认 1LSB=VREF/800000h,
                     满量程 ±VREF); 2026-08-18 E6 配方上板验证 ID=8B 全寄存器完整
*/

/*
    @brief      : 发送一个字节并接收一个字节 (MSB 先行)
    @note       : Mode0: 每 bit 先写 DIN (SCLK=0 期间建立) → SCLK=1 (上升沿锁存
                  DIN, 此时 DOUT 自上一下降沿起稳定为当前位) → delay_us(1) →
                  在 SCLK 上升沿收取 DOUT (官方手册 V1.8 P29: "IC 外部在时钟上升沿
                  收取 Data"; 下降沿是芯片切换下一位的时刻, 下降沿后采样会读到
                  下一位导致整字节左移, 2026-08-18 修正) → SCLK=0 → delay_us(1);
                  8 bit 循环完成, 每字节约 16µs
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
        delay_us(1);                                    /* DOUT 稳定裕量 (自上一下降沿起有效) */
        if (GPIO_ReadInputDataBit(h->out.port, h->out.pin) != Bit_RESET) {
            rx |= (u8)(1u << i);                        /* 上升沿收取 DOUT 位 (MSB 先行) */
        }
        GPIO_ResetBits(h->sclk.port, h->sclk.pin);      /* SCLK=0, 下降沿芯片切换下一位 */
        delay_us(1);                                    /* 下降沿到下次上升沿间隔 */
    }

    return rx;
}

/*
    @brief      : 读取一个数据字节 (E6 配方, 厂商确认 DOUT 下降沿输出)
    @note       : bit7 在前一个字节(或命令字)的最后下降沿已输出, SCLK 为低时
                  立即预取; bit6..bit0 依次在后续 7 个数据时钟下降沿输出,
                  在下降沿瞬间采样; 末尾补 1 个空时钟完成 8 时钟帧 (下一字节的
                  bit7 恰在此下降沿输出, 连续调用自然衔接);
                  2026-08-18 E6 配方上板验证: 全部寄存器含 bit0 完整读出
    @param[in]  : h    LC1258 实例句柄
    @param[out] : none
    @retval     : 完整 8 位数据
*/
static u8 lc1258_read_byte_mode1(lc1258_handle_t *h)
{
    u8  rx = 0;
    s32 i;

    /* 命令字节最后下降沿后 DOUT 已输出 bit7 (SCLK 当前为低) */
    if (GPIO_ReadInputDataBit(h->out.port, h->out.pin) != Bit_RESET) {
        rx |= 0x80u;                                    /* 预取 bit7 */
    }

    /* 7 个数据时钟, 每个下降沿瞬间采样 bit6..bit0 */
    for (i = 6; i >= 0; i--) {
        GPIO_SetBits(h->sclk.port, h->sclk.pin);        /* SCLK=1 */
        __NOP();
        GPIO_ResetBits(h->sclk.port, h->sclk.pin);      /* SCLK=0 下降沿, DOUT 输出下一位 */
        if (GPIO_ReadInputDataBit(h->out.port, h->out.pin) != Bit_RESET) {
            rx |= (u8)(1u << i);                        /* 下降沿瞬间采样 */
        }
    }

    /* 补第 8 个空时钟, 完成数据字节 8 时钟帧时序 */
    GPIO_SetBits(h->sclk.port, h->sclk.pin);
    __NOP();
    GPIO_ResetBits(h->sclk.port, h->sclk.pin);

    return rx;
}

/*
    @brief      : CS 边沿后等待 td(SCCS)/td(CSSC)
    @note       : CS 拉低后需等 td(SCCS) 再发命令, CS 拉高后需等 td(CSSC)
                  才能再次拉低, 均 ≥2tCLK (fCLK=16MHz → tCLK=62.5ns → ≈125ns),
                  保守用 delay_us(1) 实现 (厂商模板 hal.c setCS 两沿均延时)
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
static void lc1258_cs_setup_delay(void)
{
    delay_us(1);                                        /* ≥125ns 时序要求, 1us 保守裕量 */
}

/*
    @brief      : 初始化 LC1258
    @note       : 流程按厂商模板 adcStartupRoutine 对齐 (2026-08-17):
                   RST 低脉冲复位 (≥2 个系统时钟, 保守 100us) → 释放后 tWAKE
                   等待 5ms (芯片加载默认寄存器与滤波缓存复位, 模板 delay_ms(5))
                   → 读 ID 寄存器校验 == 0x8B → 全量写 9 个寄存器
                   (CONFIG0=0x0A Auto-Scan+状态字节; CONFIG1=0x41
                    DLY=64us+DRATE=01, 模板值; MUXSCH/MUXDIF/SYSRED/GPIOD=0x00,
                    MUXSG0/MUXSG1=0xFF 单端全开, GPIOC=0xFF 全输入)
                   → 回读 CONFIG0/CONFIG1/MUXSG0/MUXSG1 校验写值
                   (模板 write 后 readMultipleRegisters 回读验证);
                   本板无 PWDN/CLKSEL 引脚 (硬件固定), 对应步骤不适用
    @param[in]  : h    LC1258 实例句柄
    @param[out] : none
    @retval     : 1 = 成功 (ID 与回读校验通过); 0 = 失败 (句柄无效/ID 不匹配/回读不一致)
*/
u8 lc1258_init(lc1258_handle_t *h)
{
    u8 id = 0;
    u8 rb = 0;

    if (h == NULL) {
        return 0;
    }

    GPIO_SetBits(h->cs.port, h->cs.pin);                /* CS=1, 总线空闲态 */
    GPIO_ResetBits(h->start.port, h->start.pin);        /* START=0, 复位期间停止转换 */

    GPIO_ResetBits(h->rst.port, h->rst.pin);            /* RST=0, 触发硬件复位 */
    delay_us(100);                                      /* ≥2 个系统时钟 (125ns), 保守 100us */
    GPIO_SetBits(h->rst.port, h->rst.pin);              /* RST=1, 释放复位 */
    delay_ms(5);                                        /* tWAKE: 模板 delay_ms(5), 等待默认寄存器加载完成 */

    if (lc1258_read_reg(h, LC1258_REG_ID, &id) == 0) {
        return 0;
    }
    if (id != LC1258_CHIP_ID) {                         /* 芯片 ID 校验 */
        return 0;
    }

    /* 全量写 9 个寄存器 (与模板 initRegisterMap 一致, 复位后本为默认值, 重写保证确定性) */
    lc1258_write_reg(h, LC1258_REG_CONFIG0, 0x0A);      /* Auto-Scan + 带状态字节, 内部直连, 不开斩波 */
    lc1258_write_reg(h, LC1258_REG_CONFIG1, 0x41);      /* Standby + DLY=64us + 无偏置 + DRATE=01 (模板值: CONFIG1_DLY_64us|CONFIG1_DRATE_7813SPS) */
    lc1258_write_reg(h, LC1258_REG_MUXSCH,  0x00);      /* Fixed 模式通道选择 (Auto-Scan 下无效, 写默认) */
    lc1258_write_reg(h, LC1258_REG_MUXDIF,  0x00);      /* 差分通道全部关闭 */
    lc1258_write_reg(h, LC1258_REG_MUXSG0,  0xFF);      /* 单端通道 AIN0~AIN7 全部开启 */
    lc1258_write_reg(h, LC1258_REG_MUXSG1,  0xFF);      /* 单端通道 AIN8~AIN15 全部开启 */
    lc1258_write_reg(h, LC1258_REG_SYSRED,  0x00);      /* 内部监测通道全部关闭 */
    lc1258_write_reg(h, LC1258_REG_GPIOC,   0xFF);      /* 芯片 GPIO 全输入 (默认) */
    lc1258_write_reg(h, LC1258_REG_GPIOD,   0x00);      /* 芯片 GPIO 输出电平清零 (默认) */

    /* 回读关键寄存器校验 (模板 write 后回读验证) */
    if (lc1258_read_reg(h, LC1258_REG_CONFIG0, &rb) == 0 || rb != 0x0A) {
        return 0;
    }
    if (lc1258_read_reg(h, LC1258_REG_CONFIG1, &rb) == 0 || rb != 0x41) {
        return 0;
    }
    if (lc1258_read_reg(h, LC1258_REG_MUXSG0, &rb) == 0 || rb != 0xFF) {
        return 0;
    }
    if (lc1258_read_reg(h, LC1258_REG_MUXSG1, &rb) == 0 || rb != 0xFF) {
        return 0;
    }

    return 1;
}

/*
    @brief      : 写单个配置寄存器
    @note       : 官方手册 V1.8: 寄存器 6 位地址, 写前必须先发高 2 位地址前缀命令
                  (本工程寄存器 00h~09h A5A4=00 → 前缀 0xB0);
                  随后 WREG 命令 (011 0 A[3:0], MUL=0), 命令后紧跟 1 字节数据
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
    lc1258_cs_setup_delay();                            /* td(SCCS) ≥ 125ns */

    lc1258_spi_byte(h, LC1258_CMD_ADDR_MSB_00);         /* 高 2 位地址前缀 (A5A4=00), 必发 */
    lc1258_spi_byte(h, (u8)(LC1258_CMD_WREG | (addr & 0x0F)));  /* 命令 011 0 A[3:0] */
    lc1258_spi_byte(h, val);                            /* 1 字节寄存器数据 */

    GPIO_SetBits(h->cs.port, h->cs.pin);                /* CS=1, 结束通信 */
    lc1258_cs_setup_delay();                            /* td(CSSC) ≥ 125ns */

    return 1;
}

/*
    @brief      : 读单个配置寄存器
    @note       : 官方手册 V1.8: 寄存器 6 位地址, 读前必须先发高 2 位地址前缀命令
                  (本工程寄存器 00h~09h A5A4=00 → 前缀 0xB0);
                  随后 RREG 命令 (010 0 A[3:0], MUL=0), 数据字节按 E6 配方读取
                  (bit7 预取 + 下降沿瞬间采样, 见 lc1258_read_byte_mode1)
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
    lc1258_cs_setup_delay();                            /* td(SCCS) ≥ 125ns */

    lc1258_spi_byte(h, LC1258_CMD_ADDR_MSB_00);         /* 高 2 位地址前缀 (A5A4=00), 必发 */
    lc1258_spi_byte(h, (u8)(LC1258_CMD_RREG | (addr & 0x0F)));  /* 命令 010 0 A[3:0] */
    *val = lc1258_read_byte_mode1(h);                   /* 数据字节: E6 配方 */

    GPIO_SetBits(h->cs.port, h->cs.pin);                /* CS=1, 结束通信 */
    lc1258_cs_setup_delay();                            /* td(CSSC) ≥ 125ns */

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
                  CS=0 → 发命令 0x30 (MUL 必须=1, 通道数据读无需高 2 位地址前缀)
                  → 连续读 4 字节 [1 字节状态][3 字节 24bit ADC 数据], 每个字节
                  用 E6 配方 (bit7 预取 + 下降沿瞬间采样, 见 lc1258_read_byte_mode1,
                  连续字节自然衔接);
                  读取操作不影响进行中的转换 (数据手册 RDATA 缓冲语义);
                  状态字节含 NEW/OVF/SUPPLY/CHID, 此处仅取 CHID 返回, 其余位丢弃,
                  NEW/OVF 监测由上层通过轮询 DRDY 保证数据新鲜;
                  RDATA 为缓冲读, 缓冲数据不会被新转换覆盖 (未读期间新转换结果不更新缓冲),
                  上层须按 DRDY 节奏及时读取以保证数据新鲜 (否则丢失的是后续样本的新鲜度, 而非缓冲值)
                  24bit → s32 符号扩展 (不再 <<1, E6 采样读回完整位流;
                  上层按 code/8388608×VREF 换算, 手册默认 1LSB=VREF/800000h)
    @param[in]  : h     LC1258 实例句柄
    @param[out] : chid  状态字节低 5 位 = 当前采样通道编号 (Auto-Scan 有效), 可传 NULL
    @retval     : 24bit 补码符号扩展后的采样值 (±0x7FFFFF 量级);
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
    lc1258_cs_setup_delay();                            /* td(SCCS) ≥ 125ns */

    lc1258_spi_byte(h, LC1258_CMD_RDATA);               /* 命令 001 1 xxxx, 返回值丢弃 */
    status = lc1258_read_byte_mode1(h);                 /* 状态字节: NEW/OVF/SUPPLY/CHID */
    b1 = lc1258_read_byte_mode1(h);                     /* 24bit ADC 数据高字节 */
    b2 = lc1258_read_byte_mode1(h);                     /* 中字节 */
    b3 = lc1258_read_byte_mode1(h);                     /* 低字节 */

    GPIO_SetBits(h->cs.port, h->cs.pin);                /* CS=1, 结束通信 */
    lc1258_cs_setup_delay();                            /* td(CSSC) ≥ 125ns */

    if (chid != NULL) {
        *chid = status & LC1258_STAT_CHID;              /* 当前通道编号 (低 5 位) */
    }

    raw = (s32)(((u32)b1 << 24) | ((u32)b2 << 16) | ((u32)b3 << 8)) >> 8;   /* 24bit 补码符号扩展 */

    return raw;
}
