/*
***********************************************************************************************************************
    @brief          : 板载外设I2C通信底层接口
    @author         : xiongjinqi
    @date           : 2024/07/21
***********************************************************************************************************************
*/
#include "softtimer.h"
#include "main.h"
#include "gpio_app.h"
#include "myiic.h"
#include "periph_i2c.h"

#define I2C_CLOCK_FRQ 100000 // I2C-Frq in Hz (100 kHz)

DECLARED_GPIO_APP_SET(IPMI_I2C1_EN);
DECLARED_GPIO_APP_SET(IPMI_I2C2_EN);

DECLARED_GPIO_APP_GET(IPMI_I2C1_READY);
DECLARED_GPIO_APP_GET(IPMI_I2C2_READY);

_myiic_t gpio_i2c1 = {
    .sclport = IIC1_SCL_GPIO_Port,
    .sclpin = IIC1_SCL_Pin,
    .sdaport = IIC1_SDA_GPIO_Port,
    .sdapin = IIC1_SDA_Pin};

_myiic_t gpio_i2c2 = {
    .sclport = IIC2_SCL_GPIO_Port,
    .sclpin = IIC2_SCL_Pin,
    .sdaport = IIC2_SDA_GPIO_Port,
    .sdapin = IIC2_SDA_Pin};

/* 作为从机 */
_i2c_interrpt_t i2c1_int = {
    .dev = I2C1,
    .selfaddr = IPMI_PWR_I2CA_ID,
    .peeraddr = IPMI_MASTER_I2CA_ID,
    .status = I2C_RX_START,
};

/* 作为从机 */
_i2c_interrpt_t i2c2_int = {
    .dev = I2C2,
    .selfaddr = IPMI_PWR_I2CB_ID,
    .peeraddr = IPMI_MASTER_I2CB_ID,
    .status = I2C_RX_START,
};

/* 作为主机 */
_i2c_interrpt_t i2c3_int[] = {
    {
        .dev = I2C3,
        .selfaddr = IIC3_MASTER_ADDR,
        .peeraddr = GX21M15_DEV_ID1,
        .status = I2C_RX_START,
    },
    {
        .dev = I2C3,
        .selfaddr = IIC3_MASTER_ADDR,
        .peeraddr = GX21M15_DEV_ID2,
        .status = I2C_RX_START,
    },
    {
        .dev = I2C3,
        .selfaddr = 0x00,
        .peeraddr = AT24C02_DEV_ID,
        .status = I2C_RX_START,
    },
};
_i2c_interrpt_t *i2c3_int_ptr = NULL;

/*
    @brief      : 外设I2C1做slave初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
static void gpio_i2c1_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); // 使能GPIOF时钟

    GPIO_InitStructure.GPIO_Pin = IIC1_SCL_Pin | IIC1_SDA_Pin; // LED0和LED1对应IO口
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;              // 普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;             // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;         // 100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;               // 上拉
    GPIO_Init(IIC1_SCL_GPIO_Port, &GPIO_InitStructure);        // 初始化GPIO

    /* 设置GPIO默认状态 */
    GPIO_SetBits(IIC1_SCL_GPIO_Port, IIC1_SCL_Pin | IIC1_SDA_Pin); //
}

/*
    @brief      : 外设I2C1做slave初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
static void gpio_i2c2_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); // 使能GPIOF时钟

    GPIO_InitStructure.GPIO_Pin = IIC2_SCL_Pin | IIC2_SDA_Pin; // LED0和LED1对应IO口
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;              // 普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;             // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;         // 100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;               // 上拉
    GPIO_Init(IIC2_SCL_GPIO_Port, &GPIO_InitStructure);        // 初始化GPIO

    /* 设置GPIO默认状态 */
    GPIO_SetBits(IIC2_SCL_GPIO_Port, IIC2_SCL_Pin | IIC2_SDA_Pin); //
}

