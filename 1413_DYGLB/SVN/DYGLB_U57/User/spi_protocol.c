/*
***********************************************************************************************************************
    @brief          : 飞腾spi通信协议处理
    @author         : xiongjinqi
    @date           : 2024/07/25
***********************************************************************************************************************
*/
#include "types_def.h"
#include "alloc.h"
#include "ipmi_protocol.h"
#include "spi_protocol.h"

extern void reset_ipmi_index();
extern unsigned int lysTemp3,lysTemp4;
int cmd14SeniorId=0,cmd14TurnStatus=0,cmd14I2cAddr=0,cmd14TimeroutNum=0;
extern int cmd22RuningFlag;
extern int cmd22EndFirstEntercmd11or12;
_spi_interrupt_t spi_int = {
    .dev = SPI1,
};

_spi_t spi_protocol = {0};

_resp_buff_t respcmd11 = {0};
_resp_buff_t respcmd12 = {0};



/*
    @brief      : SPI中断服务函数
*/
#if 0
void SPI1_IRQHandler(void)
{
    volatile u8 temp = 0;
    if(SPI_I2S_GetITStatus(SPI1, SPI_I2S_IT_RXNE) == SET)
    {
        SPI_I2S_ClearITPendingBit(SPI1, SPI_I2S_IT_RXNE);
        temp = SPI_I2S_ReceiveData(SPI1);
        u8_ring_buffer_queue(&spi_protocol.spi->rb_handler, temp);    // 压入ringbuffer
    }
    
    #if (SPI_TRANS_USE_INT == 1)
    if(SPI_I2S_GetITStatus(SPI1, SPI_I2S_IT_TXE) == SET)
    {
        SPI_I2S_ClearITPendingBit(SPI1, SPI_I2S_IT_TXE);
        if(spi_protocol.spi->txindex < spi_protocol.spi->txcount)
            SPI_I2S_SendData(SPI1, spi_protocol.spi->txbuff[spi_protocol.spi->txindex++]);   // 发送数据
        else
        {
            memset(spi_protocol.spi->txbuff, 0, spi_protocol.spi->txcount); // add
            spi_protocol.spi->txindex = 0;
            spi_protocol.spi->txcount = 0;	// add
            SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_TXE, DISABLE);    // 关闭发送为空中断
            SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_RXNE, ENABLE);    // 打开接收非空中断
        }
    }
    #endif
}
#endif
 unsigned int lysspiEnterNum=0,lysRespCmd22Num=0;
static int spi_protocol_transmit( _spi_protocol_t *handler, _spi_interrupt_t *spi)
{
    u16 index = 0, data_len = 0;

    SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_RXNE, DISABLE);    // 关闭接收非空中断， 当接收缓冲区中有新数据可用时产生中断
    
    data_len = handler->length - SPI_HEADER_SIZE;
    memset(spi->txbuff, 0, handler->length+1);  // 由于经过CPLD,增加1个字节收尾,0X00
    
    spi->txbuff[index++] = handler->header;
    spi->txbuff[index++] = H8_GET(handler->length);
    spi->txbuff[index++] = L8_GET(handler->length);
    spi->txbuff[index++] = handler->cmd;
	if((handler->cmd)==SPI_CMD22)
	{
		lysspiEnterNum++;
		LYSprintf(LYSDEBUG3, "<<<<<<<	spi_protocol_transmit	==	%d	,	lysRespCmd22Num	==	%d		<<<<<<\r\n",lysspiEnterNum,lysRespCmd22Num);
	}
    if(data_len > 0)
    {
        memcpy(spi->txbuff+index, handler->data, data_len);
        index += data_len;
    }
    spi->txbuff[index++] = check_transmitsum(spi->txbuff+3, index-3); // 计算内容字段校验和
    spi->txcount = index+1;
    spi->txindex = 0;   // 从第一个字节开始发送

    #if (SPI_TRANS_USE_INT != 1)
    SPI1_DMA_Init((u32*)spi->txbuff, spi->txcount);     // 使用DMA发送数据
    
    #else
    SPI_I2S_SendData(spi->dev, 0);   // 发送数据
    delay_us(10);
    SPI_I2S_ITConfig(spi->dev, SPI_I2S_IT_TXE, ENABLE);    // 启用发送为空中断 ，当发送缓冲区为空且可以写入新的数据时产生中断。
    #endif
    
    TRACE_OUT(DEBUG_OUT,"======================================\r\n");
    TRACE_OUT(DEBUG_OUT, "SPI txcount[%d] checksum[%02x]\r\n", spi->txcount, spi->txbuff[index-1]);
    
    return 0;
}


