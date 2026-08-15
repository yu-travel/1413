/*
**************************************************************************************************************
    @brief      : 软定时器接口及软定时任务
    @author     : xiongjiqi
    @date       : 24/07/01
**************************************************************************************************************
*/
#include "timer.h"
#include "iwdg.h"
#include "softtimer.h"
#include "main.h"


_time_t blink_timer = {0};

/* 软定时事件声明 */
void blink_timer_start(void);


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
    @brief      : 看门狗喂狗和led闪烁
*/
void blinkled_callback(_time_t* timer, void* userData)
{
    static u32 blink_count = 0;

    blink_count++;
    if(blink_count%2 == 0)
        GPIO_SetBits(GPIOD,GPIO_Pin_2);
    else
        GPIO_ResetBits(GPIOD,GPIO_Pin_2);
        
    IWDG_Feed();    // 每隔500ms喂狗一次
    blink_timer_start();    // 开启下一个周期
}
void blink_timer_start(void)
{
    softtimer_start(&blink_timer, 500, blinkled_callback, &blink_timer);
}


/*
    @brief      : 软定时器初始化
*/
int softtimer_init(void)
{
    MultiTimerInstall(PlatformTicksGetFunc);

    /* 添加softtimet start */
	
    blink_timer_start();
    return 0;
}

/* 
    @brief      : 软定时器循环
*/
void softtimer_loop(void)
{

     (void)MultiTimerYield();

}




