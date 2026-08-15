/*
***********************************************************************************************************************
    @brief          : 板载adc扩展操作(基于at8951t)
    @author         : xiongjinqi
    @date           : 2024/07/21
***********************************************************************************************************************
*/
#include "gpio.h"
#include "gpio_app.h"
#include "at8591t.h"

_myiic_t at8951t_iic1 = {
    .sclport = ADC_IIC1_SCL_GPIO_Port,
    .sdaport = ADC_IIC1_SDA_GPIO_Port,
    .sclpin  = ADC_IIC1_SCL_Pin,
    .sdapin  = ADC_IIC1_SDA_Pin
};

_myiic_t at8951t_iic2 = {
    .sclport = ADC_IIC2_SCL_GPIO_Port,
    .sdaport = ADC_IIC2_SDA_GPIO_Port,
    .sclpin  = ADC_IIC2_SCL_Pin,
    .sdapin  = ADC_IIC2_SDA_Pin
};

_board_voltage_t board_adc = {0};

_at8591t_t at8951t[] = {
    {
        .i2cbus = &at8951t_iic1,
        .devid  = AT8591T_DEV_1_ID,
    },
    {
        .i2cbus = &at8951t_iic1,
        .devid  = AT8591T_DEV_2_ID,
    },
    {
        .i2cbus = &at8951t_iic1,
        .devid  = AT8591T_DEV_3_ID,
    },
    {
        .i2cbus = &at8951t_iic1,
        .devid  = AT8591T_DEV_4_ID,
    },

    
    {
        .i2cbus = &at8951t_iic2,
        .devid  = AT8591T_DEV_5_ID,
    },
    {
        .i2cbus = &at8951t_iic2,
        .devid  = AT8591T_DEV_6_ID,
    },
    {
        .i2cbus = &at8951t_iic2,
        .devid  = AT8591T_DEV_7_ID,
    },
    {
        .i2cbus = &at8951t_iic2,
        .devid  = AT8591T_DEV_8_ID,
    },
};


/*
    @brief      : at8591t写一个字节
    @param[in]  : 
        addr        写入的寄存器地址
    @param[out] : none
    @retval     : 读到的数据
*/
uint8_t at8591t_8bit_write(_myiic_t *i2cbus, u8 dev_id, uint8_t addr, u8 data)
{
    uint8_t temp = 0;
    iic_start(i2cbus);                /* 发送起始信号 */
    iic_send_byte(i2cbus, dev_id);      /* 发送设备地址 */
    iic_wait_ack(i2cbus);             /* 每次发送完一个字节,都要等待ACK */
    iic_send_byte(i2cbus, addr % 256);  /* 发送低位地址 */
    iic_wait_ack(i2cbus);             /* 等待ACK, 此时地址发送完成了 */
    iic_send_byte(i2cbus, data);
    iic_wait_ack(i2cbus);             /* 等待ACK, 此时地址发送完成了 */
    iic_stop(i2cbus);                 /* 产生一个停止条件 */
    return temp;
}

/*
    @brief      : at8591t读一个字节
    @param[in]  : 
        dev_id        读取的设备ID
        buff          读取数据存放地址的指针
        len           读取的长度
    @param[out] : none
    @retval     : 读到的数据
*/
uint8_t at8591t_8bit_read(_myiic_t *i2cbus, u8 dev_id, u8*buff, u8 len)
{
    u8 temp = 0;
    #if 1
    iic_start(i2cbus);                /* 发送起始信号 */
    iic_send_byte(i2cbus, dev_id);   /* 发送设备地址 */
    iic_wait_ack(i2cbus);
    iic_send_byte(i2cbus, 0x04);    /* 写控制byte */
    iic_wait_ack(i2cbus);
    iic_stop(i2cbus);
    
    delay_us(50);
    #endif 
    
    iic_start(i2cbus);                /* 发送起始信号 */
    iic_send_byte(i2cbus, dev_id|1);   /* 发送设备地址 */
    iic_wait_ack(i2cbus);             /* 每次发送完一个字节,都要等待ACK */
    for(int i=0; i<len; i++)
    {
        if(i==len-1)
            buff[i] = iic_read_byte(i2cbus, 0);    /* 最后一个字节NACK */
        else
            buff[i] = iic_read_byte(i2cbus, 1);    /* 接收一个字节数据 */
    }
    iic_stop(i2cbus);                 /* 产生一个停止条件 */

    return 0;
}