u8 tx_buff[12] = {0};

void respcmd12_data_set(_resp_buff_t *buffer, u8 *data, u16 datalen)
{
    memcpy((void*)(buffer->buffer + buffer->index), data, datalen);
    buffer->index += datalen;
}

void respcmd11_data_set(_resp_buff_t *buffer)
{

}

static int do_respcmd11 (void *protocol_arg, u8 *buff, u16 len)
{
	
}

static int do_respcmd12 (void *protocol_arg, u8 *buff, u16 len)
{
	
}

/*
指令0x13：查询转接板默认启动上电 
*/
static int do_respcmd13 (void *protocol_arg, u8 *buff, u16 len)
{
    return 0;
}
void cmd22_runing_start3(void);

_time_t cmd22_timer = {0};
void cmd22TimeOut_callback(MultiTimer* timer, void* userData)
{
	cmd22RuningFlag=CMD22RUNEND;/*cmd22查询结束*/
	cmd22_runing_start3();
    softtimer_stop(timer);
}
void cmd22_runing_start(void)
{
	cmd22RuningFlag=CMD22RUNING;/*cmd22正在查询标志位*/

   	 softtimer_start(&cmd22_timer, 2000, cmd22TimeOut_callback, &cmd22_timer);
}
void cmd22_stop(void)
{
    softtimer_stop(&cmd22_timer);
}
_time_t cmd22_timer_2 = {0};
void cmd22TimeOut_callback2(MultiTimer* timer, void* userData)
{
	do_reqcmd14(&ipmi_zjb, cmd14I2cAddr, cmd14SeniorId, cmd14TurnStatus);

    softtimer_stop(timer);
}
void cmd22_runing_start2(void)
{
    softtimer_start(&cmd22_timer_2, 400, cmd22TimeOut_callback2, &cmd22_timer_2);
}
void cmd22_stop2(void)
{
    softtimer_stop(&cmd22_timer_2);
}

_time_t cmd22_timer_3 = {0};
void cmd22TimeOut_callback3(MultiTimer* timer, void* userData)
{
	cmd22EndFirstEntercmd11or12=0;//等待一段时间，复位标志位。
	reset_ipmi_index();
    softtimer_stop(timer);
}

void cmd22_runing_start3(void)
{
	cmd22EndFirstEntercmd11or12=1;//才结束cmd22查询
    softtimer_start(&cmd22_timer_3, 800, cmd22TimeOut_callback3, &cmd22_timer_3);
}
void cmd22_stop3(void)
{
    softtimer_stop(&cmd22_timer_3);
}

/******************************************************************************
 * 函 数 名：do_respcmd22
 *
 * 函数说明: 控制转接板上外设开关的，打开或关闭
 * 参数说明: 
 * 输入参数: 
 * 输出参数: 
 * 返 回 值: 
 * 
 * 修改记录：
 * --------------------
 *	 
 ******************************************************************************/
