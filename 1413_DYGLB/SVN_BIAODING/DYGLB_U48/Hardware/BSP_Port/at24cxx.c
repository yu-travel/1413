/**
  ******************************************************************************
  * @file    bsp_i2c_ee.c
  * @author  STMicroelectronics
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   i2c EEPROM(AT24C02)应用函数bsp
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火  STM32 F407 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 
#include "periph_i2c.h"
#include "at24cxx.h"
#include "main.h"

static __IO uint32_t  I2CTimeout = I2CT_LONG_TIMEOUT;


_i2c_interrpt_t eeprom = {
    .dev = I2C1,
    .selfaddr = IIC3_MASTER_ADDR,
    .peeraddr = AT24C02_DEV_ID,
    .status   = I2C_RX_START,
};


#if 0
/**
  * @brief  I2C1 I/O配置
  * @param  无
  * @retval 无
  */
static void I2C_GPIO_Config(void)
{

  GPIO_InitTypeDef  GPIO_InitStructure; 
   
  /*!< EEPROM_I2C Periph clock enable */
  EEPROM_I2C_CLK_INIT(EEPROM_I2C_CLK, ENABLE);
  
  /*!< EEPROM_I2C_SCL_GPIO_CLK and EEPROM_I2C_SDA_GPIO_CLK Periph clock enable */
  RCC_AHB1PeriphClockCmd(EEPROM_I2C_SCL_GPIO_CLK | EEPROM_I2C_SDA_GPIO_CLK, ENABLE);

  /*!< GPIO configuration */
  /* Connect PXx to I2C_SCL*/
  GPIO_PinAFConfig(EEPROM_I2C_SCL_GPIO_PORT, EEPROM_I2C_SCL_SOURCE, EEPROM_I2C_SCL_AF);
  /* Connect PXx to I2C_SDA*/
  GPIO_PinAFConfig(EEPROM_I2C_SDA_GPIO_PORT, EEPROM_I2C_SDA_SOURCE, EEPROM_I2C_SDA_AF);  
  
  /*!< Configure EEPROM_I2C pins: SCL */   
  GPIO_InitStructure.GPIO_Pin = EEPROM_I2C_SCL_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(EEPROM_I2C_SCL_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure EEPROM_I2C pins: SDA */
  GPIO_InitStructure.GPIO_Pin = EEPROM_I2C_SDA_PIN;
  GPIO_Init(EEPROM_I2C_SDA_GPIO_PORT, &GPIO_InitStructure);
 
}

/**
  * @brief  I2C 工作模式配置
  * @param  无
  * @retval 无
  */
static void I2C_Mode_Configu(void)
{
  I2C_InitTypeDef  I2C_InitStructure; 

  /* I2C 配置 */
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;    
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;                            /* 高电平数据稳定，低电平数据变化 SCL 时钟线的占空比 */
  I2C_InitStructure.I2C_OwnAddress1 =I2C_OWN_ADDRESS7; 
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable ;  
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; /* I2C的寻址模式 */
  I2C_InitStructure.I2C_ClockSpeed = I2C_Speed;                             /* 通信速率 */
  I2C_Init(EEPROM_I2C, &I2C_InitStructure);                                       /* I2C1 初始化 */
  I2C_Cmd(EEPROM_I2C, ENABLE);                                                      /* 使能 I2C1 */

  I2C_AcknowledgeConfig(EEPROM_I2C, ENABLE);  
}

/**
  * @brief  I2C 外设(EEPROM)初始化
  * @param  无
  * @retval 无
  */
void I2C_EE_Init(void)
{
  I2C_GPIO_Config(); 
 
  I2C_Mode_Configu();


/* 根据头文件i2c_ee.h中的定义来选择EEPROM要写入的地址 */
#ifdef EEPROM_Block0_ADDRESS
  /* 选择 EEPROM Block0 来写入 */
  EEPROM_ADDRESS = EEPROM_Block0_ADDRESS;
#endif

#ifdef EEPROM_Block1_ADDRESS  
    /* 选择 EEPROM Block1 来写入 */
  EEPROM_ADDRESS = EEPROM_Block1_ADDRESS;
#endif

#ifdef EEPROM_Block2_ADDRESS  
    /* 选择 EEPROM Block2 来写入 */
  EEPROM_ADDRESS = EEPROM_Block2_ADDRESS;
#endif

#ifdef EEPROM_Block3_ADDRESS  
    /* 选择 EEPROM Block3 来写入 */
  EEPROM_ADDRESS = EEPROM_Block3_ADDRESS;
#endif
}
#endif 


/**
  * @brief   将缓冲区中的数据写到I2C EEPROM中
  * @param   
  *     @arg pBuffer:缓冲区指针
  *     @arg WriteAddr:写地址
  *     @arg NumByteToWrite:写的字节数
  * @retval  无
  */
void I2C_EE_BufferWrite(u8* pBuffer, u8 WriteAddr, u16 NumByteToWrite)
{
    u8 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0;

    Addr = WriteAddr % I2C_PageSize;
    count = I2C_PageSize - Addr;
    NumOfPage =  NumByteToWrite / I2C_PageSize;
    NumOfSingle = NumByteToWrite % I2C_PageSize;

    /* If WriteAddr is I2C_PageSize aligned  */
    if(Addr == 0) 
    {
        /* If NumByteToWrite < I2C_PageSize */
        if(NumOfPage == 0) 
        {
            I2C_EE_PageWrite(pBuffer, WriteAddr, NumOfSingle);
            I2C_EE_WaitEepromStandbyState();
        }
        /* If NumByteToWrite > I2C_PageSize */
        else  
        {
            while(NumOfPage--)
            {
                I2C_EE_PageWrite(pBuffer, WriteAddr, I2C_PageSize); 
                I2C_EE_WaitEepromStandbyState();
                WriteAddr +=  I2C_PageSize;
                pBuffer += I2C_PageSize;
            }

            if(NumOfSingle!=0)
            {
                I2C_EE_PageWrite(pBuffer, WriteAddr, NumOfSingle);
                I2C_EE_WaitEepromStandbyState();
            }
        }
    }
    /* If WriteAddr is not I2C_PageSize aligned  */
    else 
    {
        /* If NumByteToWrite < I2C_PageSize */
        if(NumOfPage== 0) 
        {
            I2C_EE_PageWrite(pBuffer, WriteAddr, NumOfSingle);
            I2C_EE_WaitEepromStandbyState();
        }
        /* If NumByteToWrite > I2C_PageSize */
        else
        {
            NumByteToWrite -= count;
            NumOfPage =  NumByteToWrite / I2C_PageSize;
            NumOfSingle = NumByteToWrite % I2C_PageSize;  
              
            if(count != 0)
            {
                I2C_EE_PageWrite(pBuffer, WriteAddr, count);
                I2C_EE_WaitEepromStandbyState();
                WriteAddr += count;
                pBuffer += count;
            } 

            while(NumOfPage--)
            {
                I2C_EE_PageWrite(pBuffer, WriteAddr, I2C_PageSize);
                I2C_EE_WaitEepromStandbyState();
                WriteAddr +=  I2C_PageSize;
                pBuffer += I2C_PageSize;  
            }
            if(NumOfSingle != 0)
            {
                I2C_EE_PageWrite(pBuffer, WriteAddr, NumOfSingle); 
                I2C_EE_WaitEepromStandbyState();
            }
        }
    }  
}

/**
  * @brief   写一个字节到I2C EEPROM中
  * @param   
  *     @arg pBuffer:缓冲区指针
  *     @arg WriteAddr:写地址 
  * @retval  无
  */
uint32_t I2C_EE_ByteWrite(u8* pBuffer, u8 WriteAddr)
{
    /* Send STRAT condition */
    I2C_GenerateSTART(eeprom.dev, ENABLE);

    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* Test on EV5 and clear it */
    while(!I2C_CheckEvent(eeprom.dev, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(0);
    }    

    /* Send EEPROM address for write */
    I2C_Send7bitAddress(eeprom.dev, eeprom.peeraddr, I2C_Direction_Transmitter);


    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* Test on EV6 and clear it */
    while(!I2C_CheckEvent(eeprom.dev, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(1);
    }    
      
    /* Send the EEPROM's internal address to write to */
    I2C_SendData(eeprom.dev, WriteAddr);

    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* Test on EV8 and clear it */
    while(!I2C_CheckEvent(eeprom.dev, I2C_EVENT_MASTER_BYTE_TRANSMITTED))  
    {
        if((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(2);
    } 
    /* Send the byte to be written */
    I2C_SendData(eeprom.dev, *pBuffer);

    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* Test on EV8 and clear it */
    while(!I2C_CheckEvent(eeprom.dev, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(3);
    }

    /* Send STOP condition */
    I2C_GenerateSTOP(eeprom.dev, ENABLE);

    return 1;
}

/**
  * @brief   在EEPROM的一个写循环中可以写多个字节，但一次写入的字节数
  *          不能超过EEPROM页的大小，AT24C02每页有8个字节
  * @param   
  *     @arg pBuffer:缓冲区指针
  *     @arg WriteAddr:写地址
  *     @arg NumByteToWrite:写的字节数
  * @retval  无
  */
uint32_t I2C_EE_PageWrite(u8* pBuffer, u8 WriteAddr, u8 NumByteToWrite)
{
    
    I2CTimeout = I2CT_LONG_TIMEOUT;
    while(I2C_GetFlagStatus(eeprom.dev, I2C_FLAG_BUSY))
    {
        if((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(4);
    } 

    /* Send START condition */
    I2C_GenerateSTART(eeprom.dev, ENABLE);


    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* Test on EV5 and clear it */
    while(!I2C_CheckEvent(eeprom.dev, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(5);
    } 

    /* Send EEPROM address for write */
    I2C_Send7bitAddress(eeprom.dev, eeprom.peeraddr, I2C_Direction_Transmitter);

    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* Test on EV6 and clear it */
    while(!I2C_CheckEvent(eeprom.dev, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(6);
    } 
    /* Send the EEPROM's internal address to write to */
    I2C_SendData(eeprom.dev, (WriteAddr>>8));

    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* Test on EV8 and clear it */
    while(! I2C_CheckEvent(eeprom.dev, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) 
    {
        if((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(7);
    } 
    {
		I2C_SendData(eeprom.dev, WriteAddr);

	    I2CTimeout = I2CT_FLAG_TIMEOUT;
	    /* Test on EV8 and clear it */
	    while(! I2C_CheckEvent(eeprom.dev, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) 
	    {
	        if((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(7);
	    } 
	}
    /* While there is data to be written */
    while(NumByteToWrite--)
    {
        /* Send the current byte */
        I2C_SendData(eeprom.dev, *pBuffer); 

        /* Point to the next byte to be written */
        pBuffer++; 

        I2CTimeout = I2CT_FLAG_TIMEOUT;
        /* Test on EV8 and clear it */
        while (!I2C_CheckEvent(eeprom.dev, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        {
            if((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(8);
        }
    }

    /* Send STOP condition */
    I2C_GenerateSTOP(eeprom.dev, ENABLE);

    return 1;
}

/**
  * @brief   从EEPROM里面读取一块数据 
  * @param   
  *     @arg pBuffer:存放从EEPROM读取的数据的缓冲区指针
  *     @arg WriteAddr:接收数据的EEPROM的地址
  *     @arg NumByteToWrite:要从EEPROM读取的字节数
  * @retval  无
  */
uint32_t I2C_EE_BufferRead(u8* pBuffer, u8 ReadAddr, u16 NumByteToRead)
{
    I2CTimeout = I2CT_LONG_TIMEOUT;
    //*((u8 *)0x4001080c) |=0x80; 
    while(I2C_GetFlagStatus(eeprom.dev, I2C_FLAG_BUSY))
    {
        if((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(9);
    }
    /* Send START condition */
    I2C_GenerateSTART(eeprom.dev, ENABLE);
    //*((u8 *)0x4001080c) &=~0x80;

    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* Test on EV5 and clear it */
    while(!I2C_CheckEvent(eeprom.dev, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(10);
    }

    /* Send EEPROM address for write */
    I2C_Send7bitAddress(eeprom.dev, eeprom.peeraddr, I2C_Direction_Transmitter);

    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* Test on EV6 and clear it */
    while(!I2C_CheckEvent(eeprom.dev, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) 
    {
        if((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(11);
    }
    /* Clear EV6 by setting again the PE bit */
    I2C_Cmd(eeprom.dev, ENABLE);

    /* Send the EEPROM's internal address to write to */
    I2C_SendData(eeprom.dev, ReadAddr>>8);

     I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* Test on EV8 and clear it */
    while(!I2C_CheckEvent(eeprom.dev, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(12);
    }
    {
		I2C_SendData(eeprom.dev, ReadAddr);

	    I2CTimeout = I2CT_FLAG_TIMEOUT;
	    /* Test on EV8 and clear it */
	    while(! I2C_CheckEvent(eeprom.dev, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) 
	    {
	        if((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(7);
	    } 
	}
    /* Send STRAT condition a second time */
    I2C_GenerateSTART(eeprom.dev, ENABLE);

     I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* Test on EV5 and clear it */
    while(!I2C_CheckEvent(eeprom.dev, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(13);
    }
    /* Send EEPROM address for read */
    I2C_Send7bitAddress(eeprom.dev, eeprom.peeraddr|1, I2C_Direction_Receiver);

     I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* Test on EV6 and clear it */
    while(!I2C_CheckEvent(eeprom.dev, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    {
        if((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(14);
    }
    /* While there is data to be read */
    while(NumByteToRead)  
    {
        if(NumByteToRead == 1)
        {
          /* Disable Acknowledgement */
          I2C_AcknowledgeConfig(eeprom.dev, DISABLE);
          
          /* Send STOP Condition */
          I2C_GenerateSTOP(eeprom.dev, ENABLE);
        }

        I2CTimeout = I2CT_LONG_TIMEOUT;
        while(I2C_CheckEvent(eeprom.dev, I2C_EVENT_MASTER_BYTE_RECEIVED)==0)  
        {
            if((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(3);
        }
        {
              /* Read a byte from the device */
              *pBuffer = I2C_ReceiveData(eeprom.dev);

              /* Point to the next location where the byte read will be saved */
              pBuffer++; 
              
              /* Decrement the read bytes counter */
              NumByteToRead--;
        }
    }

    /* Enable Acknowledgement to be ready for another reception */
    I2C_AcknowledgeConfig(eeprom.dev, ENABLE);

    return 1;
}

/**
  * @brief  Wait for EEPROM Standby state 
  * @param  无
  * @retval 无
  */
void I2C_EE_WaitEepromStandbyState(void)
{
  vu16 SR1_Tmp = 0;

    do
    {
        /* Send START condition */
        I2C_GenerateSTART(eeprom.dev, ENABLE);
        /* Read EEPROM_I2C SR1 register */
        SR1_Tmp = I2C_ReadRegister(eeprom.dev, I2C_Register_SR1);
        /* Send EEPROM address for write */
        I2C_Send7bitAddress(eeprom.dev, eeprom.peeraddr, I2C_Direction_Transmitter);
    }while(!(I2C_ReadRegister(eeprom.dev, I2C_Register_SR1) & 0x0002));
  
    /* Clear AF flag */
    I2C_ClearFlag(eeprom.dev, I2C_FLAG_AF);
    /* STOP condition */
    I2C_GenerateSTOP(eeprom.dev, ENABLE); 
}



#if TEST_ENABLE 

#define  EEP_Firstpage      0x00

/**
  * @brief  I2C(AT24C02)读写测试
  * @param  无
  * @retval 正常返回1 ，不正常返回0
  */
uint8_t eeprom_test(void)
{
    u16 i;
    u8 wr_buff[256] = {0};
    u8 rd_buff[256] = {0};

    TRACE_OUT(DEBUG_OUT, "write data to eeprom\r\n");
    
    for( i=0; i<=255; i++ ) //填充缓冲
    {
        wr_buff[i] = i;

        TRACE_OUT(DEBUG_OUT, "%02X ", wr_buff[i]);
        if(i%16 == 15)
            TRACE_OUT(DEBUG_OUT, "\r\n");
    }

    //将wr_buff中顺序递增的数据写入EERPOM中 
    I2C_EE_BufferWrite( wr_buff, EEP_Firstpage, 256);
  
    TRACE_OUT(DEBUG_OUT, "Write OK\r\n");
    TRACE_OUT(DEBUG_OUT, "Read data from eeprom\r\n");
    //将EEPROM读出数据顺序保持到rd_buff中
    I2C_EE_BufferRead(rd_buff, EEP_Firstpage, 256);
   
    //将rd_buff中的数据通过串口打印
    for (i=0; i<256; i++)
    {
        if(rd_buff[i] != wr_buff[i])
        {
            TRACE_OUT(DEBUG_OUT, "index[%d] val[0x%02X]\r\n", i, rd_buff[i]);
            TRACE_OUT(DEBUG_OUT, "EEPROM-ERROR: read not equal to write\r\n");
            return 0;
        }
        TRACE_OUT(DEBUG_OUT, "%02X ", rd_buff[i]);
        if(i%16 == 15)
            TRACE_OUT(DEBUG_OUT, "\r\n");
    }
    TRACE_OUT(DEBUG_OUT, "I2C(AT24C02) test successfully\r\n");
    return 1;
}


#endif 
/*********************************************END OF FILE**********************/
