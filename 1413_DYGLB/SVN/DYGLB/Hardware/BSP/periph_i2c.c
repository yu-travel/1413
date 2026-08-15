/*
***********************************************************************************************************************
    @brief          : 板载外设I2C通信底层接口
    @author         : xiongjinqi
    @date           : 2024/07/21
***********************************************************************************************************************
*/
#include "softtimer.h"
#include "main.h"
#include "periph_i2c.h"

#define I2C_CLOCK_FRQ          100000     // I2C-Frq in Hz (100 kHz)

//DECLARED_GPIO_APP_SET(IPMI_I2C1_EN);
//DECLARED_GPIO_APP_SET(IPMI_I2C2_EN);

//DECLARED_GPIO_APP_GET(IPMI_I2C1_READY);
//DECLARED_GPIO_APP_GET(IPMI_I2C2_READY);

_myiic_t gpio_i2c1 = {
    .sclport = IIC1_SCL_GPIO_Port,
    .sclpin  = IIC1_SCL_Pin,
    .sdaport = IIC1_SDA_GPIO_Port,
    .sdapin  = IIC1_SDA_Pin
};


_myiic_t gpio_i2c2 = {
    .sclport = IIC2_SCL_GPIO_Port,
    .sclpin  = IIC2_SCL_Pin,
    .sdaport = IIC2_SDA_GPIO_Port,
    .sdapin  = IIC2_SDA_Pin
};



/* 作为从机 */
_i2c_interrpt_t i2c1_int = {
    .dev = I2C1,
    .selfaddr = 0x00,
    .peeraddr = AT24C02_DEV_ID,
    .status   = I2C_RX_START,
};

/* 作为从机 */
_i2c_interrpt_t i2c2_int = {
    .dev = I2C2,
//    .selfaddr = IPMI_MASTER_I2CB_ID,
    .status   = I2C_RX_START,
};

/* 作为主机 */
_i2c_interrpt_t i2c3_int[] = {
    {
        .dev = I2C3,
        .selfaddr = 0x00,
        .peeraddr = AT24C02_DEV_ID,
        .status   = I2C_RX_START,
    },
};
_i2c_interrpt_t *i2c3_int_ptr = NULL;



/*
    @brief      : 外设I2C1做slave初始化 lys:本接口做从
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void i2c1_master_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef  I2C_InitStructure;
    /*!< sEE_I2C Periph clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    /*!< sEE_I2C_SCL_GPIO_CLK and sEE_I2C_SDA_GPIO_CLK Periph clock enable */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* Reset sEE_I2C IP */
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE);
    /* Release reset signal of sEE_I2C IP */
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE);

    /* 复位I2C模块 */
    I2C_SoftwareResetCmd(I2C1, ENABLE);
    I2C_SoftwareResetCmd(I2C1, DISABLE);
    
    /*!< GPIO configuration */
    /* Connect PXx to I2C_SCL I2C_SDA*/
    GPIO_PinAFConfig(IIC1_SCL_GPIO_Port, IIC1_SCL_SOURCE, IIC1_SCL_AF);
    GPIO_PinAFConfig(IIC1_SDA_GPIO_Port, IIC1_SDA_SOURCE, IIC1_SDA_AF);

    /*!< Configure sEE_I2C pins: SCL */   
    GPIO_InitStructure.GPIO_Pin = IIC1_SCL_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP; // GPIO_PuPd_NOPULL
    GPIO_Init(IIC1_SCL_GPIO_Port, &GPIO_InitStructure);

    /*!< Configure sEE_I2C pins: SDA */
    GPIO_InitStructure.GPIO_Pin = IIC1_SDA_Pin;
    GPIO_Init(IIC1_SDA_GPIO_Port, &GPIO_InitStructure);
    
    /* I2C configuration */
    I2C_InitStructure.I2C_Mode          = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle     = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1   = i2c1_int.selfaddr;
    I2C_InitStructure.I2C_Ack           = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed    = I2C_CLOCK_FRQ;
    /* Apply I2C configuration after enabling it */
    I2C_Init(I2C1, &I2C_InitStructure);
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    I2C_Cmd(I2C1, ENABLE);

}