static int do_respcmd22 (void *protocol_arg, u8 *buff, u16 len)
{
    int ret = 0;
    u8 slaveaddr = 0;
    _spi_t *protocol = (_spi_t *)protocol_arg;
    _spi_protocol_t *handler = &protocol->response;
//	if(cmd22RuningFlag==CMD22RUNEND)
	{
//		/*清除i2c寄存器缓冲区和buffer*/
//		clear_I2C_recvie(&i2c1_int);		
//		clear_I2C_recvie(&i2c2_int);
		
		lysRespCmd22Num++;
		cmd22_runing_start();
		cmd22_runing_start2();
		
		handler->header = SPI_HEADER;
		handler->cmd = SPI_CMD22;
	
		protocol->status = PROCESS_CMD22;
		if(len == 2)
		{
			switch(buff[0])
			{
					case ID_ZK_TEMPRETURE:
					case ID_ZK_VDDQ:
					case ID_ZK_VDD_CORE:
					case ID_ZK_VCC1V8:
					case ID_ZK_VCC2V5:
					case ID_ZK_VCC3V3:
					case ID_ZK_VCC5V0:
					case ID_ZK_VPX12V:
					case ID_PWR_TEMPRETURE:
					case ID_PWR_VCC12V_1:
					case ID_PWR_VCC12V_2:
					case ID_PWR_VCC3V3:
					case ID_JKKZ_TEMPRETURE:
					case ID_JKKZ_VCC1V2:
					case ID_JKKZ_VCC1V5:
					case ID_JKKZ_VCC1V8:
					case ID_JKKZ_VCC1V0:
					case ID_JKKZ_VCC2V5:
					case ID_JKKZ_VCC3V3:
					case ID_JKKZ_VPX12V_CURR:
					case ID_XHCL_TEMPRETURE:
					case ID_XHCL_VCC1V2:
					case ID_XHCL_VCC1V5:
					case ID_XHCL_VCC1V8:
					case ID_XHCL_VCC1V0:
					case ID_XHCL_VCC3V3:
					case ID_XHCL_SLOT_ADDR:	
					case ID_XHCL_VPX12V_CURR:
					case ID_ZJB_TEMPRETURE:
					case ID_ZJB_HALL_GSDJ1:
					case ID_ZJB_HALL_GSDJ2:
					case ID_ZJB_SLOT_ADDR:
						TRACE_OUT(DEBUG_OUT, "Can't control SENSORID[%02x]\r\n", buff[0]);
					break;
		
					case ID_ZJB_FAN1:
					case ID_ZJB_FAN2:
					case ID_ZJB_FAN3:
					case ID_ZJB_FAN4:
						if(ipmi_zjb.ipmi->i2cbus->dev == I2C1)
							slaveaddr = ipmi_zjb.i2ca_addr;
						if(ipmi_zjb.ipmi->i2cbus->dev == I2C2)
							slaveaddr = ipmi_zjb.i2cb_addr;
						do_reqcmd15(&ipmi_zjb, slaveaddr, buff[0], buff[1]);
					break;
		
					case ID_ZJB_28V_JCXJ:
					case ID_ZJB_12V_WAOXJ: 
					case ID_ZJB_28V_BQXJ:  
					case ID_ZJB_12V_DYGY_B: 
					case ID_ZJB_28V_FFXJ:	
					case ID_ZJB_12V28V_BF1: 
					case ID_ZJB_12V28V_BF2: 
					case ID_ZJB_28V_TSGY:	
					case ID_ZJB_28V_QGSJ:	
					case ID_ZJB_12V28V_HJJC1:
					case ID_ZJB_12V28V_HJJC2:
					case ID_ZJB_12V28V_HJJC3:
					case ID_ZJB_12V_GSDJ1:	
					case ID_ZJB_12V_GSDJ2:	
					case ID_ZJB_12V_XGHTM:	
					case ID_ZJB_28V_KF:
						if(ipmi_zjb.ipmi->i2cbus->dev == I2C1)
							slaveaddr = ipmi_zjb.i2ca_addr;
						if(ipmi_zjb.ipmi->i2cbus->dev == I2C2)
							slaveaddr = ipmi_zjb.i2cb_addr;
						do_reqcmd14(&ipmi_zjb, slaveaddr, buff[0], buff[1]);
						cmd14SeniorId=buff[0];
						cmd14TurnStatus=buff[1];
						cmd14I2cAddr=slaveaddr;
						cmd14TimeroutNum++; 		
					break;			  
					default:
						TRACE_OUT(DEBUG_OUT, "Line%d : invaild param[%02x]\r\n", __LINE__, buff[0]);
					break;
				}
		}
		// 由于发送控制指令后，需要读取响应的数据，因此发送函数被 ipmi协议调用
#if 0
		handler->data[index++] = buff[0];
		handler->data[index++] = 0;
		handler->data[index++] = 0;
		handler->data[index++] = 0x0B;	// 仿造数据，转速3000
		handler->data[index++] = 0xB8;
		handler->length = index+SPI_HEADER_SIZE;
#endif
			// todo: 发送数据
			//spi_protocol_transmit(handler, protocol->spi);
			ipmi_handler_update(&ipmi_zjb);
		}
		return ret;
}
		
		/*
			@brief		: 切换总线指令
		*/
