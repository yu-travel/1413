#ifndef __APP_DIAG_H_
#define __APP_DIAG_H_

#include "app_config.h"

/*
    @brief      : 芯片联调诊断 (App 层): ADC/DAC/efuse/XCA4001 工作状态打印
    @note       : 1. 总开关 CHIP_TEST_LOG (app_config.h), 联调完成后置 0 即整体关闭;
                   2. diag_init()  上电一次性: 可触发 15 路 efuse 主动上电测试
                      (DIAG_EFUSE_ACTIVE_TEST=1, 真实供电, 注意负载安全);
                   3. diag_task()  周期调用: 打印 ADC 采集值 / DAC 下发值 /
                      efuse 引脚状态 / XCA4001 告警, 便于逐项核对芯片工作
*/

void diag_init(void);   /* 上电一次性诊断 (主动测试等), 在 monitor_init 后调用 */
void diag_task(void);   /* 周期诊断打印 (app_main.c 按 TASK_DIAG_MS 注册) */

#endif /* __APP_DIAG_H_ */
