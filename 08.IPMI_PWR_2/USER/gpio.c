/*
***********************************************************************************************************************
    @brief          : 板载gpio初始化
    @author         : xiongjinqi
    @date           : 2024/07/21
***********************************************************************************************************************
*/
#include "gpio.h"

/*
    @brief      : led灯Gpio初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void led_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOF时钟

    GPIO_InitStructure.GPIO_Pin     = LED1_Pin;//LED0和LED1对应IO口
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;       //普通输出模式
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;       //推挽输出
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;   //100MHz
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;        //上拉
    GPIO_Init(LED1_GPIO_Port, &GPIO_InitStructure);                 //初始化GPIO

    /* 设置GPIO默认状态 */
    GPIO_SetBits(LED1_GPIO_Port, LED1_Pin);     //灯灭

}


/*
    @brief      : vpx_slotaddr_init
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void vpx_slotaddr_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOF时钟

    GPIO_InitStructure.GPIO_Pin     = VPX_GA0_Pin|VPX_GA1_Pin|VPX_GA2_Pin|VPX_GA3_Pin|VPX_GA4_Pin|VPX_GAP_Pin;//LED0和LED1对应IO口
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;       //普通输出模式
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;   //100MHz
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;        //上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);                 //初始化GPIO

    /* 设置GPIO默认状态 */

}


/*
    @brief      : i2c_en_ready_init
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void i2c_en_ready_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOF时钟

    GPIO_InitStructure.GPIO_Pin     = IPMI_I2C1_EN_Pin|IPMI_I2C2_EN_Pin;//LED0和LED1对应IO口
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;       //普通输出模式
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;       //推挽输出
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;   //100MHz
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;        //上拉
    GPIO_Init(IPMI_I2C1_EN_GPIO_Port, &GPIO_InitStructure);                 //初始化GPIO

    GPIO_InitStructure.GPIO_Pin     = IPMI_I2C1_READY_Pin|IPMI_I2C2_READY_Pin;//LED0和LED1对应IO口
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;       //普通输出模式
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;   //100MHz
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;        //上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);                 //初始化GPIO
    
    /* 设置GPIO默认状态 */
    GPIO_SetBits(IPMI_I2C1_EN_GPIO_Port, IPMI_I2C1_EN_Pin);     //
    GPIO_SetBits(IPMI_I2C2_EN_GPIO_Port, IPMI_I2C2_EN_Pin);     //

}

/*
    @brief      : sys_reset_init复位引脚初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void sys_reset_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOF时钟

    GPIO_InitStructure.GPIO_Pin     = SYS_RESET_Pin|NVMRO_Pin;//LED0和LED1对应IO口
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;       //普通输出模式
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;   //100MHz
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;        //上拉
    GPIO_Init(SYS_RESET_GPIO_Port, &GPIO_InitStructure);                 //初始化GPIO
}


/*
    @brief      : xc388_en_init
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void xc388_en_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOF时钟

    GPIO_InitStructure.GPIO_Pin     = XC388_EN_Pin;//LED0和LED1对应IO口
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;       //普通输出模式
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_OD;       //推挽输出
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;   //100MHz
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;        //上拉
    GPIO_Init(XC388_EN_GPIO_Port, &GPIO_InitStructure);                 //初始化GPIO

    /* 设置GPIO默认状态 */
    GPIO_ResetBits(XC388_EN_GPIO_Port, XC388_EN_Pin);     //
}




/*
    @brief      : xc4001_ctrl_init复位及过流检测初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void xc4001_ctrl_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOF时钟

    GPIO_InitStructure.GPIO_Pin     = XCA4001_RESET_Pin;//LED0和LED1对应IO口
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;       //普通输出模式
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_OD;       //推挽输出
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;   //100MHz
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;        //上拉
    GPIO_Init(XCA4001_RESET_GPIO_Port, &GPIO_InitStructure);                 //初始化GPIO

    GPIO_InitStructure.GPIO_Pin     = XCA4001_Alert_Pin;//LED0和LED1对应IO口
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;       //普通输出模式
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;   //100MHz
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;        //上拉
    GPIO_Init(XCA4001_Alert_GPIO_Port, &GPIO_InitStructure);                 //初始化GPIO

    /* 设置GPIO默认状态 */
    GPIO_ResetBits(XCA4001_RESET_GPIO_Port, XCA4001_RESET_Pin);     //设置为自恢复模式
}