/*
    @brief      : Gpio扩展AT8591T初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void at8951t_init(void)
{
    adc_i2c1_init();
    adc_i2c2_init();
}



/*
    @brief      : Gpio扩展AT8591T初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none

    数据定义：
    12V28V_HJJC2_XCA4001, // i2c1 id 0x98
    12V28V_HJJC3_XCA4001, // i2c1  id 0x98
    12V_WAOXJ_XCA4001 ,   // i2c1  id 0x98
    12V_GSDJ2_XCA4001 ,   // i2c1  id 0x98

    28V_KF_XCA4001 ,          // i2c1 id 0x94
    28V_BQXJ_XCA4001 ,        // i2c1 id 0x94
    12V_GSDJ1_XCA4001_OUT ,   // i2c1 id 0x94
    12V_DYGY_B_XCA4001 ,      // i2c1 id 0x94

    12V28V_BF2_XCA4001 ,      // i2c1 id 0x92
    28V_QGSJ_XCA4001 ,        // i2c1 id 0x92
    28V_JCXJ_XCA4001 ,        // i2c1 id 0x92
    28V_FFXJ_XCA4001 ,        // i2c1 id 0x92

    28V_TSGY_XCA4001 ,        // i2c1 id 0x9c
    12V28V_BF1_XCA4001 ,      // i2c1 id 0x9c
    12V_XGHTM_XCA4001 ,       // i2c1 id 0x9c
    12V28V_HJJC1_XCA4001 ,    // i2c1 id 0x9c

    28V_JCXJ_TST ,    // i2c2 id 0x98
    28V_QGSJ_TST ,    // i2c2 id 0x98
    12V_DYGY_B_TST ,  // i2c2 id 0x98
    12V_GSDJ1_TST ,   // i2c2 id 0x98

    12V_XGHTM_TST ,     // i2c2 id 0x94
    12V28V_HJJC1_TST , // i2c2 id 0x94
    12V28V_HJJC2_TST , // i2c2 id 0x94
    12V28V_HJJC3_TST , // i2c2 id 0x94

    12V_GSDJ2_TST , // i2c2 id 0x92
    12V_WAOXJ_TST , // i2c2 id 0x92
    28V_TSGY_TST , // i2c2 id 0x92
    12V28V_BF1_TST , // i2c2 id 0x92

    12V28V_BF2_TST , // i2c2 id 0x9c
    28V_FFXJ_TST , // i2c2 id 0x9c
    28V_KF_TST , // i2c2 id 0x9c
    28V_BQXJ_TST , // i2c2 id 0x9c

*/
void at8951t_convert(_board_voltage_t *board)
{
    int ret = 0;
    /* 1、读取原始数据 */
    for(int i=0; i<ARRAY_SIZE(at8951t); i++)
    {
        ret = at8591t_8bit_read(at8951t[i].i2cbus, at8951t[i].devid, at8951t[i].adcdata, 4);
    }

    /* 2、转换数据为电压电流值 */
    //board->VOL_28V_JCXJ.current = CURR_28V_JCXJ_CONVERT(at8951t[2].adcdata[3]);
    //board->VOL_28V_JCXJ.voltage = VOL_28V_JCXJ_CONVERT(at8951t[4].adcdata[1]);
    AT8591T_RAW_CONVERT(28V_JCXJ,   board, at8951t[2].adcdata[2], at8951t[4].adcdata[0]);
    AT8591T_RAW_CONVERT(12V_WAOXJ,  board, at8951t[0].adcdata[2], at8951t[6].adcdata[1]);
    AT8591T_RAW_CONVERT(28V_BQXJ,   board, at8951t[1].adcdata[1], at8951t[7].adcdata[3]);
    AT8591T_RAW_CONVERT(12V_DYGY_B, board, at8951t[1].adcdata[3], at8951t[4].adcdata[2]);
    AT8591T_RAW_CONVERT(28V_FFXJ,   board, at8951t[2].adcdata[3], at8951t[7].adcdata[1]);
    AT8591T_RAW_CONVERT(12V28V_BF1, board, at8951t[3].adcdata[1], at8951t[6].adcdata[3]);
    AT8591T_RAW_CONVERT(12V28V_BF2, board, at8951t[2].adcdata[0], at8951t[7].adcdata[0]);
    AT8591T_RAW_CONVERT(28V_TSGY,   board, at8951t[3].adcdata[0], at8951t[6].adcdata[2]);
    AT8591T_RAW_CONVERT(28V_QGSJ,   board, at8951t[2].adcdata[1], at8951t[4].adcdata[1]);
    AT8591T_RAW_CONVERT(12V28V_HJJC1,board,at8951t[3].adcdata[3], at8951t[5].adcdata[1]);
    AT8591T_RAW_CONVERT(12V28V_HJJC2,board,at8951t[0].adcdata[0], at8951t[5].adcdata[2]);
    AT8591T_RAW_CONVERT(12V28V_HJJC3,board,at8951t[2].adcdata[1], at8951t[5].adcdata[3]);
    AT8591T_RAW_CONVERT(12V_GSDJ1,  board, at8951t[1].adcdata[2], at8951t[4].adcdata[3]);
    AT8591T_RAW_CONVERT(12V_GSDJ2,  board, at8951t[0].adcdata[3], at8951t[6].adcdata[0]);
    AT8591T_RAW_CONVERT(12V_XGHTM,  board, at8951t[3].adcdata[2], at8951t[5].adcdata[0]);
    AT8591T_RAW_CONVERT(28V_KF,     board, at8951t[1].adcdata[0], at8951t[7].adcdata[2]);

    /* 打印原始数据及计算后的数据 */
    TRACE_OUT(DEBUG_OUT, "====================================================\r\n");
    AT8591T_PRINT(28V_JCXJ,   board, at8951t[2].adcdata[2], at8951t[4].adcdata[0]);
    AT8591T_PRINT(12V_WAOXJ,  board, at8951t[0].adcdata[2], at8951t[6].adcdata[1]);
    AT8591T_PRINT(28V_BQXJ,   board, at8951t[1].adcdata[1], at8951t[7].adcdata[3]);
    AT8591T_PRINT(12V_DYGY_B, board, at8951t[1].adcdata[3], at8951t[4].adcdata[2]);
    AT8591T_PRINT(28V_FFXJ,   board, at8951t[2].adcdata[3], at8951t[7].adcdata[1]);
    AT8591T_PRINT(12V28V_BF1, board, at8951t[3].adcdata[1], at8951t[6].adcdata[3]);
    AT8591T_PRINT(12V28V_BF2, board, at8951t[2].adcdata[0], at8951t[7].adcdata[0]);
    AT8591T_PRINT(28V_TSGY,   board, at8951t[3].adcdata[0], at8951t[6].adcdata[2]);
    AT8591T_PRINT(28V_QGSJ,   board, at8951t[2].adcdata[1], at8951t[4].adcdata[1]);
    AT8591T_PRINT(12V28V_HJJC1,board,at8951t[3].adcdata[3], at8951t[3].adcdata[1]);
    AT8591T_PRINT(12V28V_HJJC2,board,at8951t[0].adcdata[0], at8951t[5].adcdata[2]);
    AT8591T_PRINT(12V28V_HJJC3,board,at8951t[2].adcdata[1], at8951t[5].adcdata[3]);
    AT8591T_PRINT(12V_GSDJ1,  board, at8951t[1].adcdata[2], at8951t[4].adcdata[3]);
    AT8591T_PRINT(12V_GSDJ2,  board, at8951t[0].adcdata[3], at8951t[6].adcdata[0]);
    AT8591T_PRINT(12V_XGHTM,  board, at8951t[3].adcdata[2], at8951t[5].adcdata[0]);
    AT8591T_PRINT(28V_KF,     board, at8951t[1].adcdata[0], at8951t[7].adcdata[2]);
}