/*
    @brief      : 外设I2C1做slave初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
static void i2c1_slave_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;
#if 0
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOF时钟

    GPIO_InitStructure.GPIO_Pin     = IIC1_SCL_Pin | IIC1_SDA_Pin;//LED0和LED1对应IO口
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;       //普通输出模式
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;       //推挽输出
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;   //100MHz
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;        //上拉
    GPIO_Init(IIC1_SCL_GPIO_Port, &GPIO_InitStructure);                 //初始化GPIO

    /* 设置GPIO默认状态 */
    GPIO_SetBits(IIC1_SCL_GPIO_Port, IIC1_SCL_Pin|IIC1_SDA_Pin);     //
#else

    /*!< sEE_I2C Periph clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    /*!< sEE_I2C_SCL_GPIO_CLK and sEE_I2C_SDA_GPIO_CLK Periph clock enable */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* Reset sEE_I2C IP */
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE);
    /* Release reset signal of sEE_I2C IP */
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE);

    /*!< GPIO configuration */
    /*!< Configure sEE_I2C pins: SCL */
    GPIO_InitStructure.GPIO_Pin = IIC1_SCL_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(IIC1_SCL_GPIO_Port, &GPIO_InitStructure);

    /*!< Configure sEE_I2C pins: SDA */
    GPIO_InitStructure.GPIO_Pin = IIC1_SDA_Pin;
    GPIO_Init(IIC1_SDA_GPIO_Port, &GPIO_InitStructure);

    /* Connect PXx to I2C_SCL*/
    GPIO_PinAFConfig(IIC1_SCL_GPIO_Port, IIC1_SCL_SOURCE, IIC1_SCL_AF);
    /* Connect PXx to I2C_SDA*/
    GPIO_PinAFConfig(IIC1_SDA_GPIO_Port, IIC1_SDA_SOURCE, IIC1_SDA_AF);

    /* Configure the I2C event priority */
    NVIC_InitStructure.NVIC_IRQChannel = I2C1_EV_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Configure I2C error interrupt to have the higher priority */
    NVIC_InitStructure.NVIC_IRQChannel = I2C1_ER_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 7;
    NVIC_Init(&NVIC_InitStructure);

    /* 复位I2C模块 */
    I2C_SoftwareResetCmd(I2C1, ENABLE);
    I2C_SoftwareResetCmd(I2C1, DISABLE);

    /* I2C configuration */
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = i2c1_int.selfaddr << 1;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = I2C_CLOCK_FRQ;
    /* Apply I2C configuration after enabling it */
    I2C_Init(I2C1, &I2C_InitStructure);

    #if 1
    /* I2C Peripheral Enable */
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    I2C_Cmd(I2C1, ENABLE);

    I2C_ITConfig(I2C1, I2C_IT_EVT, ENABLE);
    I2C_ITConfig(I2C1, I2C_IT_BUF, ENABLE);
    I2C_ITConfig(I2C1, I2C_IT_ERR, ENABLE);
    #endif
#endif
}

/*
    @brief      : 外设I2C2做slave初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
static void i2c2_slave_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;

#if 0
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); // 使能GPIOF时钟

    GPIO_InitStructure.GPIO_Pin = IIC2_SCL_Pin | IIC2_SDA_Pin; // LED0和LED1对应IO口
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;              // 普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;             // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;         // 100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;               // 上拉
    GPIO_Init(IIC2_SCL_GPIO_Port, &GPIO_InitStructure);        // 初始化GPIO

    /* 设置GPIO默认状态 */
    GPIO_SetBits(IIC2_SCL_GPIO_Port, IIC2_SCL_Pin | IIC2_SDA_Pin); //
