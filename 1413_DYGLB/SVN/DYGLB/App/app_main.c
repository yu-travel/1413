#include "app_main.h"
#include "app_config.h"
#include "app_monitor.h"
#include "app_power.h"
#include "app_protocol.h"
#include "bsp_board.h"
#include "bsp_iwdg.h"
#include "bsp_spi.h"
#include "bsp_timer.h"
#include "delay.h"
#include "softtimer.h"
#include "usart.h"

/*
    @brief      : App 层主流程 (分层初始化 + MultiTimer 周期任务调度)
    @note       : 1. 初始化顺序: NVIC 分组 -> delay -> 板级 GPIO/JTAG
                  -> 调试串口 -> FPGA SPI -> 定时器 (TIM2 1kHz tick /
                  TIM3 1ms) -> softtimer -> 电源 (DAC+默认限流0mA+EN全关)
                  -> 采集 (4 片 ADC ID 校验 + START + Flash 校准)
                  -> 看门狗 (1.6s) -> 周期任务注册 -> 主循环
                  2. 主循环只做两件事: MultiTimer 到期回调分发 + 喂狗;
                  所有业务在周期任务回调内执行, 回调运行于主循环上下文
                  (非中断), 无重入问题
                  3. MultiTimer 为单次触发: 各任务回调末尾重注册自身
                  实现周期, 周期宏见 app_config.h TASK_*_MS
                  4. 协议收发周期: 100ms 上传 (MCU->FPGA) / 200ms 下发
                  (FPGA->MCU), 两任务同上下文顺序执行, 片选各自完整管理,
                  无 SPI 冲突
                  5. default_state 无持久化 (板载无 EEPROM), 掉电即失,
                  由 FPGA 每次上电重发 (待确认#5 已定)
*/

/* MultiTimer 周期任务句柄 (静态分配) */
static MultiTimer s_timer_monitor;
static MultiTimer s_timer_convert;
static MultiTimer s_timer_upload;
static MultiTimer s_timer_proto;
static MultiTimer s_timer_heartbeat;

/* 上传帧发送缓冲与 15 组测量值 (静态分配不进栈) */
static u8            s_tx_buf[PROTO_TX_BUF_LEN];
static dev_measure_t s_measure[PROTO_DEV_NUM];

/* 下发帧解包数据 (静态分配不进栈) */
static dev_threshold_t s_down_thr[PROTO_DEV_NUM];
static power_state_t   s_down_ps;

/* FPGA 下发的电源默认状态字 (上传帧回告;
   无持久化, 掉电即失, FPGA 每次上电重发) */
static u16 s_default_state;

/* 是否已成功消费过下发帧 (首次帧默认状态语义用) */
static u8 s_first_frame;

/*
    @brief      : 1ms 采集任务回调: 轮询 4 片 ADC DRDY, 就绪即读样本
    @note       : 重注册自身实现 1ms 周期
    @param[in]  : timer     MultiTimer 句柄 (重注册用)
                  user_data 未使用
    @param[out] : none
    @retval     : none
*/
static void task_monitor_cb(MultiTimer *timer, void *user_data)
{
    (void)user_data;

    monitor_task();

    softtimer_start(timer, TASK_MONITOR_MS, task_monitor_cb, timer);
}

/*
    @brief      : 100ms 换算任务回调: 码值→物理量 (换算+校准) + 告警判定
    @note       : 重注册自身实现 100ms 周期
    @param[in]  : timer     MultiTimer 句柄 (重注册用)
                  user_data 未使用
    @param[out] : none
    @retval     : none
*/
static void task_convert_cb(MultiTimer *timer, void *user_data)
{
    (void)user_data;

    monitor_convert_all();

    softtimer_start(timer, TASK_CONVERT_MS, task_convert_cb, timer);
}

/*
    @brief      : 100ms 上传任务回调: 采集数据组包整帧发送 (MCU->FPGA)
    @note       : 15 组测量值 + 状态字 -> protocol_build_upload (104B)
                  -> 片选内 SPI 整帧发送;
                  default_state 回告 FPGA 最近一次下发的值 (无本地持久化)
    @param[in]  : timer     MultiTimer 句柄 (重注册用)
                  user_data 未使用
    @param[out] : none
    @retval     : none
*/
static void task_upload_cb(MultiTimer *timer, void *user_data)
{
    u16 len;
    power_state_t ps;

    (void)user_data;

    monitor_measure_to_protocol(s_measure);

    ps.default_state = s_default_state;
    ps.switch_state  = power_get_switch_state();
    ps.alarm_state   = monitor_get_alarm_state();

    len = protocol_build_upload(s_tx_buf, s_measure, &ps);
    if (len != 0u) {
        bsp_spi_cs(BSP_SPI_CS_LOW);
        bsp_spi_transfer(s_tx_buf, NULL, len);
        bsp_spi_cs(BSP_SPI_CS_HIGH);
    }

    softtimer_start(timer, TASK_UPLOAD_MS, task_upload_cb, timer);
}