static int do_respcmd23 (void *protocol_arg, u8 *buff, u16 len)
{
    int index = 0, ret = 0;

    _spi_t *protocol = (_spi_t *)protocol_arg;
    _spi_protocol_t *handler = &protocol->response;
    handler->header = SPI_HEADER;
    handler->cmd = SPI_CMD23;

    if(len == 1)
    {
        if(buff[0] == IPMI_BUSA)
        {
            for(; index<ipmi_app.sub_count; index++)
                ipmi_app.ipmi_arry[index]->ipmi = &ipmi_i2ca;
        }
        
        if(buff[0] == IPMI_BUSB)
        {
            for(; index<ipmi_app.sub_count; index++)
                ipmi_app.ipmi_arry[index]->ipmi = &ipmi_i2cb;
        }
    }
    
#if 1
    handler->data[0] = 0;
    handler->length = 1+SPI_HEADER_SIZE;
#endif
    // todo: 发送数据
    spi_protocol_transmit(handler, protocol->spi);
	
    return ret;
}

/*默认启动上电*/
static int do_respcmd25 (void *protocol_arg, u8 *buff, u16 len)
{
    int ret = 0;
    u8 slaveaddr = 0;
	int i=0;
    _spi_t *protocol = (_spi_t *)protocol_arg;
    _spi_protocol_t *handler = &protocol->response;
    
    protocol->status = PROCESS_CMD25;
    
    //下发ipmi指令
    if(ipmi_zjb.ipmi->i2cbus->dev == I2C1)
    {
        slaveaddr = ipmi_zjb.i2ca_addr;
    }
    if(ipmi_zjb.ipmi->i2cbus->dev == I2C2)
    {
        slaveaddr = ipmi_zjb.i2cb_addr;
    }
	TRACE_OUT(DEBUG_OUT, "*********len==%d********\r\n",len);
	for(i=0;i<32;i=i+2)
	{
		TRACE_OUT(DEBUG_OUT, "*********devID==0x%x,buff=%d********\r\n",buff[i],buff[i+1]);
	}
	
    if(len%2 == 0)
    {
        do_reqcmd17(&ipmi_zjb, slaveaddr, buff, len);
    }
    ipmi_handler_update(&ipmi_zjb);
#if 0
    handler->header = SPI_HEADER;
    handler->cmd    = SPI_CMD25;
    handler->data[0]= 0;   // 执行成功
    handler->length = 1+SPI_HEADER_SIZE;
    ret = spi_protocol_transmit(handler, protocol->spi);
#endif
    return ret;
}


_do_data_t do_spicmd[] = {
    {SPI_CMD11, do_respcmd11},
    {SPI_CMD12, do_respcmd12},
    {SPI_CMD13, do_respcmd13},
    {SPI_CMD22, do_respcmd22},
    {SPI_CMD23, do_respcmd23},
    {SPI_CMD25, do_respcmd25}
};


#if CODE_PART("指令响应接口2（ipmi调用）")

/* 
    @brief      : cmd12指令发送函数，在ipmi协议处理中调用
    @param[in]  : 
            handler         spi数据处理句柄
            buff            数据首地址
            len             数据长度
    @param[out] : none
    @retval     : 检验和
*/
//int do_respcmd12_transmit (_spi_t *handler, u8 *buff, u16 len)
//{
//    int ret = 0;
//
//    handler->response.header = SPI_HEADER;
//    handler->response.cmd    = SPI_CMD12;
//
//    // 填充数据域
//    if(len == 1)
//    {
//        memcpy(handler->response.data, buff, len);
//        handler->response.length = len+SPI_HEADER_SIZE;
//    }
//    else
//    {
//        ret = -1;
//    }
//
//    if(handler->status == PROCESS_CMD12) // 处于cmd22,发送数据
//    {
//        handler->status = 0;
//        ret = spi_protocol_transmit(&handler->response, handler->spi);
//    }
//    
//    return ret;
//}

/* 
    @brief      : cmd13指令发送函数，在ipmi协议处理中调用
    @param[in]  : 
            handler         spi数据处理句柄
            buff            数据首地址
            len             数据长度
    @param[out] : none
    @retval     : 检验和
*/
int do_respcmd13_transmit (_spi_t *handler, u8 *buff, u16 len)
{
    int ret = 0;

    handler->response.header = SPI_HEADER;
    handler->response.cmd    = SPI_CMD13;

    // 填充数据域
    if(len)
    {
        memcpy(handler->response.data, buff, len);
        handler->response.length = len+SPI_HEADER_SIZE;
    }
    else
    {
        ret = -1;
    }

    if(handler->status == PROCESS_CMD13) // 处于cmd22,发送数据
    {
        handler->status = 0;
        ret = spi_protocol_transmit(&handler->response, handler->spi);
    }
    
    return ret;
}