#else
    /*!< sEE_I2C Periph clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
    /*!< sEE_I2C_SCL_GPIO_CLK and sEE_I2C_SDA_GPIO_CLK Periph clock enable */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* Reset sEE_I2C IP */
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C2, ENABLE);
    /* Release reset signal of sEE_I2C IP */
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C2, DISABLE);

    /*!< GPIO configuration */
    /* Connect PXx to I2C_SCL*/
    GPIO_PinAFConfig(IIC2_SCL_GPIO_Port, IIC2_SCL_SOURCE, IIC2_SCL_AF);
    /* Connect PXx to I2C_SDA*/
    GPIO_PinAFConfig(IIC2_SDA_GPIO_Port, IIC2_SDA_SOURCE, IIC2_SDA_AF);

    /*!< Configure sEE_I2C pins: SCL */
    GPIO_InitStructure.GPIO_Pin = IIC2_SCL_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(IIC2_SCL_GPIO_Port, &GPIO_InitStructure);

    /*!< Configure sEE_I2C pins: SDA */
    GPIO_InitStructure.GPIO_Pin = IIC2_SDA_Pin;
    GPIO_Init(IIC2_SDA_GPIO_Port, &GPIO_InitStructure);

    /* Configure the I2C event priority */
    NVIC_InitStructure.NVIC_IRQChannel = I2C2_EV_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Configure I2C error interrupt to have the higher priority */
    NVIC_InitStructure.NVIC_IRQChannel = I2C2_ER_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 7;
    NVIC_Init(&NVIC_InitStructure);

    /* 复位I2C模块 */
    I2C_SoftwareResetCmd(I2C2, ENABLE);
    I2C_SoftwareResetCmd(I2C2, DISABLE);

    /* I2C configuration */
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = i2c2_int.selfaddr << 1;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = I2C_CLOCK_FRQ;
    /* Apply I2C configuration after enabling it */
    I2C_Init(I2C2, &I2C_InitStructure);

    #if 1
    /* I2C Peripheral Enable */
    I2C_AcknowledgeConfig(I2C2, ENABLE);
    I2C_Cmd(I2C2, ENABLE);

    I2C_ITConfig(I2C2, I2C_IT_EVT, ENABLE);
    I2C_ITConfig(I2C2, I2C_IT_BUF, ENABLE);
    I2C_ITConfig(I2C2, I2C_IT_ERR, ENABLE);
    #endif
#endif
}

/*
    @brief      : 外设I2C3做master初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
static void i2c3_master_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    // NVIC_InitTypeDef NVIC_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;

    /*!< sEE_I2C Periph clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C3, ENABLE);

    /*!< sEE_I2C_SCL_GPIO_CLK and sEE_I2C_SDA_GPIO_CLK Periph clock enable */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* Reset sEE_I2C IP */
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C3, ENABLE);
    /* Release reset signal of sEE_I2C IP */
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C3, DISABLE);

    /*!< GPIO configuration */
    /* Connect PXx to I2C_SCL I2C_SDA*/
    GPIO_PinAFConfig(IIC3_SCL_GPIO_Port, IIC3_SCL_SOURCE, IIC3_SCL_AF);
    GPIO_PinAFConfig(IIC3_SDA_GPIO_Port, IIC3_SDA_SOURCE, IIC3_SDA_AF);

    /*!< Configure sEE_I2C pins: SCL */
    GPIO_InitStructure.GPIO_Pin = IIC3_SCL_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(IIC3_SCL_GPIO_Port, &GPIO_InitStructure);

    /*!< Configure sEE_I2C pins: SDA */
    GPIO_InitStructure.GPIO_Pin = IIC3_SDA_Pin;
    GPIO_Init(IIC3_SDA_GPIO_Port, &GPIO_InitStructure);

    /* 复位I2C模块 */
    I2C_SoftwareResetCmd(I2C3, ENABLE);
    I2C_SoftwareResetCmd(I2C3, DISABLE);

    /* I2C configuration */
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = IIC3_MASTER_ADDR << 1;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = I2C_CLOCK_FRQ;
    /* Apply I2C configuration after enabling it */
    I2C_Init(I2C3, &I2C_InitStructure);
    I2C_AcknowledgeConfig(I2C3, ENABLE);
    /* I2C Peripheral Enable */
    I2C_Cmd(I2C3, ENABLE);
