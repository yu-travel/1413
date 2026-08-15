/*
*********************************************************************************************************************
    @brief      : LTC2990驱动文件
    @author     : xjq
    @date       : 2024/06/12
    @history    : v1.0
*********************************************************************************************************************
*/
#include "delay.h"
#include "ltc2990.h"

s8 g_mon_current = 0;

/*
    @brief      : ltc2990读字节
    @param[in]  : 
        addr        读取的寄存器地址
        buff        读取数据的缓冲区
        len         读取的长度
    @param[out] : none
    @retval     : 读到的数据
*/
uint8_t ltc2990_byte_read(uint8_t addr)
{
    uint8_t tmp = 0;
    iic3_start();                /* 发送起始信号 */
    iic3_send_byte(LTC2990_DEV_ID);   /* 发送设备地址 */
    iic3_wait_ack();             /* 每次发送完一个字节,都要等待ACK */
    iic3_send_byte(addr % 256);  /* 发送低位地址 */
    iic3_wait_ack();             /* 等待ACK, 此时地址发送完成了 */
    
    iic3_start();                /* 重新发送起始信号 */ 
    iic3_send_byte(LTC2990_DEV_ID|0x01);    /* 进入接收模式, IIC规定最低位是1, 表示读取 */
    iic3_wait_ack();             /* 每次发送完一个字节,都要等待ACK */
    tmp = iic3_read_byte(0);     /* 接收一个字节数据 */
    
    iic3_stop();                 /* 产生一个停止条件 */

    return tmp;
}


/*
    @brief      : ltc2990写字节
    @param[in]  : 
        addr        写入的寄存器地址
    @param[out] : none
    @retval     : 读到的数据
*/
uint8_t ltc2990_byte_write(uint8_t addr, u8 data)
{
    iic3_start();                /* 发送起始信号 */
    iic3_send_byte(LTC2990_DEV_ID);   /* 发送设备地址 */
    iic3_wait_ack();             /* 每次发送完一个字节,都要等待ACK */
    iic3_send_byte(addr % 256);  /* 发送低位地址 */
    iic3_wait_ack();             /* 等待ACK, 此时地址发送完成了 */
    iic3_send_byte(data);  /* 发送低位地址 */
    iic3_wait_ack();             /* 每次发送完一个字节,都要等待ACK */
    iic3_stop();                 /* 产生一个停止条件 */

    return 0;
}


/*
*********************************************************************************************************************
    @brief      : 配置LTC2990
*********************************************************************************************************************
*/
/*
    @brief      : 控制寄存器配置
*/
void ltc2990_control_reg_set(void)
{
    u8 regval = 0x59;   //  Single Acquisition; TR1, V1 or V1 – V2 Only per Mode [2:0]; V1 – V2, TR2
    ltc2990_byte_write(LTC2990_REG_01, regval);
}

/*
    @brief      : 触发寄存器，写入任何值都将启动转换器
*/
void ltc2990_conver_start(void)
{
    u8 regval = 0xFF;
    ltc2990_byte_write(LTC2990_REG_02, regval);
}

/*
    @brief      : 获取电流值
*/
s8 ltc2990_current_get(void)
{
    u8 dat_l = 0, dat_h = 0;
    u16 regval = 0, tmp = 0;
    
    ltc2990_conver_start(); // 启动转换器
    delay_ms(50);
    dat_h = ltc2990_byte_read(LTC2990_REG_06);
    dat_l = ltc2990_byte_read(LTC2990_REG_07);

    regval = (dat_h<<8) | dat_l;
    if(regval&(1<<15))  // 判断数据是否是最新的
    {
        tmp = regval&0x7FFF;
        if(tmp&(1<<14)) // 有符号数
        {
            tmp = ~tmp +1;
            tmp = tmp & 0x3FFF;
            g_mon_current = (s8)(-(float)(tmp*3.884*10)/1000.0);
        }
        else
        {
            g_mon_current = (s8)((float)(tmp*3.884*10)/1000.0); // 保留1位小数
        }
    }
    TRACE_OUT(DEBUG_OUT, "======================\r\n");
    TRACE_OUT(DEBUG_OUT, "raw regval [%04x], current val [%d]A \r\n", regval, g_mon_current);
    return g_mon_current;
}


/*
    @brief      : ltc2990初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void ltc2990_init(void)
{
    u8 reg_status = 0;
    iic3_init();
    ltc2990_control_reg_set();
    delay_ms(2);
    ltc2990_conver_start();
    
    delay_ms(2);
    reg_status = ltc2990_byte_read(LTC2990_REG_01);
    TRACE_OUT(DEBUG_OUT, "LTC2990 control regval [%04x]\r\n", reg_status);
    
    delay_ms(2);
    reg_status = ltc2990_byte_read(LTC2990_REG_00);
    TRACE_OUT(DEBUG_OUT, "LTC2990 status regval [%04x]\r\n", reg_status);
}




