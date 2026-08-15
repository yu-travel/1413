/*
***************************************************************************************************
    板载gpio应用
***************************************************************************************************
*/
#ifndef __GPIO_APP_H_
#define __GPIO_APP_H_

#include "types_def.h"
#include "gpio.h"

#define LED_ON              1
#define LED_OFF             0

typedef int(*gpio_func) (u8 status);

typedef struct 
{
    u8 status;
    gpio_func func;
}_gpio_app_t;


#define DECLARED_GPIO_APP_SET(name)     \
    int name##_opt(u8 status) {         \
        if(status)                      \
            GPIO_WriteBit(name##_GPIO_Port, name##_Pin, Bit_SET);\
        else                            \
            GPIO_WriteBit(name##_GPIO_Port, name##_Pin, Bit_RESET);  \
        return 0;                       \
    }                                   \
    _gpio_app_t app_##name = {          \
        .status = LED_OFF,              \
        .func   = name##_opt,           \
    }


#define DECLARED_GPIO_APP_GET(name)          \
    int name##_opt(u8 status) \
    {                               \
        return (GPIO_ReadInputDataBit(name##_GPIO_Port, name##_Pin)?1:0);  \
    }                               \
    _gpio_app_t app_##name = {      \
        .status = LED_OFF,          \
        .func   = name##_opt,   \
    }


/*
***************************************************************************************************
    函数声明
***************************************************************************************************
*/
int gpio_setting(_gpio_app_t *handler, u8 status);
int gpio_getting(_gpio_app_t *handler, u8 status);


/*
    @brief      : blink LED控制
    @param[in]  : none
    @param[out] : 
        addr        地址码缓存指针
    @retval     : none
*/
void blink_led_set(u8 status);

/*
    @brief      : 获取地址码；组成6bit( GAP+GA[4:0])
    @param[in]  : none
    @param[out] : 
        addr        地址码缓存指针
    @retval     : none
*/
void local_address_get(u8 *addr);


#endif /* __GPIO_APP_H_ */

