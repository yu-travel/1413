#include "gda6641.h"

/*
    @brief      : GDA6641 四通道 16 位电压输出 DAC 驱动
                  采用 GPIO 位操作模拟 SPI 通信 (SPI Mode1)
    @note       : 1. 引脚方向与空闲电平由 bsp_board.c 完成初始化
                     (SCLK=0, SYNC/LDAC/POR/CLR=1, DIN=0), 速度 100MHz 推挽
                  2. 168MHz 系统时钟下, 单条 GPIO 读写 (Set/ResetBits 函数调用
                     约 20~40ns) 本身已满足芯片 ≥20ns 最小脉宽/间隔要求,
                     此处仍按位插入 __NOP() 增加时序裕量
                  3. 时序基准: 单条指令约 6ns (1/168MHz), 4 条 __NOP 约 24ns
*/

/*============================================================
    局部宏定义: 32bit 帧位域拼接 (MSB 先行)
    frame = DB31~28(无效, 0) | DB27~24(命令码) | DB23~20(通道) | DB19~4(DA值) | DB3~0(辅助, 0)
============================================================*/
#define GDA6641_CMD_WRITE_INPUT     0x0u    /* 仅写输入寄存器 (缓存, 不刷新输出) */
#define GDA6641_CMD_WRITE_UPDATE    0x2u    /* 写输入 + 同步刷新 (软件 LDAC) */

/*
    @brief      : 满足芯片最小低电平脉宽/间隔 (≥20ns) 的短延时
    @note       : 168MHz 下 4 条 __NOP 约 24ns, 叠加 GPIO 函数调用开销裕量充足
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
static void gda6641_delay_20ns(void)
{
    __NOP();
    __NOP();
    __NOP();
    __NOP();
}

/*
    @brief      : 发送一帧 32bit 数据 (MSB 先行)
    @note       : SPI Mode1: SYNC 拉低开启帧, 每 bit: SCLK 拉高 → 写 DIN →
                  __NOP 保持 (数据建立) → SCLK 拉低 (DIN 下降沿被锁存) →
                  __NOP 保持; 32 bit 发完 SYNC 拉高锁存帧
    @param[in]  : h     GDA6641 实例句柄
    @param[in]  : frame 32bit 控制 + 数据帧
    @param[out] : none
    @retval     : none
*/
static void gda6641_send_frame(gda6641_handle_t *h, u32 frame)
{
    s32 i;

    GPIO_ResetBits(h->sync.port, h->sync.pin);          /* SYNC=0, 开启一帧 */

    for (i = 31; i >= 0; i--) {
        GPIO_SetBits(h->sclk.port, h->sclk.pin);        /* SCLK=1 */
        GPIO_WriteBit(h->din.port, h->din.pin,
                      (BitAction)((frame >> i) & 0x1u)); /* 写 DIN 位 (MSB 先行) */
        __NOP();                                        /* 数据建立保持 */
        GPIO_ResetBits(h->sclk.port, h->sclk.pin);      /* SCLK 下降沿锁存 DIN */
        __NOP();                                        /* 低电平保持 */
    }

    GPIO_SetBits(h->sync.port, h->sync.pin);            /* SYNC=1, 锁存帧 */
}

/*
    @brief      : 初始化 GDA6641 控制线为上电态 (POR=1, CLR=1, LDAC=1, SYNC=1)
    @note       : 不主动复位, GPIO 方向与初始电平已由 bsp_board.c 完成,
                  此处仅保证控制线处于非动作电平
    @param[in]  : h     GDA6641 实例句柄
    @param[out] : none
    @retval     : none
*/
void gda6641_init(gda6641_handle_t *h)
{
    if (h == NULL) {
        return;
    }

    GPIO_SetBits(h->por.port, h->por.pin);              /* POR=1, 输出半满度上电态 */
    GPIO_SetBits(h->clr.port, h->clr.pin);              /* CLR=1, 不触发清零 */
    GPIO_SetBits(h->ldac.port, h->ldac.pin);            /* LDAC=1, 异步模式常态 */
    GPIO_SetBits(h->sync.port, h->sync.pin);            /* SYNC=1, 无帧传输 */
}