#if 0
    /* Configure the I2C event priority */
    NVIC_InitStructure.NVIC_IRQChannel                   = I2C3_EV_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Configure I2C error interrupt to have the higher priority */
    NVIC_InitStructure.NVIC_IRQChannel = I2C3_ER_IRQn;
    NVIC_Init(&NVIC_InitStructure);
    
    I2C_ITConfig(I2C3, I2C_IT_EVT, ENABLE); //Part of the STM32 I2C driver
    I2C_ITConfig(I2C3, I2C_IT_BUF, ENABLE);
    I2C_ITConfig(I2C3, I2C_IT_ERR, ENABLE); //Part of the STM32 I2C driver
#endif
}

/*
    @brief      : 外设I2C初始化、ringbuffer初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void i2c_all_init(void)
{
    /* 设置EN、读取Ready */
    gpio_setting(&app_IPMI_I2C1_EN, 1);
    gpio_setting(&app_IPMI_I2C2_EN, 1);

    /* 初始化i2c外设 */
    i2c1_slave_init(); // I2C1作为主机,与接口扩展板通信,用于测试
    i2c2_slave_init();
    // i2c3_master_init();

    u8_ring_buffer_init(&i2c1_int.rb_handler, (char *)i2c1_int.rxbuff, IIC_DATA_SIZE);
    u8_ring_buffer_init(&i2c2_int.rb_handler, (char *)i2c2_int.rxbuff, IIC_DATA_SIZE);

    if (gpio_getting(&app_IPMI_I2C1_READY, NULL))
    {
        TRACE_OUT(DEBUG_OUT, "I2C1 buffer ready OK \r\n");
    }

    if (gpio_getting(&app_IPMI_I2C2_READY, NULL))
    {
        TRACE_OUT(DEBUG_OUT, "I2C2 buffer ready OK \r\n");
    }
}

