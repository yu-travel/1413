/*
**************************************************************************************************************
    @brief      : 软定时器接口及软定时任务
    @author     : xiongjiqi
    @date       : 24/07/01
**************************************************************************************************************
*/
#include "timer.h"
#include "adc.h"
#include "gpio_app.h"
#include "main.h"
#include "softtimer.h"
#include "at8591t.h"
#include "main.h"

/* 软定时事件声明 */
void blink_timer_start(void);
void system_monitor_timer_start(void);
void system_reset_timer_start(void);

uint64_t PlatformTicksGetFunc(void)
{
    return timer_getms_count();
}

int softtimer_start(_time_t *timer, uint64_t period_ms, MultiTimerCallback_t callback, void *userdata)
{
    int ret = 0;
    ret = MultiTimerStart(timer, period_ms, callback, userdata);
    return ret;
}

int softtimer_stop(_time_t *timer)
{
    int ret = 0;
    ret = MultiTimerStop(timer);
    return ret;
}

/*
    @brief      : led定时器
*/
_time_t blink_timer = {0};

void blinkled_callback(_time_t *timer, void *userData)
{
    static u32 blink_count = 0;

    blink_count++;
    if (blink_count % 2 == 0)
        blink_led_set(LED_ON);
    else
        blink_led_set(LED_OFF);

    blink_timer_start(); // 开启下一个周期
}
void blink_timer_start(void)
{
    softtimer_start(&blink_timer, 500, blinkled_callback, &blink_timer);
}

/*
    @brief      : 系统复位检测定时器
        @note   :
*/
_time_t sysreset_timer = {0};
DECLARED_GPIO_APP_GET(SYS_RESET);

void system_reset_callback(_time_t *timer, void *userData)
{
    if (gpio_getting(&app_SYS_RESET, 0) == 0)
    {
        NVIC_SystemReset(); // 系统复位
    }
    system_monitor_timer_start(); // 开启下一个周期
}
void system_reset_timer_start(void)
{
    softtimer_start(&sysreset_timer, 100, system_reset_callback, &sysreset_timer);
}

/*
    @brief      : 系统监测定时器
        @note   : 监测项
            3路温度，电压，电流
*/
_time_t sysmon_timer = {0};
void system_monitor_callback(_time_t *timer, void *userData)
{
    // at8951t_convert(&sysmon.voltage);       // 更新电压电流
    board_voltage_get(&sysmon);             // 更新板载1.2V 1.5V 1.8V 1.0V 3.3V电压值
    sysmon.tempreture = gx21m15_temp_avg(); // 更新温度

    system_monitor_timer_start(); // 开启下一个周期
}
void system_monitor_timer_start(void)
{
    softtimer_start(&sysmon_timer, 1000, system_monitor_callback, &sysmon_timer);
}

/*
    @brief      : 软定时器初始化
*/
int softtimer_init(void)
{
    MultiTimerInstall(PlatformTicksGetFunc);

    /* 添加softtimet start */
    blink_timer_start();
    system_monitor_timer_start();

    return 0;
}

/*
    @brief      : 软定时器循环
*/
void softtimer_loop(void)
{
    int ret = 0;
    ret = MultiTimerYield();
}
