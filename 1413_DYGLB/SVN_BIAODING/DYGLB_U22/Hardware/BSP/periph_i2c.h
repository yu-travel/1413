/*
***********************************************************************************************************************
    @brief          : 板载外设I2C通信底层接口
    @author         : xiongjinqi
    @date           : 2024/07/21
***********************************************************************************************************************
*/
#ifndef __PERIPH_I2C_H_
#define __PERIPH_I2C_H_

#include "ringbuffer_u8.h"
#include "main.h"

#define IPMI_IIC_TX_INTERRUPT       0       // 1，使用发送中断；0，禁止
#define IIC_DATA_SIZE               512	

#define IIC3_MASTER_ADDR            0x00

#define IIC1_SCL_AF                 GPIO_AF_I2C1
#define IIC1_SCL_SOURCE             GPIO_PinSource6
#define IIC1_SCL_Pin                GPIO_Pin_6
#define IIC1_SCL_GPIO_Port          GPIOB

#define IIC1_SDA_AF                 GPIO_AF_I2C1
#define IIC1_SDA_SOURCE             GPIO_PinSource7
#define IIC1_SDA_Pin                GPIO_Pin_7
#define IIC1_SDA_GPIO_Port          GPIOB

#define IIC2_SCL_AF                 GPIO_AF_I2C2
#define IIC2_SCL_SOURCE             GPIO_PinSource10
#define IIC2_SCL_Pin                GPIO_Pin_10
#define IIC2_SCL_GPIO_Port          GPIOB

#define IIC2_SDA_AF                 GPIO_AF_I2C2
#define IIC2_SDA_SOURCE             GPIO_PinSource11
#define IIC2_SDA_Pin                GPIO_Pin_11
#define IIC2_SDA_GPIO_Port          GPIOB

#define IIC3_SCL_AF                 GPIO_AF_I2C3
#define IIC3_SCL_SOURCE             GPIO_PinSource8
#define IIC3_SCL_Pin                GPIO_Pin_8
#define IIC3_SCL_GPIO_Port          GPIOA

#define IIC3_SDA_AF                 GPIO_AF_I2C3
#define IIC3_SDA_SOURCE             GPIO_PinSource9
#define IIC3_SDA_Pin                GPIO_Pin_9
#define IIC3_SDA_GPIO_Port          GPIOC

/*等待超时时间*/
#define I2CT_FLAG_TIMEOUT         ((uint32_t)0x1000)
#define I2CT_LONG_TIMEOUT         ((uint32_t)(10 * I2CT_FLAG_TIMEOUT))


typedef enum {
    I2C_RX_START= BIT(0),
    I2C_RX_DONE = BIT(1),
    I2C_RX_ING  = BIT(2),   // 接收中
    I2C_TX_START= BIT(3),
    I2C_TX_DONE = BIT(4),
    I2C_TX_ING  = BIT(5),   // 接收中
}I2C_STATUS;
    
typedef enum {
    I2C_MODE_MASTER,
    I2C_MODE_SLAVE,
}I2C_MODE;


typedef struct {
    I2C_TypeDef *dev;
    u8_ring_buffer_t rb_handler;
    u8 rxbuff[IIC_DATA_SIZE];
    u8 txbuff[IIC_DATA_SIZE];
    u16 rxindex;     // 接收中计数
    u16 rxcount;     // 接收总字节数
    u16 txindex;     // 发送计数
    u16 txcount;     // 发送总字节数
    u8 status;
    u8 mode;        // 当前模式
    
    u8 selfaddr;
    u8 peeraddr;
}_i2c_interrpt_t;


extern _i2c_interrpt_t i2c1_int;
extern _i2c_interrpt_t i2c2_int;
extern _i2c_interrpt_t i2c3_int[];


/*
    @brief      : 外设I2C初始化、ringbuffer初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void i2c_all_init(void);


/*
    @brief      : 外设I2C主机发送数据
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
u32 i2c_master_send_bytes(_i2c_interrpt_t* handler, u8 slaveaddr, u8 *buff, u16 len);

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
uint8_t gpioi2c_send_bytes(_i2c_interrpt_t* handler, u8 slaveaddr, u8 *buff, u16 len, u8 mode);

/*
    @brief      : gpio模拟I2C读数据
    @param[in]  : 
        handler         i2c总线句柄
        slaveaddr       从机地址（0-0x7F）
        buff            发送数据缓冲区
        len             发送的数据长度
    @param[out] : none
    @retval     : 读到的数据
*/
uint8_t gpioi2c_read_bytes(_i2c_interrpt_t* handler, u8 slaveaddr);

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
void i2c_slave_send_bytes(_i2c_interrpt_t* handler, u8 slaveaddr, u8 *buff, u16 len);

/**
  * @brief  Basic management of the timeout situation.
  * @param  errorCode：错误代码，可以用来定位是哪个环节出错.
  * @retval 返回0，表示IIC读取失败.
  */
uint32_t I2C_TIMEOUT_UserCallback(uint8_t errorCode);

#if TEST_ENABLE

void periph_i2c_test(void);


#endif /* TEST_ENABLE */

#endif /* __PERIPH_I2C_H_ */

