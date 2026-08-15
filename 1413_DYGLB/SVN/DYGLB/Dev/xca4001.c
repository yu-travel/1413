#include "xca4001.h"
#include "delay.h"

/*
    @brief      : XCA4001 电流监控芯片模式切换与锁存清除驱动
    @note       : 1. RESET 引脚方向与初始电平 (推挽输出, 初始高 = 锁存模式)
                     由 bsp_board.c 完成初始化
                  2. 清除锁存要求 RESET ≥100ns 低脉冲, 若脉冲期间电流已回落
                     (V_OUT < V_LIMIT) 则 ALERT 自动恢复高, 否则保持报警低;
                     保守用 delay_us(1) 实现, 裕量约 10 倍
                  3. 清锁存后 RESET 重新拉高回到锁存模式 (数据手册场景);
                     如需自恢复模式, 调用者应另行调用 xca4001_set_auto_mode()
                  4. ALERT 开漏输出, 上拉输入由 bsp_board.c 配置, 低 = 过流
                  5. delay_us() 依赖 delay_init 已由 main.c 完成
*/

/*
    @brief      : 设置锁存模式 (RESET=1)
    @note       : 过流 ALERT 拉低后锁存, 即使电流恢复 ALERT 也不会自动回高,
                 需调用 xca4001_clear_latch() 清除
    @param[in]  : h    XCA4001 实例句柄
    @param[out] : none
    @retval     : none
*/
void xca4001_set_latch_mode(xca4001_handle_t *h)
{
    if (h == NULL) {
        return;
    }

    GPIO_SetBits(h->rst_port, h->rst_pin);              /* RESET=1, 锁存模式 */
}

/*
    @brief      : 设置自恢复模式 (RESET=0)
    @note       : 过流 ALERT 拉低, 电流回落后 ALERT 自动恢复高,
                 无需软件干预, 适合硬件中断实时监测场景
    @param[in]  : h    XCA4001 实例句柄
    @param[out] : none
    @retval     : none
*/
void xca4001_set_auto_mode(xca4001_handle_t *h)
{
    if (h == NULL) {
        return;
    }

    GPIO_ResetBits(h->rst_port, h->rst_pin);            /* RESET=0, 自恢复模式 */
}

/*
    @brief      : 清除锁存报警 (RESET ≥100ns 低脉冲)
    @note       : 脉冲期间若电流已回落 (V_OUT < V_LIMIT) 则 ALERT 恢复高,
                 若仍过流则 ALERT 保持报警低;
                 脉冲结束后 RESET 重新拉高回到锁存模式,
                 调用者如需自恢复模式请再调 xca4001_set_auto_mode()
    @param[in]  : h    XCA4001 实例句柄
    @param[out] : none
    @retval     : none
*/
void xca4001_clear_latch(xca4001_handle_t *h)
{
    if (h == NULL) {
        return;
    }

    GPIO_ResetBits(h->rst_port, h->rst_pin);            /* RESET=0, 开始清锁存低脉冲 */
    delay_us(1);                                        /* ≥100ns 低脉冲, 1us 保守裕量 */
    GPIO_SetBits(h->rst_port, h->rst_pin);              /* RESET=1, 回到锁存模式等待下次过流 */
}

/*
    @brief      : 查询过流报警状态
    @note       : ALERT 开漏输出, 正常时高阻 (上拉高), 过流时内部 MOS 导通拉低
    @param[in]  : h    XCA4001 实例句柄
    @param[out] : none
    @retval     : 1 = 过流报警 (ALERT=0); 0 = 正常或句柄无效
*/
u8 xca4001_alert_active(xca4001_handle_t *h)
{
    if (h == NULL) {
        return 0;
    }

    return (GPIO_ReadInputDataBit(h->alert_port, h->alert_pin) == Bit_RESET) ? 1u : 0u;
}
