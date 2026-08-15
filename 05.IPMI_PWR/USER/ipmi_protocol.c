/*
***********************************************************************************************************************
    @brief          : ipmi通信协议处理
    @author         : xiongjinqi
    @date           : 2024/07/25
***********************************************************************************************************************
*/
#include "types_def.h"
#include "periph_i2c.h"
#include "main.h"
#include "ipmi_protocol.h"

_protocol_t ipmi_i2ca = {0};
_protocol_t ipmi_i2cb = {0};

/*
    @brief      : 计算接收校验和
    @param[in]  :
            buff        数据场首地址
            len         数据场长度
    @param[out] : none
    @retval     : 检验和
*/
static u8 check_recvsum(u8 *buff, u8 len)
{
    u8 sum = 0, index = 0;
    for (index = 0; index < len - 1; index++) // 排除数据场最后一个字节（检验和）
    {
        sum += buff[index];
    }

    return sum;
}

/*
    @brief      : 计算发送校验和
    @param[in]  :
            buff        数据场首地址
            len         数据场长度
    @param[out] : none
    @retval     : 检验和
*/
static u8 check_transmitsum(u8 *buff, u8 len)
{
    u8 sum = 0, index = 0;
    for (index = 0; index < len; index++) // 排除数据场最后一个字节（检验和）
    {
        sum += buff[index];
    }

    return sum;
}

static int ipmi_transmit(_protocol_t *protocol)
{
    u16 index = 0, datalen = 0;
    u8 txbuff[IIC_DATA_SIZE] = {0};

    datalen = protocol->response.frame_len - RESPONSE_HEADER_SIZE;

    txbuff[index++] = protocol->response.netfn;
    txbuff[index++] = protocol->response.frame_len;
    txbuff[index++] = protocol->response.src_addr;
    txbuff[index++] = protocol->response.req_num;
    txbuff[index++] = protocol->response.cmd;
    txbuff[index++] = protocol->response.is_complete;
    if (datalen)
    {
        memcpy(txbuff + index, protocol->response.data, protocol->response.frame_len - RESPONSE_HEADER_SIZE);
        index += datalen;
    }
    txbuff[index++] = check_transmitsum(txbuff + 2, index - 2); // 计算内容字段校验和

    delay_ms(1); // 延时200ms,让master转换为slave
    /* 发送获取到的目的地址 */
    i2c_master_send_bytes(protocol->i2cbus, protocol->response.dest_addr, txbuff, index);
    return 0;
}

#if 1
// DECLARED_GPIO_APP_SET(XC388_EN);
#if 0
static int do_cmd14 (void *protocol_arg, u8 *buff, u16 len)
{
    _protocol_t *protocol = (_protocol_t*)protocol_arg;
    int ret = 0;
    u16 adcval = 0, current = 0;
    if(buff[0] == VSO_ORGANIZATION)
    {
        switch(buff[1])
        {
            case ID_VCC12V_1:
            case ID_VCC12V_2:
            case ID_VCC3V3:
                TRACE_OUT(DEBUG_OUT, "%s POWER[%02x] can't control\r\n", __FUNCTION__, buff[1]);
            break;
            case ID_VPX12V_CURR:
                TRACE_OUT(DEBUG_OUT, "%s POWER[%02x] control %s\r\n", __FUNCTION__, buff[1], buff[2]?"ON":"OFF");
                
                ret     = gpio_setting(&app_XC388_EN, buff[2]?LED_OFF:LED_ON);
                adcval  = adcval_get(XCA4001_OUT_Channel);
                current = RAW_TO_VOLTAGE(adcval)*2*10;
                /* 填充data字段 */
                protocol->response.is_complete = (ret==0)?1:0;
                protocol->response.data[0] = VSO_ORGANIZATION;
                protocol->response.data[1] = current;
                protocol->response.frame_len = 2+RESPONSE_HEADER_SIZE;

                /* 发送响应 */
                // TODO: 
            break;
            default :
                TRACE_OUT(DEBUG_OUT, "%s invaild param[%02x]\r\n", __FUNCTION__, buff[1]);
            break;
        }
    }
    return ret;
}
#endif
static int do_cmd20(void *protocol_arg, u8 *buff, u16 len)
{
    _protocol_t *protocol = (_protocol_t *)protocol_arg;
    int ret = 0;
    u16 adcval = 0, current = 0;
    if (len == 0)
    {
        /* 填充data字段 */
        protocol->response.is_complete = 1;
        protocol->response.data[0] = IPMI_SENSOR_NUM;
        protocol->response.data[1] = 0;
        protocol->response.frame_len = 2 + RESPONSE_HEADER_SIZE;

        /* 发送响应 */
        // TODO: 
        ipmi_transmit(protocol);
    }
    return ret;
}

#define SET_CMD19_VAL(name, abnormal, buff, index) \
    do{ \
        buff[index++] = abnormal.AB_ID_PWR_##name; \
        buff[index++] = abnormal.AB_PWR_##name; \
    }while(0)