/*
    @brief      : 写入单通道 DA 值并立即同步刷新输出 (命令码 0010, 软件 LDAC)
    @note       : 帧格式 = (0x2<<24) | (ch<<20) | (d<<4), 写完 SPI 帧电压立刻更新
    @param[in]  : h     GDA6641 实例句柄
    @param[in]  : ch    通道号 0~3 (VOUTA~VOUTD)
    @param[in]  : d     16 位 DA 数值 (0~65535), VOUT = VREFIN * d / 65536
    @param[out] : none
    @retval     : none
*/
void gda6641_write(gda6641_handle_t *h, u8 ch, u16 d)
{
    if (h == NULL || ch > 3) {
        return;
    }

    gda6641_send_frame(h, (GDA6641_CMD_WRITE_UPDATE << 24) | ((u32)ch << 20) |
                          ((u32)d << 4));
}

/*
    @brief      : 写入单通道 DA 值到输入寄存器, 不刷新输出 (命令码 0000)
    @note       : 数据仅缓存, 需调用 gda6641_update_all() 拉低 LDAC 统一刷新,
                  适合多通道同步输出场景
    @param[in]  : h     GDA6641 实例句柄
    @param[in]  : ch    通道号 0~3 (VOUTA~VOUTD)
    @param[in]  : d     16 位 DA 数值 (0~65535)
    @param[out] : none
    @retval     : none
*/
void gda6641_write_input(gda6641_handle_t *h, u8 ch, u16 d)
{
    if (h == NULL || ch > 3) {
        return;
    }

    gda6641_send_frame(h, (GDA6641_CMD_WRITE_INPUT << 24) | ((u32)ch << 20) |
                          ((u32)d << 4));
}

/*
    @brief      : LDAC 低脉冲统一刷新全部通道输出
    @note       : 异步模式: LDAC 下降沿将各通道输入寄存器数据加载到输出寄存器;
                  低脉冲宽度 ≥20ns, SCLK 下降沿到 LDAC 下降沿间隔 ≥20ns,
                  由 gda6641_delay_20ns() + GPIO 调用开销保证
    @param[in]  : h     GDA6641 实例句柄
    @param[out] : none
    @retval     : none
*/
void gda6641_update_all(gda6641_handle_t *h)
{
    if (h == NULL) {
        return;
    }

    gda6641_delay_20ns();                               /* 与上一帧 SCLK 下降沿间隔 ≥20ns */
    GPIO_ResetBits(h->ldac.port, h->ldac.pin);          /* LDAC=0, 下降沿刷新输出 */
    gda6641_delay_20ns();                               /* 低脉冲宽度 ≥20ns */
    GPIO_SetBits(h->ldac.port, h->ldac.pin);            /* LDAC=1, 回到常态 */
}

/*
    @brief      : CLR 低脉冲硬件异步清零
    @note       : CLR 下降沿立刻清空输入/输出寄存器, 输出回到预设清零电平,
                  优先级高于所有 SPI 写入指令
    @param[in]  : h     GDA6641 实例句柄
    @param[out] : none
    @retval     : none
*/
void gda6641_clear(gda6641_handle_t *h)
{
    if (h == NULL) {
        return;
    }

    GPIO_ResetBits(h->clr.port, h->clr.pin);            /* CLR=0, 下降沿清零 */
    gda6641_delay_20ns();                               /* 低脉冲宽度 ≥20ns */
    GPIO_SetBits(h->clr.port, h->clr.pin);              /* CLR=1, 释放清零 */
}

/*
    @brief      : POR 低脉冲软件复位
    @note       : 复位后所有寄存器恢复上电默认状态, 四路输出回到 POR 定义电平
                  (POR=1 时为半满度 VREFIN/2)
    @param[in]  : h     GDA6641 实例句柄
    @param[out] : none
    @retval     : none
*/
void gda6641_reset(gda6641_handle_t *h)
{
    if (h == NULL) {
        return;
    }

    GPIO_ResetBits(h->por.port, h->por.pin);            /* POR=0, 触发复位 */
    gda6641_delay_20ns();                               /* 低脉冲宽度 ≥20ns */
    GPIO_SetBits(h->por.port, h->por.pin);              /* POR=1, 输出半满度 */
}
