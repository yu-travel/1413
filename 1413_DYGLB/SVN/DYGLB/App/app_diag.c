#include "app_diag.h"
#include "app_config.h"
#include "app_monitor.h"
#include "app_power.h"
#include "board_map.h"
#include "delay.h"
#include "xca4001.h"

#if CHIP_TEST_LOG

/*
    @brief      : 打印 4 轨 XCA4001 ALERT 状态
    @note       : 经 Dev/xca4001 驱动读取 (handle 由 board_map.h 引脚宏局部构造);
                  低电平=过流告警, 正常=上拉高
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
static void diag_xca_dump(void)
{
    static const char *const s_name[4] = { "3V3", "12V0", "5V0", "28V0" };
    xca4001_handle_t h[4] = {
        { VCC_3V3_RST_PORT,  VCC_3V3_RST_PIN,  VCC_3V3_ALERT_PORT,  VCC_3V3_ALERT_PIN  },
        { VCC_12V0_RST_PORT, VCC_12V0_RST_PIN, VCC_12V0_ALERT_PORT, VCC_12V0_ALERT_PIN },
        { VCC_5V0_RST_PORT,  VCC_5V0_RST_PIN,  VCC_5V0_ALERT_PORT,  VCC_5V0_ALERT_PIN  },
        { VCC_28V0_RST_PORT, VCC_28V0_RST_PIN, VCC_28V0_ALERT_PORT, VCC_28V0_ALERT_PIN },
    };
    u8 i;

    TRACE_OUT_2(DEBUG_OUT, "[DIAG] XCA4001 ALERT: ");
    for (i = 0u; i < 4u; i++) {
        TRACE_OUT_2(DEBUG_OUT, "%s=%s ", s_name[i],
                  (xca4001_alert_active(&h[i]) != 0u) ? "ALARM" : "ok");
    }
    TRACE_OUT_2(DEBUG_OUT, "\r\n");
}

/*
    @brief      : 上电一次性诊断
    @note       : 在 monitor_init 之后、bsp_iwdg_init 之前调用
                  (主动测试期间喂狗未启动, 无复位风险);
                  仅做打印与主动测试, 不改运行配置
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void diag_init(void)
{
    /* 初始化 SEGGER RTT 通道 2 (buffer 2), 仅在 CHIP_TEST_LOG=1 时使用 */
    static char s_diag_rtt_buf[8192];
    SEGGER_RTT_ConfigUpBuffer(2, "DIAG", s_diag_rtt_buf, sizeof(s_diag_rtt_buf), SEGGER_RTT_MODE_NO_BLOCK_TRIM);

#if DIAG_EFUSE_ACTIVE_TEST
    power_diag_test_seq();
#else
    TRACE_OUT_2(DEBUG_OUT, "[DIAG] efuse active test disabled (DIAG_EFUSE_ACTIVE_TEST=0)\r\n");
#endif
}

/*
    @brief      : 周期诊断打印
    @note       : 由 app_main.c 按 TASK_DIAG_MS 周期调用;
                  打印 ADC 采集值 / 15 设备物理量 / DAC 下发值 / efuse 引脚 /
                  XCA4001 告警, 用于逐项核对芯片工作
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void diag_task(void)
{
    TRACE_OUT_2(DEBUG_OUT, "[DIAG] ===== ADC Raw Sampling =====\r\n");
    monitor_diag_dump();
    TRACE_OUT_2(DEBUG_OUT, "[DIAG] ===== Power DAC & Pin Status =====\r\n");
    power_diag_dump();
    TRACE_OUT_2(DEBUG_OUT, "[DIAG] ===== XCA4001 Alert =====\r\n");
    diag_xca_dump();
}

#endif /* CHIP_TEST_LOG */