static int do_cmd19 (void *protocol_arg, u8 *buff, u16 len)
{
    _protocol_t *protocol = (_protocol_t*)protocol_arg;
    int ret = 0, index = 0;
    u16 copy_len = 0;

    memset(protocol->response.data, 0, IPMI_DATA_SIZE);
    if(buff[0] == VSO_ORGANIZATION)
    {
        protocol->response.data[index++] = buff[0];
        SET_CMD19_VAL(EEPROM, pwr_abnormal, protocol->response.data, index);
        SET_CMD19_VAL(TEMPRETURE, pwr_abnormal, protocol->response.data, index);
        SET_CMD19_VAL(VCC12V_1, pwr_abnormal, protocol->response.data, index);
        SET_CMD19_VAL(VCC12V_2, pwr_abnormal, protocol->response.data, index);
        SET_CMD19_VAL(VCC3V3, pwr_abnormal, protocol->response.data, index);
    }
    protocol->response.is_complete = 1;
    protocol->response.frame_len = index+RESPONSE_HEADER_SIZE;
     /* 发送响应 */
    ret = ipmi_transmit(protocol);
    return ret;
}

static int do_cmd2d (void *protocol_arg, u8 *buff, u16 len)
{
    _protocol_t *protocol = (_protocol_t*)protocol_arg;
    int ret = 0, index = 0;
    u16 voltage = 0, current = 0, status = 0;
    s16 temp = 0;
    if(len == 1)
    {
        switch(buff[0])
        {
            case ID_VCC12V_1:
                voltage = sysmon.vcc12v_1;
                current = sysmon.vcc12v_1_curr;
                status = CMD2D_VOL_STATUS(1)|CMD2D_CURR_STATUS(1)|CMD2D_ONOFF_STATUS(1);
            break;
            case ID_VCC12V_2:
                voltage = sysmon.vcc12v_2;
                current = sysmon.vcc12v_2_curr;
                status = CMD2D_VOL_STATUS(1)|CMD2D_CURR_STATUS(1)|CMD2D_ONOFF_STATUS(1);
            break;
            case ID_TEMPRETURE:
                temp = sysmon.tempreture;
                status = CMD2D_CURR_STATUS(1)|CMD2D_ONOFF_STATUS(1);
            break;
            case ID_VCC3V3:
                voltage = sysmon.vcc3v3;
                current = sysmon.vcc3v3_curr;
                status = CMD2D_VOL_STATUS(1)|CMD2D_CURR_STATUS(1)|CMD2D_ONOFF_STATUS(1);
            break;
            default:
                TRACE_OUT(DEBUG_OUT, "%s invaild param[%02x]\r\n", __FUNCTION__, buff[0]);
            break;
        }

        /* 填充data字段 */
        protocol->response.is_complete = 1;
        protocol->response.data[index++] = buff[0];
        if(buff[0] == ID_TEMPRETURE)
        {
            protocol->response.data[index++] = 0;
            protocol->response.data[index++] = 0;
            protocol->response.data[index++] = (temp>>8)&0xFF;
            protocol->response.data[index++] = temp&0xFF;
            protocol->response.data[index++] = status;
        }
        else
        {
            protocol->response.data[index++] = (voltage>>8)&0xFF;
            protocol->response.data[index++] = voltage&0xFF;
            protocol->response.data[index++] = (current>>8)&0xFF;
            protocol->response.data[index++] = current&0xFF;
            protocol->response.data[index++] = status;
        }

        protocol->response.frame_len = index + RESPONSE_HEADER_SIZE;
        /* 发送响应 */
        // TODO: 
        ipmi_transmit(protocol);
    }
    return ret;
}


_do_data_t do_cmd[] = {
    {IPMI_CMD14, NULL},
    {IPMI_CMD15, NULL},
    {IPMI_CMD16, NULL},
    {IPMI_CMD19, do_cmd19},
    {IPMI_CMD20, do_cmd20},
    {IPMI_CMD2D, do_cmd2d},
};