/* 
    @brief      : cmd22指令发送函数，在ipmi协议处理中调用
    @param[in]  : 
            handler         spi数据处理句柄
            buff            数据首地址
            len             数据长度
    @param[out] : none
    @retval     : 检验和
*/
int do_respcmd22_transmit (_spi_t *handler, u8 *buff, u16 len)
{
    int ret = 0, index = 0;
    u16 voltage = 0;
    int temp=0;
	static unsigned int enterNum=0;
    handler->response.header = SPI_HEADER;
    handler->response.cmd    = SPI_CMD22;
	enterNum++;

    // 填充数据域
    if((buff[0] >= ID_ZJB_FAN1) && (buff[0] <= ID_ZJB_FAN4))
    {
        handler->response.data[index++] = buff[0];    // sensor id
        handler->response.data[index++] = 1; // status
        handler->response.data[index++] = 0;
        handler->response.data[index++] = 0;
        handler->response.data[index++] = buff[1];
        handler->response.data[index++] = buff[2];
        handler->response.length = 3+len+SPI_HEADER_SIZE;
    }
    else
    {
        handler->response.data[index++] = buff[0];    // sensor id
        
        voltage = U8_TO_U16(buff[1], buff[2]);
		TRACE_OUT(DEBUG_OUT, "<<<	voltage	==	%d	<<<<",voltage);
        if(voltage > 300)       // 电压大于1.5V认为电源已经打开 lys:改成了大于3V判断为电源已打开，因为下电过程，电压缓慢下降，回传的电压还是下降过程中的
        {   
        	handler->response.data[index++] = 1; // status
        	temp=1;
        }
        else
        {
            handler->response.data[index++] = 0;
			temp=0;
        }
		/*如果实际值和理论值不一样，就再发一次，开关机指令*/
       if(temp!=cmd14TurnStatus)
       {
		   u8_ring_buffer_init(&i2c1_int.rb_handler, (char *)i2c1_int.rxbuff, IIC_DATA_SIZE);
		   u8_ring_buffer_init(&i2c2_int.rb_handler, (char *)i2c2_int.rxbuff, IIC_DATA_SIZE);
		   do_reqcmd14(&ipmi_zjb, cmd14I2cAddr, cmd14SeniorId, cmd14TurnStatus);
	   }
	   LYSprintf(LYSDEBUG4, "<<<<<<<	want==%d	,	truth==%d	,	enterNum=%d<<<<<<\r\n",temp,cmd14TurnStatus,enterNum);
       memcpy(handler->response.data+index, buff+1, len-1);
       handler->response.length = 1+len+SPI_HEADER_SIZE;
    }
    
    if(handler->status == PROCESS_CMD22) // 处于cmd22,发送数据
    {
        handler->status = 0;
        ret = spi_protocol_transmit(&handler->response, handler->spi);

		cmd22RuningFlag=CMD22RUNEND;/*cmd22查询结束*/
		cmd22_stop();
		cmd22_stop2();
		cmd22_runing_start3();
    }
    
    return ret;
}


/* 
    @brief      : cmd25指令发送函数，在ipmi协议处理中调用
    @param[in]  : 
            handler         spi数据处理句柄
            buff            数据首地址
            len             数据长度
    @param[out] : none
    @retval     : 检验和
*/
int do_respcmd25_transmit (_spi_t *handler, u8 *buff, u16 len)
{
    int ret = 0;
    handler->response.header = SPI_HEADER;
    handler->response.cmd    = SPI_CMD25;

    // 填充数据域
    if(len == 1)
    {
        memcpy(handler->response.data, buff, len);
        handler->response.length = len+SPI_HEADER_SIZE;
    }
    else
    {
        ret = -1;
    }

    if(handler->status == PROCESS_CMD25) // 处于cmd22,发送数据
    {
        handler->status = 0;
        ret = spi_protocol_transmit(&handler->response, handler->spi);
    }
    
    return ret;
}

#endif /* 指令响应接口2（ipmi调用） */


#if CODE_PART("spi协议栈对外接口")

