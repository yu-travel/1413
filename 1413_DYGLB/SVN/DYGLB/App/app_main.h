#ifndef __APP_MAIN_H_
#define __APP_MAIN_H_

/*
    @brief      : App 层主流程 (分层初始化 + MultiTimer 周期任务注册)
    @note       : 1. main() 于本模块实现 (替代旧 User/main.c):
                     NVIC 分组 -> delay_init -> bsp_board_init -> usart1
                     -> bsp_spi -> bsp_timer -> softtimer -> power_init
                     -> monitor_init -> bsp_iwdg -> app_tasks_init
                     -> 主循环 (softtimer_loop + 喂狗)
                  2. 周期任务由 app_tasks_init() 注册 (MultiTimer 单次触发,
                     回调内重注册实现周期), 周期宏见 app_config.h
                      TASK_*_MS
                  3. 依赖顺序: power/monitor 初始化完成后才注册任务,
                     任务回调依赖的全局数据 (g_monitor 等) 均先就绪
*/

void app_tasks_init(void);      /* 注册 5 个 MultiTimer 周期任务 */

#endif /* __APP_MAIN_H_ */