/*
    @brief      : 计算发送校验和
    @param[in]  : 
            cmd         接收到仲裁场、控制场数据
            buff        数据场首地址
            len         数据场长度
    @param[out] : none
    @retval     : 检验和

*/
int do_cmd_dispatch(void *protocol_arg, u8 *buff, u16 len)
{
    _protocol_t *protocol = (_protocol_t*)protocol_arg;
    int index = 0, ret = 0;
    u8 recv_checksum = 0, calc_checksum = 0;
    /* 1、检验数据 */
    recv_checksum = buff[len-1];
    calc_checksum = check_recvsum(buff+2, len-2);
    if(recv_checksum == calc_checksum)
    {
        /* 2、填充响应数据 */
        if(buff[2] == IPMI_MASTER_I2CA_ID)
            TRACE_OUT(DEBUG_OUT, "request from I2CA-BUS[%02x]\r\n", buff[2]);
        if(buff[2] == IPMI_MASTER_I2CB_ID)
            TRACE_OUT(DEBUG_OUT, "request from I2CB-BUS[%02x]\r\n", buff[2]);
        
        protocol->response.dest_addr = buff[2];     // 获取要发送的目的地址
        protocol->response.netfn     = buff[0];
        protocol->response.src_addr  = protocol->i2cbus->selfaddr;
        protocol->response.req_num   = buff[3];
        protocol->response.cmd       = buff[4];
        
        /* 3、处理指令,并发送响应 */
        for(index = 0; index < ARRAY_SIZE(do_cmd); index++)
        {
            if(do_cmd[index].cmd == protocol->response.cmd)
                ret = do_cmd[index].do_data(protocol, buff+5, len-REQUEST_HEADER_SIZE);
        }
    }
    else
    {
        TRACE_OUT(DEBUG_OUT, "Check sum fail, CALC[%02x] != FRAM[%02x]\r\n", calc_checksum, recv_checksum);
    }
    return ret;
}

/* 
    @brief      : can数据处理
    @param[in]  : 
            handler         can协议句柄指针
    @param[out] : none
    @retval     : 检验和
*/
int ipmi_recv_deal(_protocol_t *protocol)
{
    int ret = 0, recv_len = 0;
    u8 temp = 0, temp1 = 0;
    u8 buff[256] = {0};

    // if(protocol->i2cbus->status == I2C_RX_DONE)
    //{
    //     protocol->i2cbus->status = I2C_RX_START;
    ret = u8_ring_buffer_peek(&protocol->i2cbus->rb_handler, (char *)&temp, 0);
    ret = u8_ring_buffer_peek(&protocol->i2cbus->rb_handler, (char *)&temp1, 1); // 获取当前帧长度
    if (ret == 1)
    {
        recv_len = temp1;
        if (u8_ring_buffer_num_items(&protocol->i2cbus->rb_handler) >= recv_len) // 接收满一帧数据
        {
            ret = u8_ring_buffer_dequeue_arr(&protocol->i2cbus->rb_handler, (char *)buff, recv_len);
            if (ret != recv_len)
            {
                TRACE_OUT(DEBUG_OUT, "Read ring buffer fail \r\n");
            }
            else
            {
                #if 1
                for(u8 num = 0;num<recv_len;num++)
                {
                    TRACE_OUT(DEBUG_OUT, "IPMI RECV buff[%d]:%x ",num,buff[num]);
                }
                TRACE_OUT(DEBUG_OUT, "\r\n");
                #endif
                ret = protocol->do_cmd(protocol, buff, recv_len);
            }
        }
    }
    //}
    return ret;
}
#endif

/*
    @brief      : IPMI协议栈初始化
*/
void ipmi_protocol_init(void)
{
    #if 0
    i2c_all_init();     /* i2c初始化 */
    #endif
    
    ipmi_i2ca.i2cbus = &i2c1_int;
    ipmi_i2cb.i2cbus = &i2c2_int;

    ipmi_i2ca.do_cmd = do_cmd_dispatch;
    ipmi_i2cb.do_cmd = do_cmd_dispatch;
}


/*
    @brief      : 判断i2c中断句柄是否接收完成,
        @note   : 用于测试I2C通信
*/
void ipmi_is_rxdone(_i2c_interrpt_t *handler)
{
    u8 len = 0;
    u8 buff[256] = {0}, txbuff[256] = {0};
    if(handler->status == I2C_RX_DONE)
    {
        handler->status == I2C_RX_START;
        len = u8_ring_buffer_num_items(&handler->rb_handler);
        if(len >= 8)
        {
            u8_ring_buffer_dequeue_arr(&handler->rb_handler, (char *)buff, len);
            
            TRACE_OUT(DEBUG_OUT, "RX DONE =========================================\r\n");
            TRACE_OUT(DEBUG_OUT, "%p : data \r\n", handler);
            for(int i = 0; i<len; i++)
            {
                TRACE_OUT(DEBUG_OUT,"%02x ", buff[i]);
                if(i%16 == 15)
                    TRACE_OUT(DEBUG_OUT, "\r\n");
            }
            TRACE_OUT(DEBUG_OUT, "\r\n");

            i2c_master_send_bytes(handler, IPMI_MASTER_I2CA_ID, "8765321", 8);
        }
    }
#if 1
    if(handler->status == I2C_TX_DONE)
    {
        handler->status = I2C_RX_START;
        TRACE_OUT(DEBUG_OUT, "TX DONE =========================================\r\n");
    }

#endif 
}

/*
    @brief      : ipmi协议循环
*/
void ipmi_loop(void)
{
    // ipmi_is_rxdone(&i2c1_int);
    ipmi_recv_deal(&ipmi_i2ca);
    ipmi_recv_deal(&ipmi_i2cb);
}