/*
    @brief      : 外设I2C发送数据
    @param[in]  :
        handler     i2c外设句柄
        slaveaddr   从机地址
        buff        发送数据缓存首地址
        len         发送数据的长度,长度最好控制在256字节以内
        mode        当前I2C模式
                        I2C_MODE_MASTER
                        I2C_MODE_SLAVE
    @param[out] : none
    @retval     : none
*/
u32 i2c_master_send_bytes(_i2c_interrpt_t *handler, u8 slaveaddr, u8 *buff, u16 len)
{
    u32 i2ctimeout = 0;
    int ret = 0;
#if (IPMI_IIC_TX_INTERRUPT != 1) // 不使用中断方式

    I2C_ITConfig(handler->dev, I2C_IT_EVT, DISABLE); // Part of the STM32 I2C driver
    I2C_ITConfig(handler->dev, I2C_IT_BUF, DISABLE);
    I2C_ITConfig(handler->dev, I2C_IT_ERR, DISABLE); // Part of the STM32 I2C driver

    i2ctimeout = I2CT_LONG_TIMEOUT;
    while (I2C_GetFlagStatus(handler->dev, I2C_FLAG_BUSY))
    {
        if ((i2ctimeout--) == 0)
        {
            ret = 4;
            goto EXIT_LABLE;
        }
    }
    /* Send START condition */
    I2C_GenerateSTART(handler->dev, ENABLE);

    i2ctimeout = I2CT_FLAG_TIMEOUT;
    /* Test on EV5 and clear it */
    while (!I2C_CheckEvent(handler->dev, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if ((i2ctimeout--) == 0)
        {
            ret = 5;
            goto EXIT_LABLE;
        }
    }

    /* Send EEPROM address for write */
    I2C_Send7bitAddress(handler->dev, slaveaddr << 1, I2C_Direction_Transmitter);

    i2ctimeout = I2CT_FLAG_TIMEOUT;
    /* Test on EV6 and clear it */
    while (!I2C_CheckEvent(handler->dev, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if ((i2ctimeout--) == 0)
        {
            ret = 6;
            goto EXIT_LABLE;
        }
    }
    /* While there is data to be written */
    while (len--)
    {
        /* Send the current byte */
        I2C_SendData(handler->dev, *buff);

        /* Point to the next byte to be written */
        buff++;

        i2ctimeout = I2CT_FLAG_TIMEOUT;
        /* Test on EV8 and clear it */
        while (!I2C_CheckEvent(handler->dev, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        {
            if ((i2ctimeout--) == 0)
            {
                ret = 8;
                goto EXIT_LABLE;
            }
        }
    }
    /* Send STOP condition */
    I2C_GenerateSTOP(handler->dev, ENABLE);

    I2C_ITConfig(handler->dev, I2C_IT_EVT, ENABLE); // Part of the STM32 I2C driver
    I2C_ITConfig(handler->dev, I2C_IT_BUF, ENABLE);
    I2C_ITConfig(handler->dev, I2C_IT_ERR, ENABLE); // Part of the STM32 I2C driver

EXIT_LABLE:
    if (ret != 0)
        I2C_TIMEOUT_UserCallback(ret);

    /* 重新初始化为slave */
    if (handler->dev == I2C1)
        i2c1_slave_init();
    if (handler->dev == I2C2)
        i2c2_slave_init();

    return 0;
#else

    handler->txcount = len;
    handler->peeraddr = slaveaddr << 1; // 用于中断中发送从机地址

    memset(handler->txbuff, 0, len);
    memcpy(handler->txbuff, buff, len); // 目前只接收256字节以内的数据

    i2ctimeout = I2CT_LONG_TIMEOUT;
    while (I2C_GetFlagStatus(handler->dev, I2C_FLAG_BUSY))
    {
        if ((i2ctimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(4);
    }
    /* Send START condition */
    I2C_GenerateSTART(handler->dev, ENABLE);

    return 0;
#endif
}

/*
    @brief      : 外设I2C从机发送数据
    @param[in]  :
        handler     i2c外设句柄
        slaveaddr   从机地址
        buff        发送数据缓存首地址
        len         发送数据的长度,长度最好控制在256字节以内
        mode        当前I2C模式
                        I2C_MODE_MASTER
                        I2C_MODE_SLAVE
    @param[out] : none
    @retval     : none
*/
void i2c_slave_send_bytes(_i2c_interrpt_t *handler, u8 slaveaddr, u8 *buff, u16 len)
{
    handler->txcount = len;
    handler->peeraddr = slaveaddr << 1; // 用于中断中发送从机地址

    memset(handler->txbuff, 0, len);
    memcpy(handler->txbuff, buff, len); // 目前只接收256字节以内的数据
}

/*
    @brief      : gpio模拟I2C写数据
    @param[in]  :
        handler         i2c总线句柄
        slaveaddr       从机地址（0-0x7F）
        buff            发送数据缓冲区
        len             发送的数据长度
    @param[out] : none
    @retval     : 读到的数据
*/
uint8_t gpioi2c_send_bytes(_i2c_interrpt_t *handler, u8 slaveaddr, u8 *buff, u16 len)
{
    uint8_t temp = 0;
    _myiic_t *i2cbus = NULL;

    if (handler->dev == I2C1)
    {
        gpio_i2c1_init(); // 初始化为GPIO 开漏输出
        i2cbus = &gpio_i2c1;
    }
    if (handler->dev == I2C2)
    {
        gpio_i2c2_init();
        i2cbus = &gpio_i2c2;
    }

    iic_start(i2cbus);                     /* 发送起始信号 */
    iic_send_byte(i2cbus, slaveaddr << 1); /* 发送设备地址 */
    iic_wait_ack(i2cbus);                  /* 每次发送完一个字节,都要等待ACK */
    for (int i = 0; i < len; i++)
    {
        iic_send_byte(i2cbus, *buff++); /* 发送低位地址 */
        iic_wait_ack(i2cbus);           /* 等待ACK, 此时地址发送完成了 */
    }
    iic_stop(i2cbus); /* 产生一个停止条件 */

    return temp;
}

/*!
    \brief      handle i2c event interrupt request
    @param[in]  :
        handler         i2c中断句柄
    @param[out] : none
    @retval     : none
*/
static void i2c_event_irq_handler(_i2c_interrpt_t *handler)
{
    u8 data = 0;
    u32 event = 0;
#if 1
    event = I2C_GetLastEvent(handler->dev);

    switch (event)
    {
    /*--------------- master transmit ---------------------*/
    /* 1、产生起始信号 */
    case I2C_EVENT_MASTER_MODE_SELECT:
        // TRACE_OUT(DEBUG_OUT, "master gen START\r\n");
        I2C_Send7bitAddress(handler->dev, handler->peeraddr, I2C_Direction_Transmitter);
        // handler->status = I2C_TX_ING;
        break;
    /* 2、发送从机地址完成 */
    case I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED:
        // TRACE_OUT(DEBUG_OUT, "address has sended\r\n");
        I2C_SendData(handler->dev, handler->txbuff[handler->txindex++]); // 向DR写操作清楚TxE标志
        handler->txcount--;
        break;

    /* 3、发送数据 */
    case I2C_EVENT_MASTER_BYTE_TRANSMITTING:
        if (handler->txcount > 0)
        {
            I2C_SendData(handler->dev, handler->txbuff[handler->txindex++]);
            handler->txcount--;
        }
        else
        {
            I2C_GenerateSTOP(handler->dev, ENABLE); // 产生stop
            handler->txindex = 0;
            handler->status = I2C_TX_DONE;
        }
        break;
    case I2C_EVENT_MASTER_BYTE_TRANSMITTED:
        if (handler->txcount > 0)
        {
            I2C_SendData(handler->dev, handler->txbuff[handler->txindex++]);
            handler->txcount--;
        }
        else
        {
            I2C_GenerateSTOP(handler->dev, ENABLE); // 产生stop
            handler->txindex = 0;
            handler->status = I2C_TX_DONE;
        }
        break;

    /*--------------- master recieve(as slave) ---------------------*/
    /* 1、slave 模式,从机地址已经匹配,EV1 */
    case I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED:
        // TRACE_OUT(DEBUG_OUT, "slave addr matched\r\n");
        break;
    /* 2、已接收一字节数据 */
    case I2C_EVENT_MASTER_BYTE_RECEIVED:
        handler->status = I2C_RX_ING;
        data = I2C_ReceiveData(handler->dev);
        u8_ring_buffer_queue(&handler->rb_handler, data); // 压入ringbuffer
        handler->rxindex++;
        if (handler->rxindex == 2) // 获取总长度
        {
            handler->rxcount = data;
        }
        if (handler->rxindex == handler->rxcount - 1)
        {
            I2C_AcknowledgeConfig(handler->dev, DISABLE); // 最后一字节NACK
        }
        if (handler->rxindex == handler->rxcount)
        {
            I2C_GenerateSTOP(handler->dev, ENABLE); // 产生STOP
            handler->rxindex = 0;
            handler->status = I2C_RX_DONE;
        }
        break;

    /*--------------- slave transmit(as master) ---------------------*/
    /* 2、发送从机地址完成 */
    case I2C_EVENT_SLAVE_TRANSMITTER_ADDRESS_MATCHED:
        // TRACE_OUT(DEBUG_OUT, "address has sended\r\n");
        // I2C_SendData(handler->dev, handler->txbuff[handler->txindex++]);    // 开始发送第一个BYTE
        // handler->txcount--;
        break;

    /* 3、发送数据 */
    case I2C_EVENT_SLAVE_BYTE_TRANSMITTING:
        handler->status = I2C_TX_ING;
        if (handler->txcount > 0)
        {
            I2C_SendData(handler->dev, handler->txbuff[handler->txindex++]);
            handler->txcount--;
        }
        break;
    case I2C_EVENT_SLAVE_BYTE_TRANSMITTED:
        if (handler->txcount > 0)
        {
            I2C_SendData(handler->dev, handler->txbuff[handler->txindex++]);
            handler->txcount--;
        }
        break;
    case I2C_EVENT_SLAVE_ACK_FAILURE: // 最后一个字节是NACK
        handler->txindex = 0;
        handler->status = I2C_TX_DONE;
        I2C_ClearITPendingBit(handler->dev, I2C_IT_AF); // 发送器STOP，清AF位
        break;

    /*--------------- slave recieve ---------------------*/
    /* 1、slave 模式,从机地址已经匹配,EV1 */
    case I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED:
        // TRACE_OUT(DEBUG_OUT, "slave addr matched\r\n");
        break;
    /* 2、已接收一字节数据 */
    case I2C_EVENT_SLAVE_BYTE_RECEIVED:
        data = I2C_ReceiveData(handler->dev);
        u8_ring_buffer_queue(&handler->rb_handler, data); // 压入ringbuffer
        break;
    /* 3、检测到STOP */
    case I2C_EVENT_SLAVE_STOP_DETECTED:
        // TRACE_OUT(DEBUG_OUT, "STOP matched\r\n");
        handler->status = I2C_RX_DONE;
        I2C_Cmd(handler->dev, ENABLE); // 清STOPF状态
        break;

    default:
        TRACE_OUT(DEBUG_OUT, "I2C invaild EVENT[%08x]\r\n", event);
        break;
    }
#endif
}

/*!
    \brief      handle i2c event interrupt request
    @param[in]  :
        i2c         i2c外设
        peeraddr    对端的地址
    @param[out] : none
    @retval     : none

*/
void i2c_slave_event_irq_handler(_i2c_interrpt_t *handler)
{
    u8 data = 0;
    u32 event = 0;
#if 1
    event = I2C_GetLastEvent(handler->dev);
    switch (event)
    {
    /*--------------- slave transmit(as master) ---------------------*/
    /* 2、发送从机地址完成 */
    case I2C_EVENT_SLAVE_TRANSMITTER_ADDRESS_MATCHED:
        // TRACE_OUT(DEBUG_OUT, "address has sended\r\n");
        // I2C_SendData(handler->dev, handler->txbuff[handler->txindex++]);    // 开始发送第一个BYTE
        // handler->txcount--;
        break;

    /* 3、发送数据 */
    case I2C_EVENT_SLAVE_BYTE_TRANSMITTING:
        handler->status = I2C_TX_ING;
        I2C_SendData(handler->dev, handler->txbuff[handler->txindex++]); // 开始发送第一个BYTE
        handler->txcount--;
        break;
    case I2C_EVENT_SLAVE_BYTE_TRANSMITTED:
        if (handler->txcount > 0)
        {
            I2C_SendData(handler->dev, handler->txbuff[handler->txindex++]);
            handler->txcount--;
        }
        break;
    case I2C_EVENT_SLAVE_ACK_FAILURE: // 最后一个字节是NACK
        handler->txindex = 0;
        handler->status = I2C_TX_DONE;
        I2C_ClearITPendingBit(handler->dev, I2C_IT_AF); // 发送器STOP，清AF位
        break;

    /*--------------- slave recieve ---------------------*/
    /* 1、slave 模式,从机地址已经匹配,EV1 */
    case I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED:
        // TRACE_OUT(DEBUG_OUT, "slave addr matched\r\n");
        break;
    /* 2、已接收一字节数据 */
    case I2C_EVENT_SLAVE_BYTE_RECEIVED:
        data = I2C_ReceiveData(handler->dev);
        u8_ring_buffer_queue(&handler->rb_handler, data); // 压入ringbuffer
        break;
    /* 3、检测到STOP */
    case I2C_EVENT_SLAVE_STOP_DETECTED:
        // TRACE_OUT(DEBUG_OUT, "STOP matched\r\n");
        handler->status = I2C_RX_DONE;
        I2C_Cmd(handler->dev, ENABLE); // 清STOPF状态
        break;

    default:
        TRACE_OUT(DEBUG_OUT, "Line%d: I2C invaild EVENT[%08x]\r\n", __LINE__, event);
        break;
    }
#endif
}

/*
    I2C1事件中断服务函数
*/
void I2C1_EV_IRQHandler(void)
{
    // i2c_master_event_irq_handler(&i2c1_int);
    i2c_event_irq_handler(&i2c1_int);
}

/*
    I2C1错误中断服务函数
*/
void I2C1_ER_IRQHandler(void)
{
    /* Check on I2C1 AF flag and clear it */
    if (I2C_GetITStatus(I2C1, I2C_IT_AF))
    {
        I2C_ClearITPendingBit(I2C1, I2C_IT_AF);
    }

    /* Check on I2C1 AF flag and clear it */
    if (I2C_GetITStatus(I2C1, I2C_IT_BERR)) // 这个就是起始和停止条件出错了
    {
        I2C_ClearITPendingBit(I2C1, I2C_IT_BERR);
    }
}

/*
    I2C1事件中断服务函数
*/
void I2C2_EV_IRQHandler(void)
{
    i2c_event_irq_handler(&i2c2_int);
}

/*
    I2C1错误中断服务函数
*/
void I2C2_ER_IRQHandler(void)
{
    /* Check on I2C1 AF flag and clear it */
    if (I2C_GetITStatus(I2C1, I2C_IT_AF))
    {
        I2C_ClearITPendingBit(I2C1, I2C_IT_AF);
    }

    /* Check on I2C1 AF flag and clear it */
    if (I2C_GetITStatus(I2C1, I2C_IT_BERR)) // 这个就是起始和停止条件出错了
    {
        I2C_ClearITPendingBit(I2C1, I2C_IT_BERR);
    }
}

/**
 * @brief  Basic management of the timeout situation.
 * @param  errorCode：错误代码，可以用来定位是哪个环节出错.
 * @retval 返回0，表示IIC读取失败.
 */
uint32_t I2C_TIMEOUT_UserCallback(uint8_t errorCode)
{
    /* Block communication and all processes */
    TRACE_OUT(DEBUG_OUT, "ERROR: I2C wait timeout, errorCode = %d\r\n", errorCode);
    return 0;
}

#if TEST_ENABLE

_time_t i2c_test_timer;

void i2c_test_callback(_time_t *timer, void *userData)
{
    _i2c_interrpt_t *i2cptr = (_i2c_interrpt_t *)userData;

    TRACE_OUT(DEBUG_OUT, "TX ================================\r\n");
#if 0
    for(int i= 0; i<256; i++)
    {
        TRACE_OUT(DEBUG_OUT, "%02x ", txbuff[i]);
        if(i%16 == 15)
            TRACE_OUT(DEBUG_OUT, "\r\n");
    }
#endif
    i2c_master_send_bytes(i2cptr, IPMI_MASTER_I2CA_ID, "87654321", 8);
    softtimer_start(&i2c_test_timer, 1000, i2c_test_callback, i2cptr); // 开启下一个循环
}

void periph_i2c_test(void)
{
    static u8 test_flag = 0;

    if (test_flag == 0)
    {
        test_flag++;
        softtimer_start(&i2c_test_timer, 1000, i2c_test_callback, &i2c1_int);
    }
}

#endif
