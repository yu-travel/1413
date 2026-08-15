/*
*********************************************************************************************************************
    @brief      : SD5075驱动文件
    @author     : xjq
    @date       : 2024/06/12
    @history    : v1.0
*********************************************************************************************************************
*/
#include "myiic.h"
#include "sd5075.h"


_sd5075_t tmp_sd5075 = {
    .devid[0] = SD5075_DEV_ID_1,
    .devid[1] = SD5075_DEV_ID_2,
    .devid[2] = SD5075_DEV_ID_3,
};



/*
    @brief      : sd5075初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void sd5075_init(void)
{
    iic_init();
}


/*
    @brief      : sd5075读一个字节
    @param[in]  : 
        addr        读取的寄存器地址
    @param[out] : none
    @retval     : 读到的数据
*/
uint8_t sd5075_8bit_read(u8 dev_id, uint8_t addr)
{
    uint8_t temp = 0;
    iic_start();                /* 发送起始信号 */
    iic_send_byte(dev_id<<1);   /* 发送设备地址 */
    iic_wait_ack();             /* 每次发送完一个字节,都要等待ACK */
    iic_send_byte(addr % 256);  /* 发送低位地址 */
    iic_wait_ack();             /* 等待ACK, 此时地址发送完成了 */
    
    iic_start();                /* 重新发送起始信号 */ 
    iic_send_byte((dev_id<<1)|1);    /* 进入接收模式, IIC规定最低位是1, 表示读取 */
    iic_wait_ack();             /* 每次发送完一个字节,都要等待ACK */
    temp = iic_read_byte(0);    /* 接收一个字节数据 */
    iic_stop();                 /* 产生一个停止条件 */

    return temp;
}

/*
    @brief      : sd5075读2个字节
    @param[in]  : 
        addr        读取的寄存器地址
    @param[out] : none
    @retval     : 读到的数据
*/
s16 sd5075_16bit_read(u8 dev_id, uint8_t addr)
{
    uint8_t dat_l = 0, dat_h = 0;
    s16 value = 0;
    
    iic_start();                /* 发送起始信号 */
    iic_send_byte(dev_id<<1);      /* 发送设备地址 */
    iic_wait_ack();             /* 每次发送完一个字节,都要等待ACK */
    iic_send_byte(addr % 256);  /* 发送低位地址 */
    iic_wait_ack();             /* 等待ACK, 此时地址发送完成了 */
    
    iic_start();                /* 重新发送起始信号 */ 
    iic_send_byte((dev_id<<1)|1);    /* 进入接收模式, IIC规定最低位是1, 表示读取 */
    iic_wait_ack();             /* 每次发送完一个字节,都要等待ACK */
    dat_h = iic_read_byte(1);    /* 接收一个字节数据 */
    dat_l = iic_read_byte(0);
    iic_stop();                 /* 产生一个停止条件 */
    value = (dat_h<<8) | dat_l;
    
    return value;
}


/*
    @brief      : sd5075写一个字节
    @param[in]  : 
        addr        写入的寄存器地址
    @param[out] : none
    @retval     : 读到的数据
*/
uint8_t sd5075_8bit_write(u8 dev_id, uint8_t addr, u8 data)
{
    uint8_t temp = 0;
    iic_start();                /* 发送起始信号 */
    iic_send_byte(dev_id<<1);      /* 发送设备地址 */
    iic_wait_ack();             /* 每次发送完一个字节,都要等待ACK */
    iic_send_byte(addr % 256);  /* 发送低位地址 */
    iic_wait_ack();             /* 等待ACK, 此时地址发送完成了 */
    iic_send_byte(data);
    iic_wait_ack();             /* 等待ACK, 此时地址发送完成了 */
    iic_stop();                 /* 产生一个停止条件 */
    return temp;
}

/*
    @brief      : sd5075写2个字节
    @param[in]  : 
        addr        写入的寄存器地址
    @param[out] : none
    @retval     : 读到的数据
*/
uint8_t sd5075_16bit_write(u8 dev_id, uint8_t addr, u16 data)
{
    uint8_t temp = 0;
    iic_start();                /* 发送起始信号 */
    iic_send_byte(dev_id<<1);   /* 发送设备地址 */
    iic_wait_ack();             /* 每次发送完一个字节,都要等待ACK */
    iic_send_byte(addr % 256);  /* 发送低位地址 */
    iic_wait_ack();             /* 等待ACK, 此时地址发送完成了 */
    iic_send_byte((data>>8)&0xFF);
    iic_wait_ack();             /* 等待ACK, 此时地址发送完成了 */
    iic_send_byte(data&0xFF);
    iic_wait_ack();             /* 等待ACK, 此时地址发送完成了 */
    iic_stop();                 /* 产生一个停止条件 */
    return temp;
}

/*
*********************************************************************************************************************
    @brief      : 配置SD5075
*********************************************************************************************************************
*/

/*
    @brief      : sd5075获取板载所有温度传感器数据
    @param[in]  : 
        handler        温度传感器句柄
    @param[out] : none
    @retval     : none
*/
int sd5075_tempreture_get(_sd5075_t *handler)
{
    u16 temp[3] = {0};
    /* 获取温度传感器原始数据 */
    temp[0] = sd5075_16bit_read(handler->devid[0], SD5075_REG_00)>>4;
    temp[1] = sd5075_16bit_read(handler->devid[1], SD5075_REG_00)>>4;
    temp[2] = sd5075_16bit_read(handler->devid[2], SD5075_REG_00)>>4;

    /* 计算温度值 */
    handler->tempreture[0] = SD5075_TEMP_CALC(temp[0]);
    handler->tempreture[1] = SD5075_TEMP_CALC(temp[1]);
    handler->tempreture[2] = SD5075_TEMP_CALC(temp[2]);
    
    TRACE_OUT(DEBUG_OUT, "======================\r\n");
    TRACE_OUT(DEBUG_OUT, "rawval[%04x] TEMP0 %d\r\n", temp[0], handler->tempreture[0]);
    TRACE_OUT(DEBUG_OUT, "rawval[%04x] TEMP1 %d\r\n", temp[1], handler->tempreture[0]);
    TRACE_OUT(DEBUG_OUT, "rawval[%04x] TEMP2 %d\r\n", temp[2], handler->tempreture[0]);
    
    return 0;
}

/*
    @brief      : sd5075设置过温报警值
        @note       建议上下限阈值合理设置
    @param[in]  : 
        handler         温度传感器句柄
        alarm_down      设置过温报警恢复的下限温度，芯片默认值75摄氏度
        alarm_upper     设置过温报警恢复的上限温度，芯片默认值80摄氏度
    @param[out] : none
    @retval     : none
*/
int sd5075_alarm_tempreture_set(_sd5075_t *handler, u8 alarm_down, u8 alarm_upper)
{
    /* 写下限值 */
    sd5075_16bit_write(handler->devid[0], SD5075_REG_02, alarm_down<<8);
    sd5075_16bit_write(handler->devid[1], SD5075_REG_02, alarm_down<<8);
    sd5075_16bit_write(handler->devid[2], SD5075_REG_02, alarm_down<<8);
    
    /* 写上限值 */
    sd5075_16bit_write(handler->devid[0], SD5075_REG_03, alarm_upper<<8);
    sd5075_16bit_write(handler->devid[1], SD5075_REG_03, alarm_upper<<8);
    sd5075_16bit_write(handler->devid[2], SD5075_REG_03, alarm_upper<<8);
}