/*
    @brief      : 外设I2C初始化、ringbuffer初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void i2c_all_init(void)
{
    i2c1_master_init(); 
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
u32 i2c_master_send_bytes(_i2c_interrpt_t* handler, u8 slaveaddr, u8 *buff, u16 len)
{
    u32 i2ctimeout = 0;
    int ret = 0;
#if (IPMI_IIC_TX_INTERRUPT != 1)    // 不使用中断方式
    
    I2C_ITConfig(handler->dev, I2C_IT_EVT, DISABLE); //Part of the STM32 I2C driver
    I2C_ITConfig(handler->dev, I2C_IT_BUF, DISABLE);
    I2C_ITConfig(handler->dev, I2C_IT_ERR, DISABLE); //Part of the STM32 I2C driver
    
    i2ctimeout = I2CT_LONG_TIMEOUT;
    while(I2C_GetFlagStatus(handler->dev, I2C_FLAG_BUSY))
    {
        if((i2ctimeout--) == 0)
        {
            ret = 4;
            goto EXIT_LABLE;
        }
    }
    /* Send START condition */
    I2C_GenerateSTART(handler->dev, ENABLE);

    i2ctimeout = I2CT_FLAG_TIMEOUT;
    /* Test on EV5 and clear it */
    while(!I2C_CheckEvent(handler->dev, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if((i2ctimeout--) == 0)
        {
            ret = 5;
            goto EXIT_LABLE;
        }
    } 

    /* Send EEPROM address for write */
    I2C_Send7bitAddress(handler->dev, slaveaddr<<1, I2C_Direction_Transmitter);

    i2ctimeout = I2CT_FLAG_TIMEOUT;
    /* Test on EV6 and clear it */
    while(!I2C_CheckEvent(handler->dev, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if((i2ctimeout--) == 0)
        {
            ret = 6;
            goto EXIT_LABLE;
        }
    } 
    /* While there is data to be written */
    while(len--)
    {
        /* Send the current byte */
        I2C_SendData(handler->dev, *buff);

        /* Point to the next byte to be written */
        buff++;

        i2ctimeout = I2CT_FLAG_TIMEOUT;
        /* Test on EV8 and clear it */
        while (!I2C_CheckEvent(handler->dev, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        {
            if((i2ctimeout--) == 0)
            {
                ret = 8;
                goto EXIT_LABLE;
            }
        }
    }
    /* Send STOP condition */
    I2C_GenerateSTOP(handler->dev, ENABLE);
    
    I2C_ITConfig(handler->dev, I2C_IT_EVT, ENABLE); //Part of the STM32 I2C driver
    I2C_ITConfig(handler->dev, I2C_IT_BUF, ENABLE);
    I2C_ITConfig(handler->dev, I2C_IT_ERR, ENABLE); //Part of the STM32 I2C driver

EXIT_LABLE:
    if(ret != 0)
        I2C_TIMEOUT_UserCallback(ret);
    
    /* 重新初始化为slave */
//    if(handler->dev == I2C1)
//        i2c1_master_init();
//    if(handler->dev == I2C2)
//        i2c2_master_init();

    return 0;
#else

    handler->txcount = len;
    handler->peeraddr = slaveaddr<<1;  // 用于中断中发送从机地址

    memset(handler->txbuff, 0, len);
    memcpy(handler->txbuff, buff, len);     // 目前只接收256字节以内的数据
    
    i2ctimeout = I2CT_LONG_TIMEOUT;
    while(I2C_GetFlagStatus(handler->dev, I2C_FLAG_BUSY))
    {
        if((i2ctimeout--) == 0) return I2C_TIMEOUT_UserCallback(4);
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
void i2c_slave_send_bytes(_i2c_interrpt_t* handler, u8 slaveaddr, u8 *buff, u16 len)
{
    handler->txcount = len%IIC_DATA_SIZE;
    handler->peeraddr = slaveaddr<<1;  // 用于中断中发送从机地址

    memset(handler->txbuff, 0, IIC_DATA_SIZE);
    memcpy(handler->txbuff, buff, len%256);     // 目前只接收256字节以内的数据
}


/******************************************************************************
 * 函 数 名：clear_I2C_recvie
 *
 * 函数说明: 清空i2c的数据接收缓冲区
 * 参数说明: 
 * 输入参数: 
 * 输出参数: 
 * 返 回 值: 
 * 
 * 修改记录：
 * --------------------
 *	 
 ******************************************************************************/
int clear_I2C_recvie(_i2c_interrpt_t *handler)
{	
	unsigned int timeOut=100000;
    while(I2C_GetFlagStatus(handler->dev, I2C_FLAG_BUSY))
    {
        if((timeOut--) == 0) return I2C_TIMEOUT_UserCallback(99);
    }
	while(I2C_GetFlagStatus(handler->dev, I2C_FLAG_RXNE))
    {
        I2C_ReceiveData(handler->dev);
    }
	u8_ring_buffer_init(&i2c1_int.rb_handler, (char *)i2c1_int.rxbuff, IIC_DATA_SIZE);
	u8_ring_buffer_init(&i2c2_int.rb_handler, (char *)i2c2_int.rxbuff, IIC_DATA_SIZE);
	return 0;
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
    return errorCode;
}


#if TEST_ENABLE



#endif


