/*
***********************************************************************************************************************
    @brief          : ipmi通信协议处理
    @author         : xiongjinqi
    @date           : 2024/07/25
***********************************************************************************************************************
*/
#include "types_def.h"
#include "periph_i2c.h"
#include "spi_protocol.h"
#include "ipmi_protocol.h"

extern unsigned int lysTemp3,lysTemp4,lyscxcmd11all,lyscxcmd12all;
extern int cmd22RuningFlag;
extern int cmd22EndFirstEntercmd11or12;
u32 resetIpmiIndexFlag=0;

_protocol_t ipmi_i2ca = {0};
_protocol_t ipmi_i2cb = {0};


void ipmi_sensortimer_start(_ipmi_sensor_t *sensor);
void ipmi_sensortimer_stop(_ipmi_sensor_t *sensor);

/*
    @brief      : 计算接收校验和
    @param[in]  : 
            buff        数据场首地址
            len         数据场长度
    @param[out] : none
    @retval     : 检验和
*/
u8 check_recvsum(u8 *buff, u8 len)
{
    u8 sum = 0, index = 0;
    for(index = 0; index<len-1; index++) // 排除数据场最后一个字节（检验和）
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
u8 check_transmitsum(u8 *buff, u8 len)
{
    u8 sum = 0, index = 0;
    for(index = 0; index<len; index++) // 排除数据场最后一个字节（检验和）
    {
        sum += buff[index];
    }

    return sum;
}


static int ipmi_request_transmit(_protocol_t *protocol)
{
    u32 ret = 0;
    u16 index = 0, data_len = 0;
    u8 txbuff[IIC_DATA_SIZE] = {0};
    
    data_len = protocol->request.frame_len-REQUEST_HEADER_SIZE;
    
    txbuff[index++] = protocol->request.netfn;
    txbuff[index++] = protocol->request.frame_len;
    txbuff[index++] = protocol->request.src_addr;
    txbuff[index++] = protocol->request.req_num;
    txbuff[index++] = protocol->request.cmd;
    if(data_len > 0)
    {
        memcpy(txbuff+index, protocol->request.data, data_len);
        index += data_len;
    }
    txbuff[index++] = check_transmitsum(txbuff+2, index-2); // 计算内容字段校验和
    
    /* 发送获取到的目的地址 */
    ret = i2c_master_send_bytes(protocol->i2cbus, protocol->request.dest_addr, txbuff, index);
    
    return ret;

}

#if CODE_PART("响应数据处理")
DECLARED_GPIO_APP_SET(XC388_EN);

static int do_respcmd14 (void *protocol_arg, u8 *buff, u16 len)
{
    _ipmi_sensor_t *sensor = (_ipmi_sensor_t*)protocol_arg;
    int ret = 0;
    //u16 adcval = 0, current = 0;
    _zjb_monitor_t *board = (_zjb_monitor_t *)&ipmi_data.zjb;
    if(len == 6)
    {
        if(buff[0] == VSO_ORGANIZATION)
        {
            sensor->status = IPMI_CMD_OK;
            switch(buff[1])
            {
                case ID_ZJB_28V_JCXJ:
                    GET_ZJB_CMD14VAL(28V_JCXJ, board, buff);
                break;
                case ID_ZJB_12V_WAOXJ:
                    GET_ZJB_CMD14VAL(12V_WAOXJ, board, buff);
                break;
                case ID_ZJB_28V_BQXJ:
                    GET_ZJB_CMD14VAL(28V_BQXJ, board, buff);
                break;
                case ID_ZJB_12V_DYGY_B:
                    GET_ZJB_CMD14VAL(12V_DYGY_B, board, buff);
                break;
                case ID_ZJB_28V_FFXJ:
                    GET_ZJB_CMD14VAL(28V_FFXJ, board, buff);
                break;
                case ID_ZJB_12V28V_BF1:
                    GET_ZJB_CMD14VAL(12V28V_BF1, board, buff);
                break;
                case ID_ZJB_12V28V_BF2:
                    GET_ZJB_CMD14VAL(12V28V_BF2, board, buff);
                break;
                case ID_ZJB_28V_TSGY:
                    GET_ZJB_CMD14VAL(12V28V_BF2, board, buff);
                break;
                case ID_ZJB_28V_QGSJ:
                    GET_ZJB_CMD14VAL(28V_QGSJ, board, buff);
                break;
                case ID_ZJB_12V28V_HJJC1:
                    GET_ZJB_CMD14VAL(12V28V_HJJC1, board, buff);
                break;
                case ID_ZJB_12V28V_HJJC2:
                    GET_ZJB_CMD14VAL(12V28V_HJJC2, board, buff);
                break;
                case ID_ZJB_12V28V_HJJC3:
                    GET_ZJB_CMD14VAL(12V28V_HJJC3, board, buff);
                break;
                case ID_ZJB_12V_GSDJ1:
                    GET_ZJB_CMD14VAL(12V_GSDJ1, board, buff);
                break;
                case ID_ZJB_12V_GSDJ2:
                    GET_ZJB_CMD14VAL(12V_GSDJ2, board, buff);
                break;
                case ID_ZJB_12V_XGHTM:
                    GET_ZJB_CMD14VAL(12V_XGHTM, board, buff);
                break;
                case ID_ZJB_28V_KF:
                    GET_ZJB_CMD14VAL(28V_KF, board, buff);
                break;
                case ID_ZJB_HALL_GSDJ1:
                    GET_ZJB_CMD14VAL(HALL_GSDJ1, board, buff);
                break;
                case ID_ZJB_HALL_GSDJ2:
                    GET_ZJB_CMD14VAL(HALL_GSDJ2, board, buff);
                break;

                default:
                    TRACE_OUT(DEBUG_OUT, "Line%d : invaild param[%02x]\r\n", __LINE__, buff[1]);
                break;
            }
        }
    }
    ret = do_respcmd22_transmit(&spi_protocol, buff+1, len-1);  // spi发送数据
	return ret;
}

static int do_respcmd15 (void *protocol_arg, u8 *buff, u16 len)
{
    _ipmi_sensor_t *sensor = (_ipmi_sensor_t*)protocol_arg;
    int ret = 0;
    u8 slaveaddr = 0;
    #if 0
    fan_id = sensor->ipmi->request.data[1]; // 记录访问的FAN ID
    if(sensor->ipmi->i2cbus->dev == I2C1)
    {
        slaveaddr = sensor->i2ca_addr;
    }
    if(sensor->ipmi->i2cbus->dev == I2C2)
    {
        slaveaddr = sensor->i2cb_addr;
    }
    #endif
    if(len)
    {
        if(buff[0] == VSO_ORGANIZATION)
        {
            //sensor->status = IPMI_CMD_OK;
            TRACE_OUT(DEBUG_OUT, "Line%d : CMD15 OK\r\n", __LINE__);
            //do_reqcmd16(sensor, slaveaddr, fan_id);       // 收到设置成功CMD15，下发查询FAN1转速
        }
    }
    return ret;
}

/*获取风扇转速*/
static int do_respcmd16 (void *protocol_arg, u8 *buff, u16 len)
{
    _ipmi_sensor_t *sensor = (_ipmi_sensor_t*)protocol_arg;
    int ret = 0;
    u16 speed = 0;
    _zjb_monitor_t *board = (_zjb_monitor_t *)&ipmi_data.zjb;
    
    if(len==4)
    {
        if(buff[0] == VSO_ORGANIZATION)
        {
            sensor->status = IPMI_CMD_OK;   // 设置状态
            switch(buff[1])
            {
                case ID_ZJB_FAN1:
                    GET_ZJB_FAN(FAN1, board, speed, buff);
                break;
                case ID_ZJB_FAN2:
                    GET_ZJB_FAN(FAN2, board, speed, buff);
                break;
                case ID_ZJB_FAN3:
                    GET_ZJB_FAN(FAN3, board, speed, buff);
                break;
                case ID_ZJB_FAN4:
                    GET_ZJB_FAN(FAN4, board, speed, buff);
                break;
                default:
                    TRACE_OUT(DEBUG_OUT, "Line%d : invaild param[%02x]\r\n", __LINE__, buff[1]);
                break;
            }
        }
    }
    // TODO: spi发送数据，lys：我的理解这里调用do_respcmd22_transmit这个接口没用，可屏蔽。
    //ret = do_respcmd22_transmit((_spi_t*)&spi_protocol, buff+1, len-1);
    return ret;
}


static int do_respcmd17 (void *protocol_arg, u8 *buff, u16 len)
{
    _ipmi_sensor_t *sensor = (_ipmi_sensor_t*)protocol_arg;
    int ret = 0;
    u8 tx_buff[1] = {0};
    
    if(len == 1 && buff[0] == VSO_ORGANIZATION)
    {
        sensor->status = IPMI_CMD_OK;   // 设置状态
        TRACE_OUT(DEBUG_OUT, "< SPI CMD25 > Config ZJB power up after power on\r\n");
    }

    // TODO: spi发送数据
    ret = do_respcmd25_transmit((_spi_t*)&spi_protocol, tx_buff, 1);
    return ret;
}

static int do_respcmd18 (void *protocol_arg, u8 *buff, u16 len)
{
    _ipmi_sensor_t *sensor = (_ipmi_sensor_t*)protocol_arg;
    int ret = 0;

    if(len !=0 && buff[0] == VSO_ORGANIZATION)
    {
        sensor->status = IPMI_CMD_OK;   // 设置状态
    }

    // TODO: spi发送数据
    ret = do_respcmd13_transmit((_spi_t*)&spi_protocol, buff+1, len-1);
    return ret;
}
/*
--------------------------------------------------------------------------------
    CMD19 数据解析
--------------------------------------------------------------------------------
*/
static void ipmi_jkkz_abnormal_prase(u8 *buff, u16 len, struct _abnormal_t *abnormal)
{
    int index = 0;
    _jkkz_abnormal_t *ab_board = &abnormal->jkkz_abnormal;

    for(index = 0; index<len/2; index++)
    {
        switch(buff[index*2])
        {
            case AB_JKKZ_EEPROM:
                ab_board->AB_JKKZ_EEPROM = buff[index*2+1];
            break;
            case ID_JKKZ_TEMPRETURE:
                ab_board->AB_JKKZ_TEMPRETURE = buff[index*2+1];
            break;
            case ID_JKKZ_VCC1V2:
                ab_board->AB_JKKZ_VCC1V2 = buff[index*2+1];
            break;
            case ID_JKKZ_VCC1V5:
                ab_board->AB_JKKZ_VCC1V5 = buff[index*2+1];
            break;
            case ID_JKKZ_VCC1V8:
                ab_board->AB_JKKZ_VCC1V8 = buff[index*2+1];
            break;
            case ID_JKKZ_VCC1V0:
                ab_board->AB_JKKZ_VCC1V0 = buff[index*2+1];
            break;
            case ID_JKKZ_VCC2V5:
                ab_board->AB_JKKZ_VCC2V5 = buff[index*2+1];
            break;
            case ID_JKKZ_VCC3V3:
                ab_board->AB_JKKZ_VCC3V3 = buff[index*2+1];
            break;
            default:
                TRACE_OUT(DEBUG_OUT, "Line%d: Unknown abnormal ID [%02x]\r\n", buff[index*2]);
            break;
        }
    }
}

static void ipmi_xhcl_abnormal_prase(u8 *buff, u16 len, struct _abnormal_t *abnormal)
{
    int index = 0;
    _xhcl_abnormal_t *ab_board = &abnormal->xhcl_abnormal;

    for(index = 0; index<len/2; index++)
    {
        switch(buff[index*2])
        {
            case AB_XHCL_EEPROM:
                ab_board->AB_XHCL_EEPROM = buff[index*2+1];
            break;
            case ID_XHCL_TEMPRETURE:
                ab_board->AB_XHCL_TEMPRETURE = buff[index*2+1];
            break;
            case ID_XHCL_VCC1V2:
                ab_board->AB_XHCL_VCC1V2 = buff[index*2+1];
            break;
            case ID_XHCL_VCC1V5:
                ab_board->AB_XHCL_VCC1V5 = buff[index*2+1];
            break;
            case ID_XHCL_VCC1V8:
                ab_board->AB_XHCL_VCC1V8 = buff[index*2+1];
            break;
            case ID_XHCL_VCC1V0:
                ab_board->AB_XHCL_VCC1V0 = buff[index*2+1];
            break;
            case ID_XHCL_VCC3V3:
                ab_board->AB_XHCL_VCC3V3 = buff[index*2+1];
            break;
            default:
                TRACE_OUT(DEBUG_OUT, "Line%d: Unknown abnormal ID [%02x]\r\n", buff[index*2]);
            break;
        }
    }
}

static void ipmi_pwr_abnormal_prase(u8 *buff, u16 len, struct _abnormal_t *abnormal)
{
    int index = 0;
    _pwr_abnormal_t *ab_board = &abnormal->pwr_abnormal;

    for(index = 0; index<len/2; index++)
    {
        switch(buff[index*2])
        {
            case AB_PWR_EEPROM:
                ab_board->AB_PWR_EEPROM = buff[index*2+1];
            break;
            case ID_PWR_TEMPRETURE:
                ab_board->AB_PWR_TEMPRETURE = buff[index*2+1];
            break;
            case ID_PWR_VCC12V_1:
                ab_board->AB_PWR_VCC12V_1 = buff[index*2+1];
            break;
            case ID_PWR_VCC12V_2:
                ab_board->AB_PWR_VCC12V_2 = buff[index*2+1];
            break;
            case ID_PWR_VCC3V3:
                ab_board->AB_PWR_VCC3V3 = buff[index*2+1];
            break;
            default:
                TRACE_OUT(DEBUG_OUT, "Line%d: Unknown abnormal ID [%02x]\r\n", buff[index*2]);
            break;
        }
    }
}

static void ipmi_zjb_abnormal_prase(u8 *buff, u16 len, struct _abnormal_t *abnormal)
{
    int index = 0;
    _zjb_abnormal_t *ab_board = &abnormal->zjb_abnormal;

    for(index = 0; index<len/2; index++)
    {
        switch(buff[index*2])
        {
            case AB_ZJB_EEPROM:
                ab_board->AB_ZJB_EEPROM = buff[index*2+1];
            break;
            case ID_ZJB_TEMPRETURE:
                ab_board->AB_ZJB_TEMPRETURE = buff[index*2+1];
            break;
            case ID_ZJB_28V_JCXJ:
                ab_board->AB_ZJB_28V_JCXJ = buff[index*2+1];
            break;
            case ID_ZJB_12V_WAOXJ:
                ab_board->AB_ZJB_12V_WAOXJ = buff[index*2+1];
            break;
            case ID_ZJB_28V_BQXJ:
                ab_board->AB_ZJB_28V_BQXJ = buff[index*2+1];
            break;
            case ID_ZJB_12V_DYGY_B:
                ab_board->AB_ZJB_12V_DYGY_B = buff[index*2+1];
            break;
            case ID_ZJB_28V_FFXJ:
                ab_board->AB_ZJB_28V_FFXJ = buff[index*2+1];
            break;
            case ID_ZJB_12V28V_BF1:
                ab_board->AB_ZJB_12V28V_BF1 = buff[index*2+1];
            break;
            case ID_ZJB_12V28V_BF2:
                ab_board->AB_ZJB_12V28V_BF2 = buff[index*2+1];
            break;
            case ID_ZJB_28V_TSGY:
                ab_board->AB_ZJB_28V_TSGY = buff[index*2+1];
            break;
            case ID_ZJB_28V_QGSJ:
                ab_board->AB_ZJB_28V_QGSJ = buff[index*2+1];
            break;
            case ID_ZJB_12V28V_HJJC1:
                ab_board->AB_ZJB_12V28V_HJJC1 = buff[index*2+1];
            break;
            case ID_ZJB_12V28V_HJJC2:
                ab_board->AB_ZJB_12V28V_HJJC2 = buff[index*2+1];
            break;
            case ID_ZJB_12V28V_HJJC3:
                ab_board->AB_ZJB_12V28V_HJJC3= buff[index*2+1];
            break;
            case ID_ZJB_12V_GSDJ1:
                ab_board->AB_ZJB_12V_GSDJ1 = buff[index*2+1];
            break;
            case ID_ZJB_12V_GSDJ2:
                ab_board->AB_ZJB_12V_GSDJ2 = buff[index*2+1];
            break;
            case ID_ZJB_12V_XGHTM:
                ab_board->AB_ZJB_12V_XGHTM = buff[index*2+1];
            break;
            case ID_ZJB_28V_KF:
                ab_board->AB_ZJB_28V_KF = buff[index*2+1];
            break;
            default:
                TRACE_OUT(DEBUG_OUT, "Line%d: Unknown abnormal ID [%02x]\r\n", buff[index*2]);
            break;
        }
    }
}

extern unsigned int abnormal_pwr,abnormal_jkkz,abnormal_xhcl,abnormal_zjb;
/*
    TODO: 获取异常状态
*/
static int do_respcmd19 (void *protocol_arg, u8 *buff, u16 len)
{
    _ipmi_sensor_t *sensor = (_ipmi_sensor_t*)protocol_arg;
    int ret = 0;
    u8 comm_buff[2] = {0};
    //_zjb_monitor_t *board = (_zjb_monitor_t *)&ipmi_data.zjb;
    
    if(len !=0 && buff[0] == VSO_ORGANIZATION)
    {

        sensor->status = IPMI_CMD_OK;   // 设置状态
        
        if(buff[1] == AB_PWR_EEPROM)
        {
            abnormal_pwr++;
            ipmi_pwr_abnormal_prase(buff+1, len-1, &all_abnormal);	//lys:这把异常状态解析出来也没有用到哇，可屏蔽，且屏蔽后对代码无影响
            respcmd12_data_set(&respcmd12, buff+1, len-1);
            comm_buff[0] = AB_PWR_SUB_COMM;
            comm_buff[1] = AB_STA_DEV_OK;
            respcmd12_data_set(&respcmd12, comm_buff, 2);
			LYSprintf(LYSDEBUG1, "<<<<<<<<		PWR====%d	<<<<<<\r\n",len+1);
        }
       if(buff[1] == AB_JKKZ_EEPROM)
        {
       		 abnormal_jkkz++;
            ipmi_jkkz_abnormal_prase(buff+1, len-1, &all_abnormal);
            respcmd12_data_set(&respcmd12, buff+1, len-1);
            comm_buff[0] = AB_JKKZ_SUB_COMM;
            comm_buff[1] = AB_STA_DEV_OK;
            respcmd12_data_set(&respcmd12, comm_buff, 2);
			LYSprintf(LYSDEBUG1, "<<<<<<<<		JKKZ====%d	<<<<<<\r\n",len+1);			
        }

        if(buff[1] == AB_XHCL_EEPROM)
        {
       		 abnormal_xhcl++;
            ipmi_xhcl_abnormal_prase(buff+1, len-1, &all_abnormal);
            respcmd12_data_set(&respcmd12, buff+1, len-1);
            comm_buff[0] = AB_XHCL_SUB_COMM;
            comm_buff[1] = AB_STA_DEV_OK;
            respcmd12_data_set(&respcmd12, comm_buff, 2);
			LYSprintf(LYSDEBUG1, "<<<<<<<<		XHCL====%d	<<<<<<\r\n",len+1);			
        }

        if(buff[1] == AB_ZJB_EEPROM)
        {
       		abnormal_zjb++;
            ipmi_zjb_abnormal_prase(buff+1, len-1, &all_abnormal);
            respcmd12_data_set(&respcmd12, buff+1, len-1);
            comm_buff[0] = AB_ZJB_SUB_COMM;
            comm_buff[1] = AB_STA_DEV_OK;
            respcmd12_data_set(&respcmd12, comm_buff, 2);
			LYSprintf(LYSDEBUG1, "<<<<<<<<		ZJB====%d	<<<<<<\r\n",len+1);			
        }
    }

    // spi发送数据
    //ret = do_respcmd12_transmit((_spi_t*)&spi_protocol, buff+1, len-1);
    return ret;
}


static int do_respcmd20 (void *protocol_arg, u8 *buff, u16 len)
{
    _protocol_t *protocol = (_protocol_t*)protocol_arg;
    int ret = 0;
    
    if(len == 0)
    {
        /* 填充data字段 */
        protocol->response.is_complete = 1;
        protocol->response.data[0] = IPMI_SENSOR_NUM;
        protocol->response.data[1] = 0;
        protocol->response.frame_len = 2+REQUEST_HEADER_SIZE;

        // TODO: 
    }
    return ret;
}

static void ipmi_power_prase(u8 *buff, u16 len, _ipmi_data_t *ipmidata)
{
    _pwr_monitor_t *board = &ipmidata->power;
    
    switch(buff[0])
    {
        case ID_PWR_TEMPRETURE:
            ipmidata->power.tempreture = (buff[3]<<8) | buff[4];
            TRACE_OUT(DEBUG_OUT, "%s() Line%d: Tempreture[%d] \r\n", __FUNCTION__, __LINE__, ipmidata->power.tempreture);
        break;
        case ID_PWR_VCC12V_1:
            GET_PWR_CMD2DVAL(VCC12V_1, board, buff);
        break;
        case ID_PWR_VCC12V_2:
            GET_PWR_CMD2DVAL(VCC12V_2, board, buff);
        break;
        case ID_PWR_VCC3V3:
            GET_PWR_CMD2DVAL(VCC3V3, board, buff);
        break;
        
        default:
            TRACE_OUT(DEBUG_OUT, "Line%d : invaild param[%02x]\r\n", __LINE__, buff[0]);
        break;
    }
}

static void ipmi_jkkz_prase(u8 *buff, u16 len, _ipmi_data_t *ipmidata)
{
    _jkkz_monitor_t *board = &ipmidata->jkkz;
    switch(buff[0])
    {
        case ID_JKKZ_TEMPRETURE:
            ipmidata->jkkz.tempreture = (buff[3]<<8) | buff[4];
        break;
        case ID_JKKZ_VCC1V2:
            GET_JKKZ_CMD2DVAL(VCC1V2, board, buff);
        break;
        case ID_JKKZ_VCC1V5:
            GET_JKKZ_CMD2DVAL(VCC1V5, board, buff);
        break;
        case ID_JKKZ_VCC1V8:
            GET_JKKZ_CMD2DVAL(VCC1V8, board, buff);
        break;
        case ID_JKKZ_VCC1V0:
            GET_JKKZ_CMD2DVAL(VCC1V0, board, buff);
        break;
        case ID_JKKZ_VCC2V5:
            GET_JKKZ_CMD2DVAL(VCC2V5, board, buff);
        break;
        case ID_JKKZ_VCC3V3:
            GET_JKKZ_CMD2DVAL(VCC3V3, board, buff);
        break;
        case ID_JKKZ_VPX12V_CURR:
                ipmidata->jkkz.VOL_VPX12V_CURR = (buff[3]<<8) | buff[4];
                ipmidata->jkkz.ONOFF_VPX12V_CURR = buff[5]>>4;
        break;
        default:
            TRACE_OUT(DEBUG_OUT, "Line%d : invaild param[%02x]\r\n", __LINE__, buff[0]);
        break;
    }
}

static void ipmi_xhcl_prase(u8 *buff, u16 len, _ipmi_data_t *ipmidata)
{
    _xhcl_monitor_t *board = &ipmidata->xhcl;
    
    switch(buff[0])
    {
        case ID_XHCL_TEMPRETURE:
            ipmidata->xhcl.tempreture = (buff[3]<<8) | buff[4];
        break;
        case ID_XHCL_VCC1V2:
            GET_XHCL_CMD2DVAL(VCC1V2, board, buff);
        break;
        case ID_XHCL_VCC1V5:
            GET_XHCL_CMD2DVAL(VCC1V5, board, buff);
        break;
        case ID_XHCL_VCC1V8:
            GET_XHCL_CMD2DVAL(VCC1V8, board, buff);
        break;
        case ID_XHCL_VCC1V0:
            GET_XHCL_CMD2DVAL(VCC1V0, board, buff);
        break;
        case ID_XHCL_VCC3V3:
            GET_XHCL_CMD2DVAL(VCC3V3, board, buff);
        break;
        case ID_XHCL_VPX12V_CURR:
            ipmidata->xhcl.VOL_VPX12V_CURR = (buff[3]<<8) | buff[4];
            ipmidata->xhcl.ONOFF_VPX12V_CURR = buff[5]>>4;
        break;
		case ID_XHCL_SLOT_ADDR:
	    	ipmidata->xhcl.slot_addr =buff[2];
		break;
        default:
            TRACE_OUT(DEBUG_OUT, "Line%d : invaild param[%02x]\r\n", __LINE__, buff[0]);
        break;
    }
}


/*
	传感器的状态赋值。VOL、CURR、ONOFF,对应上位机的电压、电流、小灯

*/
static void ipmi_zjb_prase(u8 *buff, u16 len, _ipmi_data_t *ipmidata)
{
    _zjb_monitor_t *board = &ipmidata->zjb;
    switch(buff[0])
    {
        case ID_ZJB_TEMPRETURE:
            ipmidata->zjb.tempreture = (buff[3]<<8) | buff[4];
        break;
        case ID_ZJB_28V_JCXJ:
            GET_ZJB_VAL(28V_JCXJ, board, buff);
//			TRACE_OUT(DEBUG_OUT, "<<<<<<<<	ID_28V_JCXJ , V=%d, I=%d, S=%d	<<<<<<<\r\n",\
//				board->VOL_28V_JCXJ, board->CURR_28V_JCXJ, board->ONOFF_28V_JCXJ);
//			TRACE_OUT(DEBUG_OUT, "<<<<<<<<	ID_28V_JCXJ , V=%d, I=%d, S=%d	<<<<<<<\r\n",\
//				board->VOL_28V_JCXJ, board->CURR_28V_JCXJ, board->ONOFF_28V_JCXJ);
        break;
        case ID_ZJB_12V_WAOXJ:
            GET_ZJB_VAL(12V_WAOXJ, board, buff);
        break;
        case ID_ZJB_28V_BQXJ:
            GET_ZJB_VAL(28V_BQXJ, board, buff);
        break;
        case ID_ZJB_12V_DYGY_B:
            GET_ZJB_VAL(12V_DYGY_B, board, buff);
        break;
        case ID_ZJB_28V_FFXJ:
            GET_ZJB_VAL(28V_FFXJ, board, buff);
        break;
        case ID_ZJB_12V28V_BF1:
            GET_ZJB_VAL(12V28V_BF1, board, buff);
        break;
        case ID_ZJB_12V28V_BF2:
            GET_ZJB_VAL(12V28V_BF2, board, buff);
        break;
        case ID_ZJB_28V_TSGY:
            GET_ZJB_VAL(28V_TSGY, board, buff);
        break;
        case ID_ZJB_28V_QGSJ:
            GET_ZJB_VAL(28V_QGSJ, board, buff);
        break;
        case ID_ZJB_12V28V_HJJC1:
            GET_ZJB_VAL(12V28V_HJJC1, board, buff);
        break;
        case ID_ZJB_12V28V_HJJC2:
            GET_ZJB_VAL(12V28V_HJJC2, board, buff);
        break;
        case ID_ZJB_12V28V_HJJC3:
            GET_ZJB_VAL(12V28V_HJJC3, board, buff);
        break;
        case ID_ZJB_12V_GSDJ1:
            GET_ZJB_VAL(12V_GSDJ1, board, buff);
        break;
        case ID_ZJB_12V_GSDJ2:
            GET_ZJB_VAL(12V_GSDJ2, board, buff);
        break;
        case ID_ZJB_12V_XGHTM:
            GET_ZJB_VAL(12V_XGHTM, board, buff);
        break;
        case ID_ZJB_28V_KF:
            GET_ZJB_VAL(28V_KF, board, buff);
        break;
        case ID_ZJB_HALL_GSDJ1:
            GET_ZJB_VAL(HALL_GSDJ1, board, buff);
        break;
        case ID_ZJB_HALL_GSDJ2:
            GET_ZJB_VAL(HALL_GSDJ2, board, buff);
		break;
	    case ID_ZJB_SLOT_ADDR:
	    	ipmidata->zjb.slot_addr =buff[2];
        break;
        default:
            TRACE_OUT(DEBUG_OUT, "Line%d : invaild param[%02x]\r\n", __LINE__, buff[0]);
        break;
    }
	
}

static int do_respcmd2d (void *protocol_arg, u8 *buff, u16 len)
{
    _ipmi_sensor_t *sensor = (_ipmi_sensor_t*)protocol_arg;
    int ret = 0;
    //u16 voltage = 0, current = 0, status = 0;

    sensor->status = IPMI_CMD_OK;
    if((buff[0] >= ID_PWR_TEMPRETURE) && (buff[0]<= ID_PWR_VCC3V3))
    {
        ipmi_power_prase(buff, len, (_ipmi_data_t *)&ipmi_data);
    }

    if((buff[0] >= ID_JKKZ_TEMPRETURE) && (buff[0] <= ID_JKKZ_VPX12V_CURR))
    {
        ipmi_jkkz_prase(buff, len, (_ipmi_data_t *)&ipmi_data);
    }

    if((buff[0] >= ID_XHCL_TEMPRETURE) && (buff[0] <= ID_XHCL_SLOT_ADDR	))
    {
        ipmi_xhcl_prase(buff, len, (_ipmi_data_t *)&ipmi_data);
    }

    if((buff[0] >= ID_ZJB_TEMPRETURE) && (buff[0] <= ID_ZJB_SLOT_ADDR))//13<=buf[0]<=32
    {
        ipmi_zjb_prase(buff, len, (_ipmi_data_t *)&ipmi_data);
    }
    
    return ret;
}



_do_data_t do_cmd[] = {
    {IPMI_CMD14, do_respcmd14},
    {IPMI_CMD15, do_respcmd15},
    {IPMI_CMD16, do_respcmd16},
    {IPMI_CMD17, do_respcmd17},
    {IPMI_CMD18, do_respcmd18},
    {IPMI_CMD19, do_respcmd19},
    {IPMI_CMD20, do_respcmd20},
    {IPMI_CMD2D, do_respcmd2d},
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
	static int checkErrNum=0;
	int temp0=0;
    _ipmi_sensor_t *sensor = (_ipmi_sensor_t*)protocol_arg;
    int index = 0, ret = 0;
    u8 recv_checksum = 0, calc_checksum = 0;
    /* 1、检验数据 */
    recv_checksum = buff[len-1];
    calc_checksum = check_recvsum(buff+2, len-2);
    if(recv_checksum == calc_checksum)
    {
        sensor->ipmi->response.netfn   = buff[0];
        sensor->ipmi->response.src_addr = sensor->ipmi->i2cbus->selfaddr;
        sensor->ipmi->response.req_num = buff[3];
        sensor->ipmi->response.cmd     = buff[4];
        /* 3、处理指令,并发送响应 */
        for(index = 0; index < ARRAY_SIZE(do_cmd); index++)
        {
            if(do_cmd[index].cmd == sensor->ipmi->response.cmd)
                ret = do_cmd[index].do_data(sensor, buff+6, len-7); // 4字节是数据域的头
        }
    }
    else
    {
		checkErrNum++;
//        TRACE_OUT(DEBUG_OUT, "%s Line%d: Check sum fail, CALC[%02x] != FRAM[%02x]\r\n", \
//                    __FUNCTION__, __LINE__,calc_checksum, recv_checksum);
        LYSprintf(LYSDEBUG1, "%s Line%d: Check sum fail, CALC[%02x] != FRAM[%02x]\r\n", \
                    __FUNCTION__, __LINE__,calc_checksum, recv_checksum);

		for(temp0=0;temp0<len;temp0++)
		{
			if(temp0%16==0)
			{
				LYSprintf(LYSDEBUG1, "\r\n");
			}
			LYSprintf(LYSDEBUG1, "%8x",buff[temp0]);
		}
		LYSprintf(LYSDEBUG1, "\r\n");
		if(checkErrNum>3)
		{
			checkErrNum=0;
			/*将当前执行查询的板卡状态置位，关闭定时器*/
    		ipmi_app.ipmi_arry[ipmi_app.index]->status=IPMI_CMD_START;
			ipmi_sensortimer_stop(ipmi_app.ipmi_arry[ipmi_app.index]);
			/*切换到第一张板卡开始执行，且状态置位*/
    		ipmi_app.index = 0;
			ipmi_app.ipmi_arry[ipmi_app.index]->status=IPMI_CMD_START;
			ipmi_app.ipmi_arry[ipmi_app.index]->sensor_index=0;
			/*复位循环buffer*/
			u8_ring_buffer_init(&i2c1_int.rb_handler, (char *)i2c1_int.rxbuff, IIC_DATA_SIZE);
			u8_ring_buffer_init(&i2c2_int.rb_handler, (char *)i2c2_int.rxbuff, IIC_DATA_SIZE);
//			NVIC_SystemReset();
		}
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
int errDataNum=0;
int ipmi_recv_deal(_ipmi_sensor_t *sensor)
{
    int ret = 0, recv_len = 0;
    u8 temp = 0, temp1 = 0;
    u8 buff[256] = {0};
	if(u8_ring_buffer_num_items(&sensor->ipmi->i2cbus->rb_handler) >= 2)
	{
		ret = u8_ring_buffer_peek(&sensor->ipmi->i2cbus->rb_handler, (char *)&temp, 0);
		if(temp==REQ_NET_FUNC)
		{
			ret = u8_ring_buffer_peek(&sensor->ipmi->i2cbus->rb_handler, (char *)&temp1, 1); // 获取当前帧长度
			if(ret == 1)
			{
				recv_len = temp1;
				if(u8_ring_buffer_num_items(&sensor->ipmi->i2cbus->rb_handler) >= recv_len) // 接收满一帧数据
				{
					ret = u8_ring_buffer_dequeue_arr(&sensor->ipmi->i2cbus->rb_handler, (char *)buff, recv_len);
					if(ret != recv_len)
					{
						TRACE_OUT(LYSDEBUG5, "Read ring buffer fail \r\n");
					}
					else
					{
						ret = sensor->ipmi->do_cmd(sensor, buff, recv_len);//do_cmd_dispatch()
					}
				}
			}
		}
		else
		{
			errDataNum++;
			/* 出栈，脏数据 */
			while((temp!=REQ_NET_FUNC)&&(u8_ring_buffer_num_items(&sensor->ipmi->i2cbus->rb_handler)!=0))
			{
				u8_ring_buffer_dequeue(&sensor->ipmi->i2cbus->rb_handler, (char *)&temp);
				if(temp==REQ_NET_FUNC)
				{
					u8_ring_buffer_queue(&sensor->ipmi->i2cbus->rb_handler,temp);
				}
				LYSprintf(LYSDEBUG5, "err data temp=%x, errdataNum=%d \r\n", temp,errDataNum);	
			}

		}
	}

    return ret;
}
#endif

/****************************************************************************************************************************/
#if CODE_PART("请求数据处理")

/*
    @brief      : 设置板卡是否供电
    @param[in]  :
        protocol        ipmi协议句柄指针
        slaveaddr       从机地址
        pwr_id          电源ID
        is_poweron      供电使能，0x00取消供电， 0xFF打开供电
*/
void do_reqcmd14(_ipmi_sensor_t *sensor, u8 slaveaddr, u8 pwr_id, u8 is_poweron)
{
    if(IPMI_SENSOR_GET_BUS(sensor) == I2C1)
        sensor->ipmi->request.src_addr = IPMI_MASTER_I2CA_ID;
    if(IPMI_SENSOR_GET_BUS(sensor) == I2C2)
        sensor->ipmi->request.src_addr = IPMI_MASTER_I2CB_ID;

    sensor->ipmi->request.dest_addr = slaveaddr;
    sensor->ipmi->request.netfn     = REQ_NET_FUNC;
    sensor->ipmi->request.req_num   = 0xF0;
    sensor->ipmi->request.cmd       = IPMI_CMD14;
    
    /* 数据填充 */
    sensor->ipmi->request.data[0] = VSO_ORGANIZATION;
    sensor->ipmi->request.data[1] = pwr_id;
    sensor->ipmi->request.data[2] = is_poweron;

    sensor->ipmi->request.frame_len = 3+REQUEST_HEADER_SIZE;
    ipmi_request_transmit(sensor->ipmi);
}

/*
    @brief      : 设置风扇速度
    @param[in]  :
        protocol        ipmi协议句柄指针
        slaveaddr       从机地址
        fan_id          风扇ID
        fan_level       设置风扇转速等级(1-4)
*/
void do_reqcmd15(_ipmi_sensor_t *sensor, u8 slaveaddr, u8 fan_id, u8 fan_level)
{
    if(IPMI_SENSOR_GET_BUS(sensor) == I2C1)
        sensor->ipmi->request.src_addr = IPMI_MASTER_I2CA_ID;
    if(IPMI_SENSOR_GET_BUS(sensor) == I2C2)
        sensor->ipmi->request.src_addr = IPMI_MASTER_I2CB_ID;

    sensor->ipmi->request.dest_addr = slaveaddr;
    sensor->ipmi->request.netfn     = REQ_NET_FUNC;
    sensor->ipmi->request.req_num   = 0xF0;
    sensor->ipmi->request.cmd       = IPMI_CMD15;
    
    /* 数据填充 */
    sensor->ipmi->request.data[0] = VSO_ORGANIZATION;
    sensor->ipmi->request.data[1] = fan_id;
    sensor->ipmi->request.data[2] = fan_level;

    sensor->ipmi->request.frame_len = 3+REQUEST_HEADER_SIZE;
    ipmi_request_transmit(sensor->ipmi);
}


/*
    @brief      : 获取风扇速度
    @param[in]  :
        protocol        ipmi协议句柄指针
        slaveaddr       从机地址
        fan_id          风扇ID
*/
void do_reqcmd16(_ipmi_sensor_t *sensor, u8 slaveaddr, u8 fan_id)
{
    if(IPMI_SENSOR_GET_BUS(sensor) == I2C1)
        sensor->ipmi->request.src_addr = IPMI_MASTER_I2CA_ID;
    if(IPMI_SENSOR_GET_BUS(sensor) == I2C2)
        sensor->ipmi->request.src_addr = IPMI_MASTER_I2CB_ID;
    
    sensor->ipmi->request.dest_addr = slaveaddr;
    sensor->ipmi->request.netfn     = REQ_NET_FUNC;
    sensor->ipmi->request.req_num   = 0xF0;
    sensor->ipmi->request.cmd       = IPMI_CMD16;
    
    /* 数据填充 */
    sensor->ipmi->request.data[0] = VSO_ORGANIZATION;
    sensor->ipmi->request.data[1] = fan_id;

    sensor->ipmi->request.frame_len = 2+REQUEST_HEADER_SIZE;
    ipmi_request_transmit(sensor->ipmi);
}

/*
    @brief      : 配置转接板电源启动上电是否打开
    @param[in]  :
        protocol        ipmi协议句柄指针
        slaveaddr       从机地址
        config          配置数组首地址
        len             数组长度
*/
void do_reqcmd17(_ipmi_sensor_t *sensor, u8 slaveaddr, u8 *config, u16 len)
{
    if(IPMI_SENSOR_GET_BUS(sensor) == I2C1)
        sensor->ipmi->request.src_addr = IPMI_MASTER_I2CA_ID;
    if(IPMI_SENSOR_GET_BUS(sensor) == I2C2)
        sensor->ipmi->request.src_addr = IPMI_MASTER_I2CB_ID;
    
    sensor->ipmi->request.dest_addr = slaveaddr;
    sensor->ipmi->request.netfn     = REQ_NET_FUNC;
    sensor->ipmi->request.req_num   = 0xF0;
    sensor->ipmi->request.cmd       = IPMI_CMD17;
    
    /* 数据填充 */
    sensor->ipmi->request.data[0] = VSO_ORGANIZATION;
    memcpy(sensor->ipmi->request.data+1, config, len);

    sensor->ipmi->request.frame_len = 1+len+REQUEST_HEADER_SIZE;
    ipmi_request_transmit(sensor->ipmi);
}


/*
    @brief      : 获取转接板电源启动上电配置
    @param[in]  :
        protocol        ipmi协议句柄指针
        slaveaddr       从机地址
        config          配置数组首地址
        len             数组长度
*/
void do_reqcmd18(_ipmi_sensor_t *sensor, u8 slaveaddr)
{
    if(IPMI_SENSOR_GET_BUS(sensor) == I2C1)
        sensor->ipmi->request.src_addr = IPMI_MASTER_I2CA_ID;
    if(IPMI_SENSOR_GET_BUS(sensor) == I2C2)
        sensor->ipmi->request.src_addr = IPMI_MASTER_I2CB_ID;
    
    sensor->ipmi->request.dest_addr = slaveaddr;
    sensor->ipmi->request.netfn     = REQ_NET_FUNC;
    sensor->ipmi->request.req_num   = 0xF0;
    sensor->ipmi->request.cmd       = IPMI_CMD18;
    
    /* 数据填充 */
    sensor->ipmi->request.data[0] = VSO_ORGANIZATION;

    sensor->ipmi->request.frame_len = 1+REQUEST_HEADER_SIZE;
    ipmi_request_transmit(sensor->ipmi);
}

/*
    @brief      : 获取异常状态
    @param[in]  :
        protocol        ipmi协议句柄指针
        slaveaddr       从机地址
        config          配置数组首地址
        len             数组长度
*/
void do_reqcmd19(_ipmi_sensor_t *sensor, u8 slaveaddr)
{
    if(IPMI_SENSOR_GET_BUS(sensor) == I2C1)
        sensor->ipmi->request.src_addr = IPMI_MASTER_I2CA_ID;
    if(IPMI_SENSOR_GET_BUS(sensor) == I2C2)
        sensor->ipmi->request.src_addr = IPMI_MASTER_I2CB_ID;
    
    sensor->ipmi->request.dest_addr = slaveaddr;
    sensor->ipmi->request.netfn     = REQ_NET_FUNC;
    sensor->ipmi->request.req_num   = 0xF0;
    sensor->ipmi->request.cmd       = IPMI_CMD19;
    
    /* 数据填充 */
    sensor->ipmi->request.data[0] = VSO_ORGANIZATION;

    sensor->ipmi->request.frame_len = 1+REQUEST_HEADER_SIZE;
    ipmi_request_transmit(sensor->ipmi);
}

/*
    @brief      : 读取设备传感器个数
        @note   : 每个传感器数量基本都是已知的,此条指令基本无用
    @param[in]  :
        protocol        ipmi协议句柄指针
        slaveaddr       从机地址
        pwr_id          电源ID
        is_poweron      供电使能，0x00取消供电， 0xFF打开供电
*/
void do_reqcmd20(_ipmi_sensor_t *sensor, u8 slaveaddr, u8 req_num)
{
    if(IPMI_SENSOR_GET_BUS(sensor) == I2C1)
        sensor->ipmi->request.src_addr = IPMI_MASTER_I2CA_ID;
    if(IPMI_SENSOR_GET_BUS(sensor) == I2C2)
        sensor->ipmi->request.src_addr = IPMI_MASTER_I2CB_ID;

    sensor->ipmi->request.dest_addr = slaveaddr;
    sensor->ipmi->request.netfn     = REQ_NET_FUNC;
    sensor->ipmi->request.req_num   = 0xF0;
    sensor->ipmi->request.cmd       = IPMI_CMD20;
    /* 数据填充 */
    // 空
    sensor->ipmi->request.frame_len = REQUEST_HEADER_SIZE;
    ipmi_request_transmit(sensor->ipmi);
}


/*
    @brief      : 读取传感器采样值
    @param[in]  :
    protocol        ipmi协议句柄指针
    slaveaddr       从机地址
    sensor_id       传感器id
*/
void do_reqcmd2d(_ipmi_sensor_t *sensor, u8 slaveaddr, u8 sensor_id)
{
    if(IPMI_SENSOR_GET_BUS(sensor) == I2C1)
        sensor->ipmi->request.src_addr = IPMI_MASTER_I2CA_ID;
    if(IPMI_SENSOR_GET_BUS(sensor) == I2C2)
        sensor->ipmi->request.src_addr = IPMI_MASTER_I2CB_ID;
    
    sensor->ipmi->request.dest_addr = slaveaddr;
    sensor->ipmi->request.netfn     = REQ_NET_FUNC;
    sensor->ipmi->request.req_num   = 0xF0;
    sensor->ipmi->request.cmd       = IPMI_CMD2D;
    /* 数据填充 */
    sensor->ipmi->request.data[0]   = sensor_id;
    
    sensor->ipmi->request.frame_len = 1+REQUEST_HEADER_SIZE;
    ipmi_request_transmit(sensor->ipmi);
}


#endif

/****************************************************************************************************************************/
#if CODE_PART("协议栈应用，定时获取数据")

#if 0
/* 预留给 哈特曼板卡 */
_ipmi_sensor_t ipmi_htm = {
    .ipmi = &ipmi_i2ca,
    .timeout    = 1000,
    .sensor_num = 4,
    .sensor_id = {
        ID_PWR_TEMPRETURE,
        ID_PWR_VCC12V_1,
        ID_PWR_VCC12V_2,
        ID_PWR_VCC3V3
    },
};
#endif
/*lys:1000->500*/
#define CHECKTIMEOUT (500)
_ipmi_sensor_t ipmi_pwr = {
    .ipmi = &ipmi_i2ca,
    .timeout    = CHECKTIMEOUT,
    .i2ca_addr  = IPMI_PWR_I2CA_ID,
    .i2cb_addr  = IPMI_PWR_I2CB_ID,
    .sensor_num = 4,
    .sensor_id = {
        ID_PWR_TEMPRETURE,
        ID_PWR_VCC12V_1,
        ID_PWR_VCC12V_2,
        ID_PWR_VCC3V3
    },
};

_ipmi_sensor_t ipmi_jkkz = {
    .ipmi = &ipmi_i2ca,
    .timeout    = CHECKTIMEOUT,
    .i2ca_addr  = IPMI_JKKZ_I2CA_ID,
    .i2cb_addr  = IPMI_JKKZ_I2CB_ID,
    .sensor_num = 7,
    .sensor_id = {
        ID_JKKZ_TEMPRETURE,
        ID_JKKZ_VCC1V2,
        ID_JKKZ_VCC1V5,
        ID_JKKZ_VCC1V8,
        ID_JKKZ_VCC1V0,
        ID_JKKZ_VCC3V3,
        ID_JKKZ_VPX12V_CURR,
    },
};

_ipmi_sensor_t ipmi_xhcl = {
    .ipmi = &ipmi_i2ca,
    .timeout    = CHECKTIMEOUT,
    .i2ca_addr  = IPMI_XHCL_I2CA_ID,
    .i2cb_addr  = IPMI_XHCL_I2CB_ID,
    .sensor_num = 8,
    .sensor_id = {
        ID_XHCL_TEMPRETURE,
        ID_XHCL_VCC1V2,
        ID_XHCL_VCC1V5,
        ID_XHCL_VCC1V8,
        ID_XHCL_VCC1V0,
        ID_XHCL_VCC3V3,
        ID_XHCL_VPX12V_CURR,
        ID_XHCL_SLOT_ADDR
    },
};

_ipmi_sensor_t ipmi_zjb = {
    .ipmi = &ipmi_i2ca,
    .timeout    = CHECKTIMEOUT,
    .i2ca_addr  = IPMI_ZJB_I2CA_ID,
    .i2cb_addr  = IPMI_ZJB_I2CB_ID,
    .sensor_num = 24,
    .sensor_id = {
        ID_ZJB_FAN1,
        ID_ZJB_FAN2,
        ID_ZJB_FAN3,
        ID_ZJB_FAN4,
        ID_ZJB_TEMPRETURE,
        ID_ZJB_28V_JCXJ,
        ID_ZJB_12V_WAOXJ,
        ID_ZJB_28V_BQXJ,
        ID_ZJB_12V_DYGY_B,
        ID_ZJB_28V_FFXJ,
        ID_ZJB_12V28V_BF1,
        ID_ZJB_12V28V_BF2,
        ID_ZJB_28V_TSGY,
        ID_ZJB_28V_QGSJ,
        ID_ZJB_12V28V_HJJC1,
        ID_ZJB_12V28V_HJJC2,
        ID_ZJB_12V28V_HJJC3,
        ID_ZJB_12V_GSDJ1,
        ID_ZJB_12V_GSDJ2,
        ID_ZJB_12V_XGHTM,
        ID_ZJB_28V_KF,
        ID_ZJB_HALL_GSDJ1,
        ID_ZJB_HALL_GSDJ2,
        ID_ZJB_SLOT_ADDR,
    },
};



_ipmi_sensor_t *g_ipmi_current = NULL;
_ipmi_sensor_t *g_ipmi_fan_setting = NULL;

_ipmi_app_t ipmi_app = {0};


void ipmi_sensordata_callback(MultiTimer* timer, void* userData)
{
    _ipmi_sensor_t *sensor = (_ipmi_sensor_t*)userData;
    sensor->status = IPMI_CMD_TIMEOUT;
    //softtimer_stop(timer);
    //TRACE_OUT(DEBUG_OUT, "Line%d: ipmi callback count[%d]\r\n", __LINE__, ipmi_timer_count);
//    LYSprintf(LYSDEBUG1, "<<<<<<<<	ipmi_app.index=%d	sensor_index=%d<<<<<<<<<<<\r\n",ipmi_app.index,ipmi_app.ipmi_arry[ipmi_app.index]->sensor_index);
}

void ipmi_sensortimer_start(_ipmi_sensor_t *sensor)
{
    //TRACE_OUT(DEBUG_OUT, "=====> ipmi sensor timer[%p] START, index[%d]\r\n", &sensor->timer, sensor->sensor_index);
    softtimer_start(&sensor->timer, sensor->timeout, ipmi_sensordata_callback, sensor);
}

void ipmi_sensortimer_stop(_ipmi_sensor_t *sensor)
{
    //TRACE_OUT(DEBUG_OUT, "=====> ipmi sensor timer[%p] STOP , index[%d]\r\n", &sensor->timer, sensor->sensor_index);
    softtimer_stop(&sensor->timer);
}


/* 定义以下IPMI数据从哪个子卡开始 */
#define IPMI_APP_START_INDEX        0

/*
    @brief      : ipmi循环获取子卡sensor数据
    @param[in]  : 
        app         ipmi队列
    @retval     : 当前子卡的状态
*/
_IPMI_SENSOR_STA ipmi_sensordata_get_loop( _ipmi_app_t *app)
{
    u8 slaveaddr = 0;
    _ipmi_sensor_t *sensor= NULL;
    if(IPMI_SENSOR_GET_BUS(app->ipmi_arry[app->index]) == I2C1)
    {
        slaveaddr = app->ipmi_arry[app->index]->i2ca_addr;
    }
    if(IPMI_SENSOR_GET_BUS(app->ipmi_arry[app->index]) == I2C2)
    {
        slaveaddr = app->ipmi_arry[app->index]->i2cb_addr;
    }

    sensor = app->ipmi_arry[app->index];
	
//	LYSprintf(LYSDEBUG1, "<<<<<<<<<<<<<<<<app->index====%d<<<<<<<<<<<\r\n",app->index);
    g_ipmi_current = sensor;
    
    #if 1
    /* 1、开始查询当前板卡第一个sensor */
   if((sensor->sensor_index == 0) && (sensor->status == IPMI_CMD_START))
   {
       goto NEXT_CMD;
   }
    
    switch(sensor->status)
    {
        case IPMI_CMD_START:
             goto RETURN;
        break;

        case IPMI_CMD_OK:
        case IPMI_CMD_TIMEOUT:
            TRACE_OUT(DEBUG_OUT, "Line%d : cmd response %s !!!\r\n", __LINE__, (sensor->status == IPMI_CMD_OK) ? "OK" : "TIMEOUT");
            ipmi_sensortimer_stop(sensor);  // 成功获取子卡回复，关闭定时器
            sensor->status = IPMI_CMD_START;
////            if(sensor->sensor_index==23)
//            {
//	        //    LYSprintf(LYSDEBUG1,"*************sensor->sensor_index==%d**************\r\n",sensor->sensor_index);
				delay_ms(1);
//            }
            /* 2、继续查询当前板卡其他sensor */
            if(sensor->sensor_index < sensor->sensor_num)
            {
//                if(app->index==1)
//                {
//             //       LYSprintf(LYSDEBUG1,"*************sensor->sensor_index==%d**************\r\n",sensor->sensor_index);
//					delay_ms(10);
//                }
                goto NEXT_CMD;
            }
            else    /* 3、当前板卡sensor查询完毕 */
            {
                sensor->sensor_index = 0;//lys:将传感器编号置零
                //sensor->status = IPMI_SUB_OK;
                TRACE_OUT(DEBUG_OUT, "Line%d : sensor[%d] request completed !!!\r\n", __LINE__, app->index);
                
                // 下一个子卡指令
                if(app->index < (app->sub_count-1))
                {                  
                    app->index++;		//lys:切换板卡操作
                    //					if(app->index==1)
                    if(app->index==(app->sub_count/2))
                    {
						 app->ipmi_arry[app->index]->status = IPMI_TOTAL_OK;
					}
                    goto RETURN;
                }
                else
                {
                    sensor->status = IPMI_TOTAL_OK;
                    goto RETURN;
                }
            }
        break;
        case IPMI_TOTAL_OK:
            sensor->status = IPMI_CMD_START;    // 清状态
            app->index = 0;
            app->is_startloop = PROCESS_CMD12;              // 清状态
            lyscxcmd11all++;
            goto RETURN;
        break;
        
        default:
            TRACE_OUT(DEBUG_OUT, "Line%d : invalid STATUS[%04x]\r\n", __LINE__, sensor->status);
            goto RETURN;
        break;
    }
#endif

NEXT_CMD:
    ipmi_sensortimer_start(sensor);   // 启动1s超时定时器，lys：启动这个定时器的目的是防止一直没有收到响应，状态位不变会陷入死循环。
    if(sensor->sensor_id[sensor->sensor_index] >= ID_ZJB_FAN1 && sensor->sensor_id[sensor->sensor_index] <= ID_ZJB_FAN4)
    {
        do_reqcmd16(sensor, slaveaddr, sensor->sensor_id[sensor->sensor_index++]);  // 查询转接板转速
    }
    else
    {
        do_reqcmd2d(sensor, slaveaddr, sensor->sensor_id[sensor->sensor_index++]);//轮询读取传感器的电压电流及开关状态信息。
    }
    if(sensor->sensor_index > sensor->sensor_num)
    {
        sensor->sensor_index = 0; // 可能会陷入单块板卡死循环
    }

RETURN:
    //app->index = IPMI_APP_START_INDEX;
    return (_IPMI_SENSOR_STA)(sensor->status);
}

#endif

/****************************************************************************************************************************/
#if CODE_PART("风扇转速随温度自适应")
/*
    @brief      
            1、先查找子板最高温和最低温，取平均值
            2、如果最高温超过80度，使用最高温，设置转速
            3、温度不高于80度，最高温和最低温的平均值，设置转速
*/


/* 温度扩大100倍后的值，duty占空比 */
_fan_list_t fan_list[] = {
    {.tempreture = -2000,   .duty = 0},  /* -20度, 占空比10%基本停止了 */
    {.tempreture = -1000,   .duty = 15},  /* -10度 */
    {.tempreture = 0000,    .duty = 25},
    {.tempreture = 3000,    .duty = 40},
    {.tempreture = 5000,    .duty = 50},
    {.tempreture = 6000,    .duty = 70},
    {.tempreture = 8000,    .duty = 80},
    {.tempreture = 9000,    .duty = 90},
    {.tempreture = 10000,   .duty = 95},
    {.tempreture = 10500,   .duty = 97} /* 基本满转速 */
};

/*
    @brief      : 查找子板最高温和最低温
    @retval     : none
*/
void board_tempreture_find(_ipmi_data_t *ipmi_data, s16 *max, s16 *min)
{
    s16 temp_max = 0, temp_min = 0;

    temp_min = MIN(ipmi_data->power.tempreture, ipmi_data->jkkz.tempreture);
    temp_min = MIN(temp_min, ipmi_data->xhcl.tempreture);
    temp_min = MIN(temp_min, ipmi_data->zjb.tempreture);
    temp_min = MIN(temp_min, ipmi_data->zkb.tempreture);

    temp_max = MAX(ipmi_data->power.tempreture, ipmi_data->jkkz.tempreture);
    temp_max = MAX(temp_max, ipmi_data->xhcl.tempreture);
    temp_max = MAX(temp_max, ipmi_data->zjb.tempreture);
    temp_max = MAX(temp_max, ipmi_data->zkb.tempreture);

    *max = temp_max;
    *min = temp_min;
}


/*
    @brief      : 循环获取温度，设置风扇占空比
                    在softtimer定时任务中调用
*/
void fan_auto_setting_loop(void)
{
    s16 max = 0, min = 0, temp = 0;
    u8 slaveaddr = 0, list_len = 0;
    static u8 old_duty = 0;

    list_len = ARRAY_SIZE(fan_list);
    /* 获取从机地址 */
    if(ipmi_zjb.ipmi->i2cbus->dev == I2C1)
        slaveaddr = ipmi_zjb.i2ca_addr;
    if(ipmi_zjb.ipmi->i2cbus->dev == I2C2)
        slaveaddr = ipmi_zjb.i2cb_addr;
    
    /* 获取子卡最高温，最低温 */
    board_tempreture_find((_ipmi_data_t *)&ipmi_data, &max, &min);

    if(max >= 8000) // 温度大于80度
    {
        temp = max;
    }
    else
    {
        if(min != 0)
            temp = (max+min)/2;
        else
            temp = max*80/100; // 最低温度是0时（其他板卡没有插），取最高温的80%
    }
    #if 1
    for(int i = 0; i<list_len; i++)
    {
        if( i <= list_len-2)
        {
            if(temp >= fan_list[i].tempreture && temp < fan_list[i+1].tempreture)
            {
                if(old_duty != fan_list[i+1].duty)
                {
                    old_duty = fan_list[i+1].duty;
                    do_reqcmd15(&ipmi_zjb, slaveaddr, ID_ZJB_FAN_ALL, fan_list[i+1].duty);  // ID_ZJB_FAN_ALL直接传入duty值
		
                    TRACE_OUT(DEBUG_OUT, "Line%d: List[%d] Threshold tempreture[%d]\r\n", __LINE__, i, fan_list[i].tempreture);
                    TRACE_OUT(DEBUG_OUT, "Line%d: Tempreture[%d] Setting fan duty[%d]\r\n", __LINE__, temp, fan_list[i+1].duty);
                }
                else
                {
                    // todo nothing
                }
            }
            else
            {
                // todo nothing
                //if(i==0 && temp<fan_list[i].tempreture)
            }
        }
        else // 当前列表最后一项
        {
            if(temp >= fan_list[i].tempreture)
            {
                all_abnormal.zkb_abnormal.AB_ZKB_TEMPRETURE = AB_STA_DEV_OVER_TEMP;  // 过温异常
                do_reqcmd15(&ipmi_zjb, slaveaddr, ID_ZJB_FAN_ALL, fan_list[i].duty);
                TRACE_OUT(DEBUG_OUT, "Line%d: Tempreture[%d] Setting fan duty[%d]\r\n", __LINE__, temp, fan_list[i].duty);
            }
        }
    }
    #else
    TRACE_OUT(DEBUG_OUT, "Line%d: Index[%d] Tempreture[%d] Setting fan duty[%d]\r\n", \
                __LINE__, index, fan_list[index].tempreture, fan_list[index].duty);
    do_reqcmd15(&ipmi_zjb, slaveaddr, ID_ZJB_FAN_ALL, fan_list[index++].duty);  // 2s更新一次
    if(index >= list_len)
        index = 0;
    #endif
}

#endif


/****************************************************************************************************************************/
#if CODE_PART("子板异常查询")

_time_t abnormal_timer = {0};
static u8 request_count = 0;

void sysmon_abnormal_timer_start(void);

/******************************************************************************
 * 函 数 名：ipmi_abnormal_data_get_loop
 *
 * 函数说明: 异常状态信息获取，注意获取异常状态的cmd19，是一次性将当前板卡的所有信息获取到，如若获取超时，则获取下一板卡信息。
 * 参数说明: 
 * 输入参数: 
 * 输出参数: 
 * 返 回 值: 
 * 
 * 修改记录：
 * --------------------
 *	 
 ******************************************************************************/
_IPMI_SENSOR_STA ipmi_abnormal_data_get_loop(_ipmi_app_t *app)
{
    u8 slaveaddr = 0;
    _ipmi_sensor_t *sensor= NULL;

    sensor = app->ipmi_arry[app->index];
    g_ipmi_current = sensor;

    if(IPMI_SENSOR_GET_BUS(sensor) == I2C1)
    {
        slaveaddr = sensor->i2ca_addr;
    }
    if(IPMI_SENSOR_GET_BUS(sensor) == I2C2)
    {
        slaveaddr = sensor->i2cb_addr;
    }

    // 当前板卡异常查询完毕
    switch(sensor->status)
    {
        case IPMI_CMD_START:
				if((app->index)==0)
				{
					LYSprintf(LYSDEBUG1, "<<<<<<<<		app->index=%d	<<<<<<\r\n",app->index);
				}
                ipmi_sensortimer_start(sensor); // 启用定时器
                do_reqcmd19(sensor, slaveaddr);
                sensor->status = IPMI_AB_TRANS;
				sensor->sensor_index=0;			//lys:这条语句是为了cmd11服务，cmd12不会使用到这个变量，因为在执行cmd11时可能执行到某个寄存器查询，
												//就被cmd12打断了，导致传感器编号不为0，在后续执行cmd11时，会造成死锁的现象。
            //    delay_ms(10);
        break;
        
        case IPMI_AB_TRANS:
            // to do nothing, wait for CMD OK
        break;
        
        case IPMI_CMD_OK:
        case IPMI_CMD_TIMEOUT:
            TRACE_OUT(DEBUG_OUT, "Line%d : cmd response %s !!!\r\n", __LINE__, \
                    (sensor->status == IPMI_CMD_OK) ? "OK" : "TIMEOUT");
            
            ipmi_sensortimer_stop(sensor);
            sensor->status = IPMI_CMD_START;
            if(app->index < (app->sub_count-1))
            {
                app->index++;					//lys：查询下一张板卡
            }
            else    // 所有板卡查询完毕
            {
                request_count = 0;
                app->index = 0;
                app->is_startloop = PROCESS_CMD11;
                // TODO : 封装数据到结构体
                respcmd12.count = respcmd12.index;
				LYSprintf(LYSDEBUG1, "<<<<<<<<		respcmd12.index====%d	<<<<<<\r\n",respcmd12.index);
				/*lys:所有板子查询完，告警信息编号置0*/
				respcmd12.index=0;
				lyscxcmd12all++;
            }
        break;
        
        default:
            TRACE_OUT(DEBUG_OUT, "Line%d : invalid STATUS[%04x]\r\n", __LINE__, sensor->status);
        break;
    }
    
    //app->index = IPMI_APP_START_INDEX;
    return sensor->status;
}

/*lys：程序里没用到sysmon_abnormal_callback、sysmon_abnormal_timer_start*/
/*
    @brief      : 系统定时获取异常状态
        @note   : 监测项
            3路温度，电压，电流
*/
void sysmon_abnormal_callback(_time_t* timer, void* userData)
{
    if(ipmi_app.is_startloop == 0)
    {
        ipmi_app.is_startloop = PROCESS_CMD12;  // 启动定时获取异常
    }
    respcmd11_data_set(&respcmd11); // 刷新cmd11需要的数据
    sysmon_abnormal_timer_start();    // 开启下一个周期
}
void sysmon_abnormal_timer_start(void)
{
    softtimer_start(&abnormal_timer, 3000, sysmon_abnormal_callback, &abnormal_timer); // 每3s更新一次异常状态
}


#endif

/****************************************************************************************************************************/
#if CODE_PART("协议栈初始化")

/*
    @brief      : IPMI协议栈初始化
*/
void ipmi_protocol_init(void)
{
    ipmi_i2ca.i2cbus = &i2c1_int;
    ipmi_i2cb.i2cbus = &i2c2_int;

    ipmi_i2ca.do_cmd = do_cmd_dispatch;
    ipmi_i2cb.do_cmd = do_cmd_dispatch;

    ipmi_app.ipmi_arry[ipmi_app.index++] = &ipmi_zjb;
	ipmi_app.ipmi_arry[ipmi_app.index++] = &ipmi_pwr;
	ipmi_app.ipmi_arry[ipmi_app.index++] = &ipmi_xhcl;
    ipmi_app.ipmi_arry[ipmi_app.index++] = &ipmi_jkkz;
    #if 0
    #endif
    
    ipmi_app.sub_count = ipmi_app.index;
    ipmi_app.index = IPMI_APP_START_INDEX;
    ipmi_app.is_startloop=PROCESS_CMD11;
    g_ipmi_current     = ipmi_app.ipmi_arry[ipmi_app.index];
    g_ipmi_fan_setting = &ipmi_zjb; // 设置fan句柄

    /* 启动异常状态查询定时器 */
    //sysmon_abnormal_timer_start();
}

/*
    @brief      : 更新ipmi sensor句柄
*/
void ipmi_handler_update(_ipmi_sensor_t *sensor)
{
    g_ipmi_current = sensor;
}

/*
    @brief      : ipmi协议循环
*/
static int loopFirst=0;
void ipmi_loop(void)
{
    _IPMI_SENSOR_STA ipmi_status = 0;
	static unsigned int enterCmd11Num=0;
    //delay_ms(100);
 	if(cmd22EndFirstEntercmd11or12==0)
 	{
		/* CMD11 loop */
		if(ipmi_app.is_startloop == PROCESS_CMD11)
		{
			if(loopFirst==0)
			{
				/*将cmd12时正在查询的板卡的状态置位*/
				ipmi_app.ipmi_arry[ipmi_app.index]->status=IPMI_CMD_START;
				ipmi_app.ipmi_arry[ipmi_app.index]->sensor_index=0;
				ipmi_sensortimer_stop(ipmi_app.ipmi_arry[ipmi_app.index]); 
				if(resetIpmiIndexFlag==1)
				{
					ipmi_app.index =0;
					resetIpmiIndexFlag=0;
					ipmi_app.ipmi_arry[ipmi_app.index]->sensor_index=0;
				}
				else
				{
					/*执行cmd11指令前，切换到第一张板卡,第一个外设*/
					ipmi_app.index = enterCmd11Num%2?0:ipmi_app.sub_count/2;
					//ipmi_app.index = enterCmd11Num%2?0:1;
				}
				ipmi_app.ipmi_arry[ipmi_app.index]->status=IPMI_CMD_START;
				loopFirst=1;
				enterCmd11Num=enterCmd11Num+1;
			}
			lysTemp4++;
	//	  TRACE_OUT(DEBUG_OUT, "************enter %s,%d*************\r\n", __FUNCTION__, __LINE__);
			if(cmd22RuningFlag==CMD22RUNEND)
			{			
				ipmi_status = ipmi_sensordata_get_loop(&ipmi_app);
			}
    #if 0
			if(ipmi_status == IPMI_TOTAL_OK)
			{
				ipmi_app.is_startloop = 0;
				//ipmi_app.index = IPMI_APP_START_INDEX;
				g_ipmi_current->status = IPMI_CMD_START;	// 最后一个句柄
				g_ipmi_current = ipmi_app.ipmi_arry[ipmi_app.index];	// 设置为第一个句柄
				//respcmd11_data_set(&respcmd11); // 设置cmd11的数据
			}
    #endif
		}
		
		/* CMD19 loop */
		if(ipmi_app.is_startloop == PROCESS_CMD12)
		{
			if(loopFirst==1)
			{
				/*将cmd11时正在查询的板卡的状态置位*/
				ipmi_app.ipmi_arry[ipmi_app.index]->status=IPMI_CMD_START;
				ipmi_sensortimer_stop(ipmi_app.ipmi_arry[ipmi_app.index]);
				/*执行cmd12指令前，切换到第一张板卡*/
				ipmi_app.index = 0;
				ipmi_app.ipmi_arry[ipmi_app.index]->status=IPMI_CMD_START;
				//respcmd12.index=0;//将数据在查询前置0。
				loopFirst=0;
			}
	//		  TRACE_OUT(DEBUG_OUT, "************enter %s,%d*************\r\n", __FUNCTION__, __LINE__);
			//ipmi_app.index = IPMI_APP_START_INDEX;
			lysTemp3++;
			if(cmd22RuningFlag==CMD22RUNEND)
			{
				ipmi_status = ipmi_abnormal_data_get_loop(&ipmi_app);
			}
		}

	}

    
    #if 1   /* ipmi回复报文处理 */
    ipmi_recv_deal(g_ipmi_current);
    /*lys:这个操作看起来像是，无论当前查询的是哪块板子，都要处理一下zjb的数据，因为g_ipmi_fan_setting==ipmi_zjb
			但是这几块板子的最底层缓冲buffer都是一个，应该不需要这样操作吧？？？？
	*/
    if(g_ipmi_current != &ipmi_zjb)
    {
        ipmi_recv_deal(g_ipmi_fan_setting);
    }
    #endif
}


/******************************************************************************
 * 函 数 名：reset_ipmi_index
 *
 * 函数说明: 将板卡查询，置为第一张板卡也就是转接板。保证查询完后，可以第一时间更新一次。
 * 参数说明: 
 * 输入参数: 
 * 输出参数: 
 * 返 回 值: 
 * 
 * 修改记录：todo
 * --------------------
 *	 
 ******************************************************************************/
void reset_ipmi_index()
{
	resetIpmiIndexFlag=1;
	loopFirst=0;
	ipmi_app.is_startloop = PROCESS_CMD11;
}



#endif




