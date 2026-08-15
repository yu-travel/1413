
#include "gpio_app.h"


/*
*********************************************************************************************************************
    GPIO句柄定义
*********************************************************************************************************************
*/

DECLARED_GPIO_APP_SET(LED1);

DECLARED_GPIO_APP_GET(VPX_GA0);
DECLARED_GPIO_APP_GET(VPX_GA1);
DECLARED_GPIO_APP_GET(VPX_GA2);
DECLARED_GPIO_APP_GET(VPX_GA3);
DECLARED_GPIO_APP_GET(VPX_GA4);
DECLARED_GPIO_APP_GET(VPX_GAP);


/*
*********************************************************************************************************************
    GPIO句柄操作函数
*********************************************************************************************************************
*/

int gpio_setting(_gpio_app_t *handler, u8 status)
{
    int ret = 0;
    ret = handler->func(status);
    return ret;
}


int gpio_getting(_gpio_app_t *handler, u8 status)
{
    int ret = 0;
    ret = handler->func(status);
    return ret;
}


/*
*********************************************************************************************************************
    GPIO APP 
*********************************************************************************************************************
*/

/*
    @brief      : blink LED控制
    @param[in]  : none
    @param[out] : 
        addr        地址码缓存指针
    @retval     : none
*/
void blink_led_set(u8 status)
{
    /* LED1 LED2交叉闪烁 */
    gpio_setting(&app_LED1, status);
}


/*
    @brief      : 获取地址码；组成8bit( RA[3:0] | GA[3:0])
    @param[in]  : none
    @param[out] : 
        addr        地址码缓存指针
    @retval     : none
*/
void local_address_get(u8 *addr)
{
    u8 address = 0;
    #if 0
    address |= gpio_getting(&app_VPX_GAP, NULL);
    address |= gpio_getting(&app_VPX_GA4, NULL)<<1;
    address |= gpio_getting(&app_VPX_GA3, NULL)<<2;
    address |= gpio_getting(&app_VPX_GA2, NULL)<<3;
    address |= gpio_getting(&app_VPX_GA1, NULL)<<4;
    address |= gpio_getting(&app_VPX_GA0, NULL)<<5;
    #else
    address |= (GPIO_ReadInputDataBit(VPX_GAP_GPIO_Port, VPX_GAP_Pin)); 
    address |= (GPIO_ReadInputDataBit(VPX_GA4_GPIO_Port, VPX_GA4_Pin)<<1); 
    address |= (GPIO_ReadInputDataBit(VPX_GA3_GPIO_Port, VPX_GA3_Pin)<<2); 
    address |= (GPIO_ReadInputDataBit(VPX_GA2_GPIO_Port, VPX_GA2_Pin)<<3);
    address |= (GPIO_ReadInputDataBit(VPX_GA1_GPIO_Port, VPX_GA1_Pin)<<4);
    address |= (GPIO_ReadInputDataBit(VPX_GA0_GPIO_Port, VPX_GA0_Pin)<<5);
    #endif
    TRACE_OUT(DEBUG_OUT,"local address <%02x> \r\n", address);
    *addr = address;
}




