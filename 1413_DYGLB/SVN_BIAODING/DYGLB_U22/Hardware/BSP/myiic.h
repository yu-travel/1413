/**
 ****************************************************************************************************
 * @file        myiic.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2021-10-23
 * @brief       IIC 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 探索者 F407开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20211023
 * 第一次发布
 *
 ****************************************************************************************************
 */
 
#ifndef __MYIIC_H
#define __MYIIC_H

#include "sys.h"


/******************************************************************************************/
/* 引脚 定义 */
#if 0
#define IIC_SCL_GPIO_PORT               GPIOB
#define IIC_SCL_GPIO_PIN                GPIO_PIN_8
#define IIC_SCL_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)   /* PB口时钟使能 */

#define IIC_SDA_GPIO_PORT               GPIOB
#define IIC_SDA_GPIO_PIN                GPIO_PIN_9
#define IIC_SDA_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)   /* PB口时钟使能 */

#define IIC3_SCL_GPIO_PORT               GPIOA
#define IIC3_SCL_GPIO_PIN                GPIO_PIN_8
#define IIC3_SCL_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)   /* PA口时钟使能 */

#define IIC3_SDA_GPIO_PORT               GPIOC
#define IIC3_SDA_GPIO_PIN                GPIO_PIN_9
#define IIC3_SDA_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)   /* PC口时钟使能 */
#endif


/******************************************************************************************/
typedef struct {
    GPIO_TypeDef    *sclport;
    GPIO_TypeDef    *sdaport;
    u16             sclpin;
    u16             sdapin;
}_myiic_t;

/* IO操作 */
#define IIC_SCL(myiic_ptr,x)        do{ x ? \
                              GPIO_WriteBit(myiic_ptr->sclport, myiic_ptr->sclpin, Bit_SET) : \
                              GPIO_WriteBit(myiic_ptr->sclport, myiic_ptr->sclpin, Bit_RESET); \
                          }while(0)       /* SCL */

#define IIC_SDA(myiic_ptr,x)        do{ x ? \
                              GPIO_WriteBit(myiic_ptr->sdaport, myiic_ptr->sdapin, Bit_SET) : \
                              GPIO_WriteBit(myiic_ptr->sdaport, myiic_ptr->sdapin, Bit_RESET); \
                          }while(0)       /* SDA */

#define IIC_READ_SDA(myiic_ptr)     GPIO_ReadInputDataBit(myiic_ptr->sdaport, myiic_ptr->sdapin) /* 读取SDA */



/* IIC所有操作函数 */
/**
 * @brief       产生IIC起始信号
 * @param       无
 * @retval      无
 */
void iic_start(_myiic_t *i2c);
/**
 * @brief       产生IIC停止信号
 * @param       无
 * @retval      无
 */
void iic_stop(_myiic_t *i2c);
/**
 * @brief       等待应答信号到来
 * @param       无
 * @retval      1，接收应答失败
 *              0，接收应答成功
 */
uint8_t iic_wait_ack(_myiic_t *i2c);

/**
 * @brief       产生ACK应答
 * @param       无
 * @retval      无
 */
void iic_ack(_myiic_t *i2c);
/**
 * @brief       不产生ACK应答
 * @param       无
 * @retval      无
 */
void iic_nack(_myiic_t *i2c);

/**
 * @brief       IIC发送一个字节
 * @param       data: 要发送的数据
 * @retval      无
 */
void iic_send_byte(_myiic_t *i2c,uint8_t data);

/**
 * @brief       IIC读取一个字节
 * @param       ack:  ack=1时，发送ack; ack=0时，发送nack
 * @retval      接收到的数据
 */
uint8_t iic_read_byte(_myiic_t *i2c, uint8_t ack);


#endif