/* 
    @brief      : 指令分发
    @param[in]  : 
            protocol_arg    spi数据处理句柄
            buff            数据场首地址
            len             数据场长度
    @param[out] : none
    @retval     : 检验和
*/
int do_spicmd_dispatch(void *protocol_arg, u8 *buff, u16 len)
{
    _spi_t *handler = (_spi_t*)protocol_arg;
    int index = 0, ret = 0;
    u8 recv_checksum = 0, calc_checksum = 0;
    
    /* 1、检验数据 */
    recv_checksum = buff[len-1];
    calc_checksum = check_recvsum(buff+3, len-3);
    if(recv_checksum == calc_checksum)
    {
        //recv_len = U8_TO_U16(buff[1], buff[2]); // 获取长度
        handler->response.header = buff[0];
        handler->response.cmd    = buff[3];
		TRACE_OUT(DEBUG_OUT, "*********buff[0]_header=0x%x,buff[3]_cmd=0x%x********\r\n", buff[0],buff[3]);

        /* 3、处理指令,并发送响应 */
        for(index = 0; index < ARRAY_SIZE(do_spicmd); index++)
        {
            if(do_spicmd[index].cmd == handler->response.cmd)
            {
            	TRACE_OUT(DEBUG_OUT, "*********buff[0]_header=0x%x,buff[3]_cmd=0x%x,index=%d,len=%d********\r\n", buff[0],buff[3],index,len);
                ret = do_spicmd[index].do_data(handler, buff+4, len-5); // 4字节是数据域的头
            }
        }
    }
    else
    {
        TRACE_OUT(DEBUG_OUT, "%s Line%d: Check sum fail, CALC[%02x] != FRAM[%02x]\r\n", \
                    __FUNCTION__, __LINE__, calc_checksum, recv_checksum);
    }
    return ret;
}


/* 
    @brief      : 对上位机通过spi发过来的指令进行响应，通过do_spicmd_dispatch来解析执行。
    @param[in]  : 
            handler         can协议句柄指针
    @param[out] : none
    @retval     : 检验和
*/
static int spi_recv_deal(_spi_t *handler)
{
    int ret = 0, recv_len = 0;
    u8 temp = 0, len_h = 0, len_l = 0;
    u8 buff[256] = {0};
//	if(cmd22RuningFlag==CMD22RUNEND)
    {
	    if(u8_ring_buffer_num_items(&handler->spi->rb_handler) > 1)
	    {
	        ret = u8_ring_buffer_peek(&handler->spi->rb_handler, (char *)&temp, 0);
	        if(temp == SPI_HEADER)
	        {
	            ret = u8_ring_buffer_peek(&handler->spi->rb_handler, (char *)&len_h, 1); // 获取当前帧长度
	            ret = u8_ring_buffer_peek(&handler->spi->rb_handler, (char *)&len_l, 2); 
	            if(ret == 1)
	            {
	                recv_len = U8_TO_U16(len_h, len_l);
	                if(u8_ring_buffer_num_items(&handler->spi->rb_handler) >= recv_len) // 接收满一帧数据
	                {
	                    ret = u8_ring_buffer_dequeue_arr(&handler->spi->rb_handler, (char *)buff, recv_len);
	                    if(ret != recv_len)
	                    {
	                        TRACE_OUT(DEBUG_OUT, "Read ring buffer fail \r\n");
	                    }
	                    else
	                    {
	                        ret = handler->do_cmd(handler, buff, recv_len);//handler->do_cmd==do_spicmd_dispatch
	                        TRACE_OUT(DEBUG_OUT, "*********recv_len=%d********\r\n", recv_len);
	                        
	                    }
						 TRACE_OUT(DEBUG_OUT, "recv_len  \r\n");
	                }
	            }
	        }
	        else
	        {
	            /* 出栈，脏数据 */
	            u8_ring_buffer_dequeue(&handler->spi->rb_handler, (char *)&temp);
	        }

	    }
    }
    return ret;
}




/*
    @brief      : SPI协议栈初始化
*/
void spi_protocol_init(void)
{
    SPI1_Init();

    spi_protocol.spi = (_spi_interrupt_t*)&spi_int;
    spi_protocol.do_cmd = do_spicmd_dispatch;
    
    u8_ring_buffer_init(&spi_protocol.spi->rb_handler, (char *)spi_protocol.spi->rxbuff, SPI_DATA_SIZE);

    respcmd11.buffer = (u8*)wjq_malloc(SPI_DATA_SIZE);
    respcmd12.buffer = (u8*)wjq_malloc(SPI_DATA_SIZE);
}

/*
    @brief      : SPI协议栈初始化
*/
void spi_protocol_close(void)
{
    wjq_free(respcmd11.buffer);
    wjq_free(respcmd12.buffer);
}

/*
    @brief      : spi协议栈处理
*/
int spi_protocol_loop(void)
{
    int ret = 0;
    ret = spi_recv_deal(&spi_protocol);
    return ret;
}

#endif

