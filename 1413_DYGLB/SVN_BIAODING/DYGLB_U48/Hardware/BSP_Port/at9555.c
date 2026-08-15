/*
***********************************************************************************************************************
    @brief          : 板载gpio扩展操作
    @author         : xiongjinqi
    @date           : 2024/07/21
***********************************************************************************************************************
*/
#include "gpio.h"
#include "gpio_app.h"
#include "at9555.h"

_myiic_t at9555_iic = {
    .sclport = GPIO_IIC1_SCL_GPIO_Port,
    .sdaport = GPIO_IIC1_SDA_GPIO_Port,
    .sclpin  = GPIO_IIC1_SCL_Pin,
    .sdapin  = GPIO_IIC1_SDA_Pin
};

_at9555_t at9555 = {0};

DECLARED_GPIO_APP_SET(BH54HC573_OE);
DECLARED_GPIO_APP_SET(BH54HC573_LE);



/*
    @brief      : at9555读2个字节
    @param[in]  : 
        addr        读取的寄存器地址
    @param[out] : none
    @retval     : 读到的数据
*/
u16 at9555_16bit_read(_myiic_t *i2cbus, u8 dev_id, u8 addr)
{
    uint8_t dat_l = 0, dat_h = 0;
    u16 value = 0;
    
    iic_start(i2cbus);                /* 发送起始信号 */
    iic_send_byte(i2cbus, dev_id);      /* 发送设备地址 */
    iic_wait_ack(i2cbus);             /* 每次发送完一个字节,都要等待ACK */
    iic_send_byte(i2cbus, addr % 256);  /* 发送低位地址 */
    iic_wait_ack(i2cbus);             /* 等待ACK, 此时地址发送完成了 */
    
    iic_start(i2cbus);                /* 重新发送起始信号 */ 
    iic_send_byte(i2cbus, dev_id|1);    /* 进入接收模式, IIC规定最低位是1, 表示读取 */
    iic_wait_ack(i2cbus);             /* 每次发送完一个字节,都要等待ACK */
    dat_h = iic_read_byte(i2cbus, 1);    /* 接收一个字节数据 */
    dat_l = iic_read_byte(i2cbus, 0);
    iic_stop(i2cbus);                 /* 产生一个停止条件 */
    value = (dat_h<<8) | dat_l;
    
    return value;
}


/*
    @brief      : at9555写2个字节
    @param[in]  : 
        addr        写入的寄存器地址
        data        高8位写入port1, 低8位写入port0
    @param[out] : none
    @retval     : 读到的数据
*/
void at9555_16bit_write(_myiic_t *i2cbus, u8 dev_id, uint8_t addr, u16 data)
{

    iic_start(i2cbus);                /* 发送起始信号 */
    iic_send_byte(i2cbus, dev_id);    /* 发送设备地址 */
    iic_wait_ack(i2cbus);             /* 每次发送完一个字节,都要等待ACK */
    iic_send_byte(i2cbus, addr % 256);/* 发送低位地址 */
    iic_wait_ack(i2cbus);             /* 等待ACK, 此时地址发送完成了 */
    iic_send_byte(i2cbus, data&0xFF); /* 先写port0 */
    iic_wait_ack(i2cbus);             /* 等待ACK, 此时地址发送完成了 */
    iic_send_byte(i2cbus, (data>>8)&0xFF);/* 再写port0 */
    iic_wait_ack(i2cbus);             /* 等待ACK, 此时地址发送完成了 */
    iic_stop(i2cbus);                 /* 产生一个停止条件 */
}



/*
    @brief      : Gpio扩展AT9555初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void at9555_init(void)
{
    at9555.i2cbus   = &at9555_iic;
    at9555.devid[0] = AT9555_DEV_1_ID;
    at9555.devid[1] = AT9555_DEV_2_ID;

    gpio_i2c_init();
    bh53hc573_en_init();

    /* 全部配置为输出0 */
    at9555_16bit_write(at9555.i2cbus, at9555.devid[0], AT9555_REG02, at9555.output[0]);
    at9555_16bit_write(at9555.i2cbus, at9555.devid[1], AT9555_REG02, at9555.output[0]);
    
    /* 全部配置为输出模式 */
    at9555_16bit_write(at9555.i2cbus, at9555.devid[0], AT9555_REG06, at9555.config[0]);
    at9555_16bit_write(at9555.i2cbus, at9555.devid[1], AT9555_REG06, at9555.config[0]);
}



/*
    @brief      : Gpio扩展AT9555初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void at9555_output_config(u16 dev1_data, u16 dev2_data)
{
    /* 1、禁止输出 */
    gpio_setting(&app_BH54HC573_OE, 1);

    /* 2、设置输出值 */
    at9555_16bit_write(at9555.i2cbus, at9555.devid[0], AT9555_REG02, dev1_data);
    at9555_16bit_write(at9555.i2cbus, at9555.devid[0], AT9555_REG02, dev2_data);
    delay_ms(2);
    
    /* 3、使能输出 */
    gpio_setting(&app_BH54HC573_LE, 1);
    gpio_setting(&app_BH54HC573_OE, 0);
}


#if TEST_ENABLE
void at9555_test(void)
{
    static u8 test_flag = 0;
    if(test_flag == 0)
    {
        test_flag++;
        //at9555_output_config(0xFFFF, 0xFFFF);
        at9555_output_config(0, 0);
    }
}


#endif