/*
    @brief      : 200ms 协议下发任务回调: 前导帧收发 -> 解包 -> 消费指令
    @note       : 仅本次周期解包成功且新鲜时消费 (失败周期保留旧状态):
                  a. monitor_set_threshold  更新 15 路基准阈值
                  b. 逐路 power_set_limit   写限流 DAC 输入寄存器 (缓存);
                     简单实现每帧全量写 15 路, 可靠且无状态比较开销
                     (优化空间: 与上次帧比较仅写变化路, 联调稳定后可做)
                  c. power_flush_limits     4 片 DAC LDAC 同步刷新
                  d. 首次收到帧且 FPGA 未明确开关 (switch_state==0) 且
                     default_state!=0: 应用开机默认状态 (待联调确认语义)
                  e. power_apply_state      应用 FPGA 下发开关状态
                  解析诊断打印由 app_protocol.c 负责, 本层不再重复打印
    @param[in]  : timer     MultiTimer 句柄 (重注册用)
                  user_data 未使用
    @param[out] : none
    @retval     : none
*/
static void task_proto_cb(MultiTimer *timer, void *user_data)
{
    u8 ok;
    u8 fresh;
    u16 id;

    (void)user_data;

    ok = protocol_read_task();
    protocol_read_result(s_down_thr, &s_down_ps, &fresh);

    if (ok != 0u && fresh != 0u) {
        /* a. 更新 15 路基准阈值 (告警判定用) */
        monitor_set_threshold(s_down_thr);

        /* b. 逐路限流写 DAC 输入寄存器 (缓存) */
        for (id = 1u; id < DEV_NUM; id++) {
            power_set_limit((dev_id_e)id, s_down_thr[id - 1u].ref_cur_ma);
        }

        /* c. 4 片 DAC LDAC 脉冲同步刷新 */
        power_flush_limits();

        /* d. 开机默认状态: 首次收帧且 FPGA 未明确开关且默认状态非 0
           (待联调确认语义) */
        if (s_first_frame == 0u &&
            s_down_ps.switch_state == 0u &&
            s_down_ps.default_state != 0u) {
            power_apply_state(s_down_ps.default_state);
        }

        /* e. 应用 FPGA 下发开关状态 */
        power_apply_state(s_down_ps.switch_state);

        /* 记录 FPGA 默认状态字供上传帧回告 */
        s_default_state = s_down_ps.default_state;
        s_first_frame = 1u;
    }

    softtimer_start(timer, TASK_PROTO_MS, task_proto_cb, timer);
}

/*
    @brief      : 1000ms RTT 心跳任务回调: 输出一条心跳日志指示运行
    @note       : 新板无调试 LED (PD2 为 HWXJ2_PWR_EN 电源使能,
                  不可当 LED 用), 改由 RTT 日志心跳替代;
                  重注册自身实现 1000ms 周期, 每秒一行不刷屏
    @param[in]  : timer     MultiTimer 句柄 (重注册用)
                  user_data 未使用
    @param[out] : none
    @retval     : none
*/
static void task_heartbeat_cb(MultiTimer *timer, void *user_data)
{
    static u32 hb_count = 0u;

    (void)user_data;

    hb_count++;
    TRACE_OUT(DEBUG_OUT, "<DYGLB> hb tick %u\r\n", hb_count);

    softtimer_start(timer, TASK_HEARTBEAT_MS, task_heartbeat_cb, timer);
}

/*
    @brief      : 注册 MultiTimer 周期任务 (主循环前调用一次)
    @note       : 周期宏 TASK_*_MS 见 app_config.h;
                  MultiTimer 单次触发, 各回调末尾自重注册实现周期
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void app_tasks_init(void)
{
    softtimer_start(&s_timer_monitor,   TASK_MONITOR_MS,   task_monitor_cb,   &s_timer_monitor);
    softtimer_start(&s_timer_convert,   TASK_CONVERT_MS,   task_convert_cb,   &s_timer_convert);
    softtimer_start(&s_timer_upload,    TASK_UPLOAD_MS,    task_upload_cb,    &s_timer_upload);
    softtimer_start(&s_timer_proto,     TASK_PROTO_MS,     task_proto_cb,     &s_timer_proto);
    softtimer_start(&s_timer_heartbeat, TASK_HEARTBEAT_MS, task_heartbeat_cb, &s_timer_heartbeat);
}

/*
    @brief      : 主函数: 分层初始化 + 主循环 (任务分发 + 喂狗)
    @note       : 1. 初始化顺序见文件头注释; 看门狗最后启动,
                  避免 Flash 校准等慢操作期间被误复位
                  2. 主循环 softtimer_loop() 分发到期任务回调,
                  随后喂狗; 若任一任务回调阻塞超 1.6s, 看门狗复位
    @param[in]  : none
    @param[out] : none
    @retval     : 0 (启动后不返回, 由看门狗/复位接管)
*/
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);    /* 中断优先级分组 2 */
    delay_init(168);                                   /* SysTick 延时, 168MHz */
    bsp_board_init();                                  /* GPIO/JTAG 板级初始化 */
    usart1_init(115200);                               /* 调试串口 */
    bsp_spi_init();                                    /* FPGA SPI1 */
    bsp_timer_init();                                  /* TIM2 1kHz tick + TIM3 1ms */
    softtimer_init();                                  /* MultiTimer 安装 tick 源 */
    power_init();                                      /* DAC + 默认限流 0mA + EN 全关 */
    monitor_init();                                    /* 4 片 ADC ID 校验 + START + Flash 校准 */
    bsp_iwdg_init();                                   /* 1.6s 独立看门狗 */
    app_tasks_init();                                  /* 注册周期任务 */
    TRACE_OUT(DEBUG_OUT, "<DYGLB> init done, enter loop\r\n");

    while (1) {
        softtimer_loop();                              /* 分发到期任务回调 */
        bsp_iwdg_feed();                               /* 喂狗 */
    }
}
