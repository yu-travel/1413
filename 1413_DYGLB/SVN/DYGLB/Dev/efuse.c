#include "efuse.h"
#include "delay.h"

/*
    @brief      : 电子保险丝 (MAC5048 / HQEF5016) 使能与锁存清除驱动
    @note       : 1. EN 高电平有效, GPIO 方向与初始电平 (推挽输出, 初始低)
                     由 bsp_board.c 完成初始化
                  2. 两芯片故障锁存清除时序一致: EN 低脉冲 (保守 100us) 后
                     重新拉高, 芯片清除锁存并重启软启动流程
                  3. 本驱动专注 EN 控制; MAC5048 的 FAULT 状态引脚直读
                     由 App 层完成 (dev_map_t.fault_port[]),
                     efuse_is_gok_goc() 仅面向 HQEF5016 的 GOK/GOC 设计
                     (MAC5048 handle 调用恒返回 0, 勿用于故障判定)
                  4. delay_us() 依赖 delay_init 已由 main.c 完成
*/

/*
    @brief      : 开启电子保险丝输出
    @note       : EN=1, 芯片执行软启动输出 VOUT
    @param[in]  : h    EFUSE 实例句柄
    @param[out] : none
    @retval     : none
*/
void efuse_on(efuse_handle_t *h)
{
    if (h == NULL) {
        return;
    }

    GPIO_SetBits(h->en_port, h->en_pin);                /* EN=1, 开启输出 */
}

/*
    @brief      : 关闭电子保险丝输出
    @note       : EN=0, 功率 MOS 关断, VOUT 停止输出
    @param[in]  : h    EFUSE 实例句柄
    @param[out] : none
    @retval     : none
*/
void efuse_off(efuse_handle_t *h)
{
    if (h == NULL) {
        return;
    }

    GPIO_ResetBits(h->en_port, h->en_pin);              /* EN=0, 关闭输出 */
}

/*
    @brief      : 清除故障锁存并重新软启动
    @note       : MAC5048/HQEF5016 均要求 EN 低脉冲清除锁存:
                  EN=0 → 保持 100us → EN=1, 芯片清除锁存并重启软启动流程
    @param[in]  : h    EFUSE 实例句柄
    @param[out] : none
    @retval     : none
*/
void efuse_clear_latch(efuse_handle_t *h)
{
    if (h == NULL) {
        return;
    }
    GPIO_ResetBits(h->en_port, h->en_pin);              /* EN=0, 进入清除锁存低脉冲 */
    delay_us(100);                                      /* 低电平保持 100us, 满足两芯片清锁存要求 */
    GPIO_SetBits(h->en_port, h->en_pin);                /* EN=1, 清除锁存并重新软启动 */
}

/*
    @brief      : 查询电子保险丝 GOK/GOC 状态 (仅 HQEF5016 有效)
    @note       : GOK/GOC 均为开漏输出, 低电平有效:
                  GOK=0 全局故障 (锁存, 需 efuse_clear_latch 清除),
                  GOC=0 稳态过流预警 (负载回落自动恢复);
                  任一拉低即返回 1
                  本函数仅面向 HQEF5016 的 GOK/GOC 设计:
                  MAC5048 的 handle 中 gok_port/goc_port 为 NULL/0,
                  本函数不读取且恒返回 0, 其 FAULT 引脚由 App 层经
                  dev_map_t.fault_port[] 直读, 切勿用本函数判定 MAC5048 故障
    @param[in]  : h    EFUSE 实例句柄
    @param[out] : none
    @retval     : 1 = GOK/GOC 任一拉低 (故障/过流预警); 0 = 无故障或句柄无效
*/
u8 efuse_is_gok_goc(efuse_handle_t *h)
{
    if (h == NULL) {
        return 0;
    }

    if (h->gok_port != NULL) {
        if (GPIO_ReadInputDataBit(h->gok_port, h->gok_pin) == Bit_RESET) {
            return 1;                                   /* GOK 拉低 = 全局故障 */
        }
    }

    if (h->goc_port != NULL) {
        if (GPIO_ReadInputDataBit(h->goc_port, h->goc_pin) == Bit_RESET) {
            return 1;                                   /* GOC 拉低 = 稳态过流预警 */
        }
    }

    return 0;
}
