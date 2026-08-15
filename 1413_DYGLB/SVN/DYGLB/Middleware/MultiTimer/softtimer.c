/*
**************************************************************************************************************
    @brief      : Software timer interface and loop processing
    @author     : xiongjiqi
    @date       : 24/07/01
    @note       : 1. RTT heartbeat moved to App layer (app_main.c
                   task_heartbeat_cb), watchdog is fed by main loop;
                   legacy blink timer removed 2026-08 (Task12 integration)
**************************************************************************************************************
*/
#include "bsp_timer.h"
#include "softtimer.h"


/* Platform tick source: TIM2 1kHz ms counter (bsp_timer_getms_count) */
uint64_t PlatformTicksGetFunc(void)
{
    return bsp_timer_getms_count();
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
    @brief      : Software timer init (install tick source only)
    @note       : Periodic tasks are registered by app_tasks_init()
                  in App layer after softtimer_init()
*/
int softtimer_init(void)
{
    MultiTimerInstall(PlatformTicksGetFunc);

    return 0;
}

/*
    @brief      : Software timer loop (dispatch expired timer callbacks)
*/
void softtimer_loop(void)
{

     (void)MultiTimerYield();

}
