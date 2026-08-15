/*
***********************************************************************************************************************
    @brief          : 板载gx21m15温度传感器
    @author         : xiongjinqi
    @date           : 2024/07/21
***********************************************************************************************************************
*/
#include "periph_i2c.h"
#include "gx21m15.h"


#define GX21M15_REG00        0x00        //Input port register pair
#define GX21M15_REG01        0x01        //Input port register pair
#define GX21M15_REG02        0x02        //Output port register pair
#define GX21M15_REG03        0x03        //Output port register pair
#define GX21M15_REG06        0x06        //Configuration port register pair

static __IO uint32_t  I2CTimeout = I2CT_LONG_TIMEOUT;


_tempreture_t temp1_handler = {
    .i2cbus = &i2c3_int[0],
};

_tempreture_t temp2_handler = {
    .i2cbus = &i2c3_int[1],
};


//void gx21m15_rawtotemp(_tempreture_t *temp)
//{
//    u16 tmp_rawval = 0;
//    tmp_rawval = temp->rawval;
//    tmp_rawval >>= 5;
//    if(tmp_rawval&(1<<10)) // 负温
//    {
//        tmp_rawval =~(tmp_rawval)+1;
//        temp->temp = (s16)(-(tmp_rawval*12.5)); // 扩大100倍
//    }
//    else
//    {
//        temp->temp = (s16)(tmp_rawval*12.5); // 扩大100倍
//    }
//}

void gx21m15_rawtotemp(_tempreture_t *temp)
{
    s16 raw11 = (temp->rawval >> 5) & 0x07FF;
    if(raw11 & 0x0400)
    {
        raw11 |= 0xF800;
    }
    temp->temp = (s16)((raw11 * 125) / 10);
}



/**
  * @brief   从gx21m15里面读取温度值
  * @param   
  *     @arg pBuffer:存放从EEPROM读取的数据的缓冲区指针
  *     @arg WriteAddr:接收数据的EEPROM的地址
  *     @arg NumByteToWrite:要从EEPROM读取的字节数
  * @retval  无
  */
u32 gx21m15_tempreture_get(_tempreture_t *temp)
{
    u8 rd_temp[2] = {0};
    u8 rd_count = 2, rd_index = 0;
    
    I2CTimeout = I2CT_LONG_TIMEOUT;
    //*((u8 *)0x4001080c) |=0x80; 
    while(I2C_GetFlagStatus(temp->i2cbus->dev, I2C_FLAG_BUSY))
    {
        if((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(9);
    }
    /* Send START condition */
    I2C_GenerateSTART(temp->i2cbus->dev, ENABLE);
    //*((u8 *)0x4001080c) &=~0x80;

    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* Test on EV5 and clear it */
    while(!I2C_CheckEvent(temp->i2cbus->dev, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(10);
    }

    /* Send EEPROM address for write */
    I2C_Send7bitAddress(temp->i2cbus->dev, temp->i2cbus->peeraddr, I2C_Direction_Transmitter);

    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* Test on EV6 and clear it */
    while(!I2C_CheckEvent(temp->i2cbus->dev, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) 
    {
        if((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(11);
    }
    /* Clear EV6 by setting again the PE bit */
    I2C_Cmd(temp->i2cbus->dev, ENABLE);

    /* Send the EEPROM's internal address to write to */
    I2C_SendData(temp->i2cbus->dev, GX21M15_REG00);

     I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* Test on EV8 and clear it */
    while(!I2C_CheckEvent(temp->i2cbus->dev, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(12);
    }
    
    /* Send STRAT condition a second time */
    I2C_GenerateSTART(temp->i2cbus->dev, ENABLE);

     I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* Test on EV5 and clear it */
    while(!I2C_CheckEvent(temp->i2cbus->dev, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(13);
    }
    /* Send EEPROM address for read */
    I2C_Send7bitAddress(temp->i2cbus->dev, temp->i2cbus->peeraddr|1, I2C_Direction_Receiver);

     I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* Test on EV6 and clear it */
    while(!I2C_CheckEvent(temp->i2cbus->dev, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    {
        if((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(14);
    }
    
    /* While there is data to be read */
    while(rd_count)
    {
        if(rd_count == 1)
        {
            /* Disable Acknowledgement */
            I2C_AcknowledgeConfig(temp->i2cbus->dev, DISABLE);

            /* Send STOP Condition */
            I2C_GenerateSTOP(temp->i2cbus->dev, ENABLE);
        }

        I2CTimeout = I2CT_LONG_TIMEOUT;
        while(I2C_CheckEvent(temp->i2cbus->dev, I2C_EVENT_MASTER_BYTE_RECEIVED)==0)  
        {
            if((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(3);
        }
        {
              /* Read a byte from the device */
              rd_temp[rd_index++] = I2C_ReceiveData(temp->i2cbus->dev);
              /* Decrement the read bytes counter */
              rd_count--;
        }
    }

    /* Enable Acknowledgement to be ready for another reception */
    I2C_AcknowledgeConfig(temp->i2cbus->dev, ENABLE);
    
    temp->rawval = (rd_temp[0]<<8) | rd_temp[1];
    TRACE_OUT(DEBUG_OUT,"%p rawval[%04x]\r\n", temp, temp->rawval);
    gx21m15_rawtotemp(temp);    // 转化温度

    return 0;
}

/*
    @brief      : 获取平均温度值
*/
s16 gx21m15_temp_avg(void)
{
    s16 temp_avg = 0;
    
    gx21m15_tempreture_get(&temp1_handler);
    //gx21m15_tempreture_get(&temp2_handler);

    //temp_avg = (temp1_handler.temp + temp2_handler.temp)/2;
    temp_avg = temp1_handler.temp;
    TRACE_OUT(DEBUG_OUT,"======================================\r\n");
    TRACE_OUT(DEBUG_OUT, "TEMP1[%d] AVG[%d]\r\n", \
            temp1_handler.temp, temp_avg);

    return temp_avg;
}



