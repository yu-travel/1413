/*
***********************************************************************************************************************
    @brief          : 飞腾spi通信协议处理
    @author         : xiongjinqi
    @date           : 2024/07/25
***********************************************************************************************************************
*/

/* add by lkx 25.7.10 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // sleep
#include <pthread.h>
#include <errno.h>

#include "datatype.h"
#include "spi_dev.h"
#include "types_def.h"
#include "spi_protocol.h"


_spi_buffer_t   spi_dev = {0};
_spi_t          spi_protocol = {0};
_ipmi_data_t    ipmi_data = {0};
_ipmi_abnormal_t ipmi_abnormal = {0};
//pthread_t       g_spi_tid;
pthread_mutex_t g_mutex;
static u8 spi_recvbuff[1024] = {0};

#if 0
_zjb_power_ctrl_t zjb_power_ctrl_set[] = {
    {.zjb_power_id = ID_ZJB_28V_JCXJ    , .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_12V_WAOXJ   , .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_28V_BQXJ    , .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_12V_DYGY_B  , .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_28V_FFXJ    , .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_12V28V_BF1  , .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_12V28V_BF2  , .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_28V_TSGY    , .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_28V_QGSJ    , .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_12V28V_HJJC1, .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_12V28V_HJJC2, .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_12V28V_HJJC3, .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_12V_GSDJ1   , .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_12V_GSDJ2   , .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_12V_XGHTM   , .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_28V_KF      , .onoff_ctrl = 0},
};

_zjb_power_ctrl_t zjb_power_ctrl_get[16] = {
    {.zjb_power_id = ID_ZJB_28V_JCXJ    , .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_12V_WAOXJ   , .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_28V_BQXJ    , .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_12V_DYGY_B  , .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_28V_FFXJ    , .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_12V28V_BF1  , .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_12V28V_BF2  , .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_28V_TSGY    , .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_28V_QGSJ    , .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_12V28V_HJJC1, .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_12V28V_HJJC2, .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_12V28V_HJJC3, .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_12V_GSDJ1   , .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_12V_GSDJ2   , .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_12V_XGHTM   , .onoff_ctrl = 0},
    {.zjb_power_id = ID_ZJB_28V_KF      , .onoff_ctrl = 0},
};
#endif

char *abnormal_name[11] = {
    "AB_STA_DEV_OK       ",
    "AB_STA_OVERVOLTAGE  ",
    "AB_STA_UNDERVOLTAGE ",
    "AB_STA_OPT_FAIL     ",
    "AB_STA_EEPROM       ",
    "AB_STA_TEMPRETURE   ",
    "AB_STA_DEV_OVER_TEMP",
    "AB_STA_SUB_COMM     ",
    "AB_STA_OVERCURRENT	",  			
    "AB_STA_OVERCURANDOVERVOL",  		
    "AB_STA_OVERCURANDUNVOL",	
};


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
/*
****************************************************************************************************************************
    指令接收处理区
****************************************************************************************************************************
*/
#define GET_ZJB_FAN(name, board, buff, index)   \
    do{                                         \
        board->ONOFF_##name = buff[index+1];    \
        board->name##_speed = (buff[index+4]<<8) | buff[index+5];     \
        TRACE_OUT(DEBUG_OUT, "\tID_ZJB_%-*s speed[%d] onoff[%d]\r\n", \
                    15, (char*)#name, board->name##_speed, board->ONOFF_##name);\
    }while(0)

#define GET_ZJB_PWR(name, board, buff, index)        \
    do{                                              \
        board->ONOFF_##name  = buff[index+1];        \
        board->VOL_##name  = U8_TO_U16(buff[index+2], buff[index+3]);  \
        board->CURR_##name = U8_TO_U16(buff[index+4], buff[index+5]);  \
        TRACE_OUT(DEBUG_OUT, "\tID_ZJB_%-*s voltage[%d] current[%d] onoff[%d]\r\n", \
                    15, (char*)#name, board->VOL_##name, board->CURR_##name, board->ONOFF_##name); \
    }while(0)


#define GET_PWR_PWR(name, board, buff, index)        \
    do{                                              \
        board->ONOFF_##name  = buff[index+1];        \
        board->VOL_##name  = U8_TO_U16(buff[index+2], buff[index+3]);  \
        board->CURR_##name = U8_TO_U16(buff[index+4], buff[index+5]);  \
        TRACE_OUT(DEBUG_OUT, "\tID_PWR_%-*s voltage[%d] current[%d] onoff[%d]\r\n", \
                    15, (char*)#name, board->VOL_##name, board->CURR_##name, board->ONOFF_##name); \
    }while(0)
    
#define GET_PWR2_PWR(name, board, buff, index)        \
			do{ 											 \
				board->ONOFF_##name  = buff[index+1];		 \
				board->VOL_##name  = U8_TO_U16(buff[index+2], buff[index+3]);  \
				board->CURR_##name = U8_TO_U16(buff[index+4], buff[index+5]);  \
				TRACE_OUT(DEBUG_OUT, "\tID_PWR2_%-*s voltage[%d] current[%d] onoff[%d]\r\n", \
							15, (char*)#name, board->VOL_##name, board->CURR_##name, board->ONOFF_##name); \
			}while(0)


#define PRINT_SPACE           (22)

/*
    @brief         : spi响应数据解析
        @note      : 格式
                        Sensor ID(1byte, index 0) 
                        status (1byte, index 1)
                        电压值(2byte, index 2, 3)
                        电流值(2bytes, index 4, 5) / 温度值 / 转速值
    @param[in]     : 
*/
static int respcmd_data_get(u8 *buff, u16 len, _ipmi_data_t *data)
{
    int index = 0, ret = 0, status = 0;
    _pwr_monitor_t      *board_pwr  = &data->power;  // 电源板数据
    _jkkz_monitor_t     *board_jkkz = &data->jkkz;   // 接口扩展板数据
    _xhcl_monitor_t     *board_xhcl = &data->xhcl;   // 信号处理板数据
    _zjb_monitor_t      *board_zjb  = &data->zjb;    // 转接板数据
    _zkb_monitor_t      *board_zkb  = &data->zkb;    // 主控板数据
    _htm_monitor_t      *board_htm  = &data->htm;    // 哈特曼数据
	_pwr2_monitor_t		*board_pwr2	= &data->power2;  // 新增电源板数据

    if(len%6 != 0)
    {
        ret = 1;
        pr_info("response data length error: %d\r\n", len);
        return ret;
    }

    for(; index < len; index+=6)
    {
        switch(buff[index])
        {
            case ID_ZJB_FAN1:
                GET_ZJB_FAN(FAN1, board_zjb, buff, index);
            break;
            case ID_ZJB_FAN2:
                GET_ZJB_FAN(FAN2, board_zjb, buff, index);
            break;
            case ID_ZJB_FAN3:
                GET_ZJB_FAN(FAN3, board_zjb, buff, index);
            break;
            case ID_ZJB_FAN4:
                GET_ZJB_FAN(FAN4, board_zjb, buff, index);
            break;

        /****************************************************************************************************************/
            case ID_XHCL_TEMPRETURE: 
                status = buff[index+1];
                data->xhcl.tempreture = U8_TO_U16(buff[index+4], buff[index+5]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s tempreture[%d], onoff status[%d]\r\n", \
                        PRINT_SPACE, "ID_XHCL_TEMPRETURE", data->xhcl.tempreture, status);
            break;
            case ID_XHCL_VCC1V2:
                status = buff[index+1];
                data->xhcl.vcc1v2 = U8_TO_U16(buff[index+2], buff[index+3]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s voltage[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_XHCL_VCC1V2", data->xhcl.vcc1v2, status);
            break;
            case ID_XHCL_VCC1V5:
                status = buff[index+1];
                data->xhcl.vcc1v5 = U8_TO_U16(buff[index+2], buff[index+3]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s voltage[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_XHCL_VCC1V5",data->xhcl.vcc1v5, status);
            break;
            case ID_XHCL_VCC1V8:
                status = buff[index+1];
                data->xhcl.vcc1v8 = U8_TO_U16(buff[index+2], buff[index+3]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s voltage[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_XHCL_VCC1V8",data->xhcl.vcc1v8, status);
            break;
            case ID_XHCL_VCC1V0:
                status = buff[index+1];
                data->xhcl.vcc1v0 = U8_TO_U16(buff[index+2], buff[index+3]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s voltage[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_XHCL_VCC1V0",data->xhcl.vcc1v0, status);
            break;
            case ID_XHCL_VCC3V3:
                status = buff[index+1];
                data->xhcl.vcc3v3 = U8_TO_U16(buff[index+2], buff[index+3]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s voltage[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_XHCL_VCC3V3",data->xhcl.vcc3v3, status);
            break;
            case ID_XHCL_VPX12V_CURR:
                status = buff[index+1];
                data->xhcl.vpx12v_curr = U8_TO_U16(buff[index+4], buff[index+5]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s current[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_XHCL_VPX12V_CURR",data->xhcl.vpx12v_curr, status);
            break;
			case ID_XHCL_SLOT_ADDR:
                data->xhcl.slot_addr = buff[index+3];
                TRACE_OUT(DEBUG_OUT, "\t%-*s slot_addr[%d]\r\n", PRINT_SPACE, "ID_XHCL_SLOT_ADDR",data->xhcl.slot_addr);
			break;
        /****************************************************************************************************************/
            case ID_JKKZ_TEMPRETURE:
                status = buff[index+1];
                data->jkkz.tempreture = U8_TO_U16(buff[index+4], buff[index+5]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s tempreture[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_JKKZ_TEMPRETURE",data->jkkz.tempreture, status);
            break;
            case ID_JKKZ_VCC1V2:
                status = buff[index+1];
                data->jkkz.vcc1v2 = U8_TO_U16(buff[index+2], buff[index+3]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s voltage[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_JKKZ_VCC1V2",data->jkkz.vcc1v2, status);
            break;
            case ID_JKKZ_VCC1V5:
                status = buff[index+1];
                data->jkkz.vcc1v5 = U8_TO_U16(buff[index+2], buff[index+3]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s voltage[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_JKKZ_VCC1V5",data->jkkz.vcc1v5, status);
            break;
            case ID_JKKZ_VCC1V8:
                status = buff[index+1];
                data->jkkz.vcc1v8 = U8_TO_U16(buff[index+2], buff[index+3]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s voltage[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_JKKZ_VCC1V8",data->jkkz.vcc1v8, status);
            break;
            case ID_JKKZ_VCC1V0:
                status = buff[index+1];
                data->jkkz.vcc1v0 = U8_TO_U16(buff[index+2], buff[index+3]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s voltage[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_JKKZ_VCC1V0",data->jkkz.vcc1v0, status);
            break;
            case ID_JKKZ_VCC3V3:
                status = buff[index+1];
                data->jkkz.vcc3v3 = U8_TO_U16(buff[index+2], buff[index+3]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s voltage[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_JKKZ_VCC3V3",data->jkkz.vcc3v3, status);
            break;
            case ID_JKKZ_VPX12V_CURR:
                status = buff[index+1];
                data->jkkz.vpx12v_curr = U8_TO_U16(buff[index+4], buff[index+5]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s current[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_JKKZ_VPX12V_CURR",data->jkkz.vpx12v_curr, status);
            break;
        /****************************************************************************************************************/
            case ID_ZK_TEMPRETURE:
                status = buff[index+1];
                data->zkb.tempreture = U8_TO_U16(buff[index+4], buff[index+5]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s tempreture[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_ZK_TEMPRETURE",data->zkb.tempreture, status);
            break;
            case ID_ZK_VDDQ:
                status = buff[index+1];
                data->zkb.vddq = U8_TO_U16(buff[index+2], buff[index+3]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s voltage[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_ZK_VDDQ",data->zkb.vddq, status);
            break;
            case ID_ZK_VDD_CORE:
                status = buff[index+1];
                data->zkb.vdd_core = U8_TO_U16(buff[index+2], buff[index+3]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s voltage[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_ZK_VDD_CORE",data->zkb.vdd_core, status);
            break;
            case ID_ZK_VCC1V8:
                status = buff[index+1];
                data->zkb.vcc1v8 = U8_TO_U16(buff[index+2], buff[index+3]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s voltage[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_ZK_VCC1V8",data->zkb.vcc1v8, status);
            break;
            case ID_ZK_VCC2V5:
                status = buff[index+1];
                data->zkb.vcc2v5 = U8_TO_U16(buff[index+2], buff[index+3]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s voltage[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_ZK_VCC2V5",data->zkb.vcc2v5, status);
            break;
            case ID_ZK_VCC3V3:
                status = buff[index+1];
                data->zkb.vcc3v3 = U8_TO_U16(buff[index+2], buff[index+3]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s voltage[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_ZK_VCC3V3",data->zkb.vcc3v3, status);
            break;
            case ID_ZK_VCC5V0:
                status = buff[index+1];
                data->zkb.vcc5v0 = U8_TO_U16(buff[index+2], buff[index+3]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s voltage[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_ZK_VCC5V0",data->zkb.vcc5v0, status);
            break;
            case ID_ZK_VPX12V_CURR:
                status = buff[index+1];
                data->zkb.vpx12v_curr = U8_TO_U16(buff[index+4], buff[index+5]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s current[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_ZK_VPX12V_CURR",data->zkb.vpx12v_curr, status);
            break;
        /****************************************************************************************************************/
            case ID_PWR_TEMPRETURE:
                status = buff[index+1];
                data->power.tempreture = U8_TO_U16(buff[index+4], buff[index+5]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s tempreture[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_PWR_TEMPRETURE", data->power.tempreture, status);
            break;
            case ID_PWR_VCC12V_1:
                GET_PWR_PWR(VCC12V_1, board_pwr, buff, index);
            break;
            case ID_PWR_VCC12V_2:
                GET_PWR_PWR(VCC12V_2, board_pwr, buff, index);
            break;
            case ID_PWR_VCC3V3:
                GET_PWR_PWR(VCC3V3, board_pwr, buff, index);
            break;



		/****************************************************************************************************************/
            case ID_HTM_TEMPRETURE: 
                status = buff[index+1];
                data->htm.tempreture = U8_TO_U16(buff[index+4], buff[index+5]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s tempreture[%d], onoff status[%d]\r\n", \
                        PRINT_SPACE, "ID_HTM_TEMPRETURE", data->htm.tempreture, status);
            break;
            case ID_HTM_VCC1V2:
                status = buff[index+1];
                data->htm.vcc1v2 = U8_TO_U16(buff[index+2], buff[index+3]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s voltage[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_HTM_VCC1V2", data->htm.vcc1v2, status);
            break;
            case ID_HTM_VCC1V5:
                status = buff[index+1];
                data->htm.vcc1v5 = U8_TO_U16(buff[index+2], buff[index+3]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s voltage[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_HTM_VCC1V5",data->htm.vcc1v5, status);
            break;
            case ID_HTM_VCC1V8:
                status = buff[index+1];
                data->htm.vcc1v8 = U8_TO_U16(buff[index+2], buff[index+3]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s voltage[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_HTM_VCC1V8",data->htm.vcc1v8, status);
            break;
            case ID_HTM_VCC1V0:
                status = buff[index+1];
                data->htm.vcc1v0 = U8_TO_U16(buff[index+2], buff[index+3]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s voltage[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_HTM_VCC1V0",data->htm.vcc1v0, status);
            break;
            case ID_HTM_VCC3V3:
                status = buff[index+1];
                data->htm.vcc3v3 = U8_TO_U16(buff[index+2], buff[index+3]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s voltage[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_HTM_VCC3V3",data->htm.vcc3v3, status);
            break;
            case ID_HTM_VPX12V_CURR:
                status = buff[index+1];
                data->htm.vpx12v_curr = U8_TO_U16(buff[index+4], buff[index+5]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s current[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_HTM_VPX12V_CURR",data->htm.vpx12v_curr, status);
            break;
			case ID_HTM_SLOT_ADDR:
                data->htm.slot_addr = buff[index+3];
                TRACE_OUT(DEBUG_OUT, "\t%-*s slot_addr[%d]\r\n", PRINT_SPACE, "ID_HTM_SLOT_ADDR",data->htm.slot_addr);
			break;

		/****************************************************************************************************************/
            case ID_PWR2_TEMPRETURE:
                status = buff[index+1];
                data->power2.tempreture = U8_TO_U16(buff[index+4], buff[index+5]);
                TRACE_OUT(DEBUG_OUT, "\t%-*s tempreture[%d], onoff status[%d] \r\n", \
                        PRINT_SPACE, "ID_PWR2_TEMPRETURE", data->power2.tempreture, status);
            break;
            case ID_PWR2_VCC28V_1:
                GET_PWR2_PWR(VCC28V_1, board_pwr2, buff, index);
            break;
            case ID_PWR2_VCC28V_2:
                GET_PWR2_PWR(VCC28V_2, board_pwr2, buff, index);
            break;

            default:
                pr_err("invalid parameter[%02x]\r\n", buff[index]);
            break;
        }
    }

    return ret;
}

/*==============================================================================================================================*/
char *find_board_abnormal(u8 abnormal_id)
{
    int index = 0;
    if(abnormal_id==0x20)
	{
		return abnormal_name[8];
	}
	else if(abnormal_id==0x30)
	{
		return abnormal_name[9];
	}
	else if(abnormal_id==0x31)
	{
		return abnormal_name[10];
	}
	else
	{
		for(index = 0; index < 8; index++)
		{
		
			if(abnormal_id == (index+AB_STA_DEV_OK))
			{
				return abnormal_name[index];
			}
		}
	}
	return NULL;
}

#define ABNORMAL_ZJB_GET(name, abnormal, value)   \
    do{ \
        abnormal->AB_ZJB_##name = value; \
        TRACE_OUT(DEBUG_OUT, "\tAB_ZJB_%-*s status[ %s ]\r\n",22, (char *)#name, \
            find_board_abnormal(abnormal->AB_ZJB_##name));   \
    }while(0)

#define ABNORMAL_PWR_GET(name, abnormal, value)   \
    do{ \
        abnormal->AB_PWR_##name = value; \
        TRACE_OUT(DEBUG_OUT, "\tAB_PWR_%-*s status[ %s ]\r\n",22, (char *)#name, \
            find_board_abnormal(abnormal->AB_PWR_##name));   \
    }while(0)

#define ABNORMAL_PWR2_GET(name, abnormal, value)   \
		do{ \
			abnormal->AB_PWR2_##name = value; \
			TRACE_OUT(DEBUG_OUT, "\tAB_PWR2_%-*s status[ %s ]\r\n",22, (char *)#name, \
				find_board_abnormal(abnormal->AB_PWR2_##name));	 \
		}while(0)


#define ABNORMAL_JKKZ_GET(name, abnormal, value)   \
    do{ \
        abnormal->AB_JKKZ_##name = value; \
        TRACE_OUT(DEBUG_OUT, "\tAB_JKKZ_%-*s status[ %s ]\r\n",22, (char *)#name, \
            find_board_abnormal(abnormal->AB_JKKZ_##name));   \
    }while(0)

#define ABNORMAL_XHCL_GET(name, abnormal, value)   \
    do{ \
        abnormal->AB_XHCL_##name = value; \
        TRACE_OUT(DEBUG_OUT, "\tAB_XHCL_%-*s status[ %s ]\r\n",22, (char *)#name, \
            find_board_abnormal(abnormal->AB_XHCL_##name));   \
    }while(0)

#define ABNORMAL_ZK_GET(name, abnormal, value)   \
    do{ \
        abnormal->AB_ZK_##name = value; \
        TRACE_OUT(DEBUG_OUT, "\tAB_ZK_%-*s status[ %s ]\r\n",22, (char *)#name, \
            find_board_abnormal(abnormal->AB_ZK_##name));   \
    }while(0)
    
#define ABNORMAL_HTM_GET(name, abnormal, value)   \
			do{ \
				abnormal->AB_HTM_##name = value; \
				TRACE_OUT(DEBUG_OUT, "\tAB_HTM_%-*s status[ %s ]\r\n",22, (char *)#name, \
					find_board_abnormal(abnormal->AB_HTM_##name));   \
			}while(0)


/*
    @brief         : spi响应数据解析
        @note      : 格式
                        Sensor ID(1byte, index 0) 
                        status (1byte, index 1)
                        电压值(2byte, index 2, 3)
                        电流值(2bytes, index 4, 5) / 温度值 / 转速值
    @param[in]     : 
*/
static int respcmd12_data_get(u8 *buff, u16 len, _ipmi_abnormal_t *abnormal)
{
    int index = 0, ret = 0, status = 0;

    if(len%2 != 0)
    {
        ret = 1;
        pr_info("response data length error: %d\r\n", len);
        return ret;
    }

    for(; index < len; index+=2)
    {
        switch(buff[index])
        {
            case AB_ID_ZJB_SUB_COMM:
                ABNORMAL_ZJB_GET(SUB_COMM, abnormal, buff[index+1]);
            break;

        /****************************************************************************************************************/
            case AB_ID_XHCL_EEPROM:
                ABNORMAL_XHCL_GET(EEPROM, abnormal, buff[index+1]);
            break;
            case AB_ID_XHCL_SUB_COMM:
                ABNORMAL_XHCL_GET(SUB_COMM, abnormal, buff[index+1]);
            break;
            case ID_XHCL_TEMPRETURE: 
                ABNORMAL_XHCL_GET(TEMPRETURE, abnormal, buff[index+1]);
            break;
            case ID_XHCL_VCC1V2:
                ABNORMAL_XHCL_GET(VCC1V2, abnormal, buff[index+1]);
            break;
            case ID_XHCL_VCC1V5:
                ABNORMAL_XHCL_GET(VCC1V5, abnormal, buff[index+1]);
            break;
            case ID_XHCL_VCC1V8:
                ABNORMAL_XHCL_GET(VCC1V8, abnormal, buff[index+1]);
            break;
            case ID_XHCL_VCC1V0:
                ABNORMAL_XHCL_GET(VCC1V0, abnormal, buff[index+1]);
            break;
            case ID_XHCL_VCC3V3:
                ABNORMAL_XHCL_GET(VCC3V3, abnormal, buff[index+1]);
            break;

        /****************************************************************************************************************/
            case AB_ID_JKKZ_EEPROM:
                ABNORMAL_JKKZ_GET(EEPROM, abnormal, buff[index+1]);
            break;
            case AB_ID_JKKZ_SUB_COMM:
                ABNORMAL_JKKZ_GET(SUB_COMM, abnormal, buff[index+1]);
            break;
            case ID_JKKZ_TEMPRETURE:
                ABNORMAL_JKKZ_GET(TEMPRETURE, abnormal, buff[index+1]);
            break;
            case ID_JKKZ_VCC1V2:
                ABNORMAL_JKKZ_GET(VCC1V2, abnormal, buff[index+1]);
            break;
            case ID_JKKZ_VCC1V5:
                ABNORMAL_JKKZ_GET(VCC1V5, abnormal, buff[index+1]);
            break;
            case ID_JKKZ_VCC1V8:
                ABNORMAL_JKKZ_GET(VCC1V8, abnormal, buff[index+1]);
            break;
            case ID_JKKZ_VCC1V0:
                ABNORMAL_JKKZ_GET(VCC1V0, abnormal, buff[index+1]);
            break;
            case ID_JKKZ_VCC3V3:
                ABNORMAL_JKKZ_GET(VCC3V3, abnormal, buff[index+1]);
            break;
        /****************************************************************************************************************/
            case ID_ZK_TEMPRETURE:
                abnormal->AB_BOX_OVER_TEMPRETURE = buff[index+1];
            break;
            case ID_ZK_VDDQ:
                ABNORMAL_ZK_GET(VDDQ, abnormal, buff[index+1]);
            break;
            case ID_ZK_VDD_CORE:
                ABNORMAL_ZK_GET(VDD_CORE, abnormal, buff[index+1]);
            break;
            case ID_ZK_VCC1V8:
                ABNORMAL_ZK_GET(VCC1V8, abnormal, buff[index+1]);
            break;
            case ID_ZK_VCC2V5:
                ABNORMAL_ZK_GET(VCC2V5, abnormal, buff[index+1]);
            break;
            case ID_ZK_VCC3V3:
                ABNORMAL_ZK_GET(VCC3V3, abnormal, buff[index+1]);
            break;
            case ID_ZK_VCC5V0:
                ABNORMAL_ZK_GET(VCC5V0, abnormal, buff[index+1]);
            break;

        /****************************************************************************************************************/
            case AB_ID_PWR_EEPROM:
                ABNORMAL_PWR_GET(EEPROM, abnormal, buff[index+1]);
            break;
            case AB_ID_PWR_SUB_COMM:
                ABNORMAL_PWR_GET(SUB_COMM, abnormal, buff[index+1]);
            break;
            case ID_PWR_TEMPRETURE:
                ABNORMAL_PWR_GET(TEMPRETURE, abnormal, buff[index+1]);
            break;
            case ID_PWR_VCC12V_1:
                ABNORMAL_PWR_GET(VCC12V_1, abnormal, buff[index+1]);
            break;
            case ID_PWR_VCC12V_2:
                ABNORMAL_PWR_GET(VCC12V_2, abnormal, buff[index+1]);
            break;
            case ID_PWR_VCC3V3:
                ABNORMAL_PWR_GET(VCC3V3, abnormal, buff[index+1]);
            break;
			
		/****************************************************************************************************************/
			case AB_ID_HTM_EEPROM:
				ABNORMAL_HTM_GET(EEPROM, abnormal, buff[index+1]);
			break;
			case AB_ID_HTM_SUB_COMM:
				ABNORMAL_HTM_GET(SUB_COMM, abnormal, buff[index+1]);
			break;
			case ID_HTM_TEMPRETURE: 
				ABNORMAL_HTM_GET(TEMPRETURE, abnormal, buff[index+1]);
			break;
			case ID_HTM_VCC1V2:
				ABNORMAL_HTM_GET(VCC1V2, abnormal, buff[index+1]);
			break;
			case ID_HTM_VCC1V5:
				ABNORMAL_HTM_GET(VCC1V5, abnormal, buff[index+1]);
			break;
			case ID_HTM_VCC1V8:
				ABNORMAL_HTM_GET(VCC1V8, abnormal, buff[index+1]);
			break;
			case ID_HTM_VCC1V0:
				ABNORMAL_HTM_GET(VCC1V0, abnormal, buff[index+1]);
			break;
			case ID_HTM_VCC3V3:
				ABNORMAL_HTM_GET(VCC3V3, abnormal, buff[index+1]);
			break;
		/****************************************************************************************************************/
			case AB_ID_PWR2_EEPROM:
				ABNORMAL_PWR2_GET(EEPROM, abnormal, buff[index+1]);
			break;
			case AB_ID_PWR2_SUB_COMM:
				ABNORMAL_PWR2_GET(SUB_COMM, abnormal, buff[index+1]);
			break;
			case ID_PWR2_TEMPRETURE:
				ABNORMAL_PWR2_GET(TEMPRETURE, abnormal, buff[index+1]);
			break;
			case ID_PWR2_VCC28V_1:
				ABNORMAL_PWR2_GET(VCC28V_1, abnormal, buff[index+1]);
			break;
			case ID_PWR2_VCC28V_2:
				ABNORMAL_PWR2_GET(VCC28V_2, abnormal, buff[index+1]);
			break;

            default:
                pr_err("invalid parameter[%02x]\r\n", buff[index]);
            break;
        }
    }
    return ret;
}

/*
    @brief         : 打印电源风扇控制及状态
*/
static void print_power_control(u8 sensor_id, u8 onoff)
{
    switch(sensor_id) 
    {
        case ID_ZJB_FAN1:
            TRACE_OUT(TEST_ENABLE, "\t%-*s CTRL: %d\r\n", 24, "ID_ZJB_FAN1", onoff);
        break;
        case ID_ZJB_FAN2:
            TRACE_OUT(TEST_ENABLE, "\t%-*s CTRL: %d\r\n", 24, "ID_ZJB_FAN2", onoff);
        break;
        case ID_ZJB_FAN3:
            TRACE_OUT(TEST_ENABLE, "\t%-*s CTRL: %d\r\n", 24, "ID_ZJB_FAN3", onoff);
        break;
        case ID_ZJB_FAN4:
            TRACE_OUT(TEST_ENABLE, "\t%-*s CTRL: %d\r\n", 24, "ID_ZJB_FAN4", onoff);
        break;
        default:
            TRACE_OUT(TEST_ENABLE,"Line%d: invalid parameter[%02x]\r\n", __LINE__, sensor_id);
        break;
    }
}

/*
***********************************************************************************************************************************
    响应数据处理函数
***********************************************************************************************************************************
*/
#if CODE_PART("响应数据处理")

static int do_respcmd11 (void *protocol_arg, u8 *buff, u16 len)
{
    int ret = 0;

    pr_info("%s() Line%d: response data len[%d]\r\n", __FUNCTION__, __LINE__, len);

    if(len)
    {
        ret = respcmd_data_get(buff, len, &ipmi_data);
    }

    return 0;
}

static int do_respcmd12 (void *protocol_arg, u8 *buff, u16 len)
{
    int ret = 0;

    pr_info("%s() Line%d: response data len[%d]\r\n", __FUNCTION__, __LINE__, len);

    if(len)
    {
        ret = respcmd12_data_get(buff, len, &ipmi_abnormal);
    }

    return 0;
}
#if 0
static int do_respcmd13 (void *protocol_arg, u8 *buff, u16 len)
{
    int ret = 0, index = 0;

    pr_info("%s() Line%d: response data len[%d]\r\n", __FUNCTION__, __LINE__, len);

    if(len%16 == 0)
    {
        for(index=0; index<len/2; index++)
        {
            if(zjb_power_ctrl_get[index].zjb_power_id == buff[index*2])
            {
                zjb_power_ctrl_get[index].onoff_ctrl = buff[index*2+1];
                print_power_control(buff[index*2], buff[index*2+1]);
            }
        }
    }

    return 0;
}

static int do_respcmd22 (void *protocol_arg, u8 *buff, u16 len)
{
    int ret = 0;

    pr_info("%s() Line%d: response data len[%d]\r\n", __FUNCTION__, __LINE__, len);

    if(len)
    {
        ret = respcmd_data_get(buff, len, &ipmi_data);
    }

    return ret;
}

/*
    @brief      : 切换总线指令响应处理
*/
static int do_respcmd23 (void *protocol_arg, u8 *buff, u16 len)
{
    pr_info("%s() Line%d: response data len[%d]\r\n", __FUNCTION__, __LINE__, len);

    if(len)
    {
        return buff[0];
    }
}

/*
    @brief      : 配置转接板默认启动上电响应处理
*/
static int do_respcmd25 (void *protocol_arg, u8 *buff, u16 len)
{
    pr_info("%s() Line%d: response data len[%d]\r\n", __FUNCTION__, __LINE__, len);

    if(len)
    {
        return buff[0];
    }
}
#endif

_do_data_t do_spicmd[] = {
    {SPI_CMD11, do_respcmd11},
    {SPI_CMD12, do_respcmd12},
//    {SPI_CMD13, do_respcmd13},
//
//    {SPI_CMD22, do_respcmd22},
//    {SPI_CMD23, do_respcmd23},
//    {SPI_CMD25, do_respcmd25},

};

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
    int index = 0, ret = 1;
    u8 recv_checksum = 0, calc_checksum = 0;
    u8 recv_checksum2 = 0;
    u16 recv_len = 0;

    recv_checksum = buff[len-1];
    recv_checksum2 = buff[len];
    pr_info("Line%d: dispatch len[%d] recv_sum[%02x] recv_sum2[%02x]\r\n", __LINE__, len, recv_checksum, recv_checksum2);
    /* 1、检验数据 */
    calc_checksum = check_recvsum(buff+3, len-3);
    if(recv_checksum == calc_checksum || recv_checksum2 == calc_checksum)
    {
        /* 1、处理指令,并发送响应 */
        for(index = 0; index < ARRAY_SIZE(do_spicmd); index++)
        {
            if(do_spicmd[index].cmd == buff[3])
                ret = do_spicmd[index].do_data(handler, buff+4, len-5); // 4字节是数据域的头
        }
    }
    else
    {
        pr_info("Check sum fail, CALC[%02x] != FRAM[%02x]\r\n", calc_checksum, recv_checksum);
    }
    return ret;
}


/* 
    @brief      : spi数据处理
    @param[in]  : 
            handler         spi协议句柄指针
    @param[out] : none
    @retval     : 0成功；1失败
*/
static int spi_recv_deal(_spi_t *handler)
{
    int ret = 1, recv_len = 0;
    u8 temp = 0, len_h = 0, len_l = 0;

    memset(spi_recvbuff, 0, sizeof(u8)*1024);   // 清数据缓冲
    ret = ring_buffer_peek(&handler->spi->rb_handler, (char *)&temp, 0);
    //pr_info("%s() Line%d: get header[%02x]\r\n", __FUNCTION__, __LINE__, temp);
    if(temp == SPI_HEADER)
    {
        pr_info("Line%d: get spi frame header[%02x]\r\n", __LINE__, temp);

        ret = ring_buffer_peek(&handler->spi->rb_handler, (char *)&len_h, 1); // 获取当前帧长度
        ret = ring_buffer_peek(&handler->spi->rb_handler, (char *)&len_l, 2); 
        if(ret == 1)
        {
            recv_len = U8_TO_U16(len_h, len_l);
            pr_info("Line%d: get spi frame length[%d]; len_h[%02x] len_l[%02x]\r\n", __LINE__, recv_len, len_h, len_l);

            if(ring_buffer_num_items(&handler->spi->rb_handler) >= recv_len) // 接收满一帧数据
            {
                ret = ring_buffer_dequeue_arr(&handler->spi->rb_handler, (char *)spi_recvbuff, recv_len);
                if(ret != recv_len)
                {
                    pr_info("Read ring buffer fail \r\n");
                    ret = 1;
                }
                else
                {
                    ret = handler->do_cmd(handler, spi_recvbuff, recv_len);
                }
            }
        }
    }
    else
    {
        /* 出栈，脏数据 */
        //pr_info("%s() Line%d: remove dirty data[%02x]\r\n", __FUNCTION__, __LINE__, temp);
        ring_buffer_dequeue(&handler->spi->rb_handler, (char *)&temp);
        ret = 1;
    }
    return ret;
}

/*
    @brief        : 查找当前接收数据是否有帧头，并获取数据帧长度
*/
static int spi_protocol_find_header(u8 *buffer, int len, int *data_len)
{
    int index = 0, unframe_len = 0;
    int ret = 0;

    for(index = 0; index < len; index++)
    {
        if((buffer[index] == SPI_HEADER) && (buffer[index + 1] != SPI_HEADER))
        {
            ret = index;
            unframe_len = U8_TO_U16(buffer[index+1], buffer[index+2]);
            *data_len = unframe_len;
            pr_info("Line%d: Find header at index[%d] data len[%d]\n", __LINE__, index, unframe_len);
            break;
        }
    }
    
    return ret;
}

/* 
    @brief      : 指令发送后接收循环，限时1s
    @param[in]  : 
            handler         spi数据处理句柄
    @param[out] : none
    @retval     : 0成功，1失败
*/
static int spi_protocol_receive_loop(_spi_t *handler)
{
    int ret = 1;
    int time_count = 0, recv_len = 0, data_len = 0;
    u8 *buffer = NULL;

    /* 分配数据缓冲区 */
	buffer = (u8*)malloc(SPI_DATA_SIZE);
    memset(buffer, 0, SPI_DATA_SIZE);

    handler->status = SPI_STATUS_RECEIVE;   /* 设置句柄为接收状态 */
    while((time_count < SPI_RECEIVE_CYCLE) && (handler->status != SPI_STATUS_IDLE))  	// 超时时间1s && (ret != 1)
	{
		recv_len = 0;
        memset(buffer, 0, SPI_DATA_SIZE);
		usleep(SPI_RECEIVE_WAIT);

        pthread_mutex_lock(&g_mutex);
        recv_len = spi_read_bytes(handler->spi->fd, buffer, SPI_DATA_SIZE);
        
		if(recv_len < 0)
		{
			pr_err("read error! ret: %d\n", recv_len);
		}
		else
		{
            /*
                1、由于接收的数据包全部压入buffer, 会丢失第一个字节；
                因此先压入第一个字节，再压入剩余字节
                2、CPLD会保持电平，读到很多0x00, 因此对读到数据做初步过滤
            */
            ret = spi_protocol_find_header(buffer, recv_len, &data_len);
            if(data_len != 0)
            {
                #if DEBUG_OUT
                PRINT_ARRAY(buffer, recv_len, SPI_READ_ARRY_OPT);
                #endif
                pr_info("Line%d: push ring buffer, start index[%d] len[%d]\n", __LINE__, ret, data_len+1);

                ring_buffer_queue_arr(&handler->spi->rb_handler, buffer+ret, data_len+1);
                data_len = 0;
            }
		}
        #if 0
        pr_info("==================================================================\r\n");
        memset(buffer, 0, SPI_DATA_SIZE);
        ring_buffer_dequeue_arr(&handler->spi->rb_handler, buffer, SPI_DATA_SIZE);
        PRINT_ARRAY(buffer, SPI_DATA_SIZE, SPI_READ_ARRY_OPT);
        pr_info("==================================================================\r\n");
        pr_info("%s() Line%d: handler status[%d]\r\n", __FUNCTION__, __LINE__, handler->status);
        #endif
        ret = spi_recv_deal(handler);
        if(ret == 0)
        {
            pr_info("%s() Line%d: change HANDLER[%p] as status[%d]\r\n", __FUNCTION__, __LINE__, handler, SPI_STATUS_IDLE);
            handler->status = SPI_STATUS_IDLE;
        }
        pthread_mutex_unlock(&g_mutex);
        time_count++;
    }

    if(time_count >= SPI_RECEIVE_CYCLE)
    {
        pr_info("%s() Line%d: timeout waiting [%d]*[%d]=[%d]us\r\n", __FUNCTION__, __LINE__, \
                    time_count, SPI_RECEIVE_WAIT, time_count*SPI_RECEIVE_WAIT);
        ret = 1;
        handler->status = SPI_STATUS_ERROR;
    }
    else    /* 在1s内收到数据 */
    {
        ret = 0;
    }

    free(buffer);
    return ret;
}

#endif /* 相应数据处理 */

/*
***********************************************************************************************************************************
    指令发送函数
***********************************************************************************************************************************
*/
static int spi_protocol_transmit( _spi_protocol_t *handler, _spi_buffer_t *spi)
{
    u16 index = 0, data_len = 0;

    data_len = handler->length - SPI_HEADER_SIZE;
    memset(spi->txbuff, 0, handler->length);
    
    spi->txbuff[index++] = handler->header;
    spi->txbuff[index++] = H8_GET(handler->length);
    spi->txbuff[index++] = L8_GET(handler->length);
    spi->txbuff[index++] = handler->cmd;
    if(data_len > 0)
    {
        memcpy(spi->txbuff+index, handler->data, data_len);
        index += data_len;
    }
    spi->txbuff[index++] = check_transmitsum(spi->txbuff+3, index-3); // 计算内容字段校验和
    spi->txcount = index;

    spi_write_bytes(spi->fd, spi->txbuff, spi->txcount);
    
    return 0;
}

/* 
    @brief      : cmd11指令发送函数,查询ipmi数据
    @param[in]  : 
            handler         spi数据处理句柄
    @param[out] : none
    @retval     : 0成功，1失败
*/
int do_reqcmd11(_spi_t *handler)
{
    int ret = 1;
    
    /* 1、发送指令 */
    handler->resquest.header = SPI_HEADER;
    handler->resquest.cmd    = SPI_CMD11;
    handler->resquest.length = SPI_HEADER_SIZE;
    ret = spi_protocol_transmit(&handler->resquest, handler->spi);

    /* 间隔1s后读取数据，并处理数据 */
    //usleep(1000);  // sleep 1ms

    ret = spi_protocol_receive_loop(handler);   // 接收循环，限时1s

    return ret;
}

/* 
    @brief      : cmd12指令发送函数,查询异常情况
        @note   : 定时执行，查询是否有异常
    @param[in]  : 
            handler         spi数据处理句柄
    @param[out] : none
    @retval     : 0成功，1失败
*/
int do_reqcmd12(_spi_t *handler)
{
    int ret = 1;
    
    /* 1、发送指令 */
    handler->resquest.header = SPI_HEADER;
    handler->resquest.cmd    = SPI_CMD12;
    handler->resquest.length = SPI_HEADER_SIZE;
    ret = spi_protocol_transmit(&handler->resquest, handler->spi);

    /* 间隔1s后读取数据，并处理数据 */
    //usleep(100000);  // sleep 100ms

    ret = spi_protocol_receive_loop(handler);   // 接收循环，限时1s

    return ret;
}


#if 0
/* 
    @brief      : cmd13指令发送函数,查询转接板默认启动上电 
            zjb_power_ctrl_get是获取转接板默认启动上电状态句柄，
            POWER_OFF表示上电关闭，POWER_ON表示上电打开，
            用法参考demo.c文件，
            获取电源启动上电状态顺序固定（不可改变），如下：
                ID_ZJB_28V_JCXJ    
                ID_ZJB_12V_WAOXJ   
                ID_ZJB_28V_BQXJ    
                ID_ZJB_12V_DYGY_B  
                ID_ZJB_28V_FFXJ    
                ID_ZJB_12V28V_BF1  
                ID_ZJB_12V28V_BF2  
                ID_ZJB_28V_TSGY    
                ID_ZJB_28V_QGSJ    
                ID_ZJB_12V28V_HJJC1
                ID_ZJB_12V28V_HJJC2
                ID_ZJB_12V28V_HJJC3
                ID_ZJB_12V_GSDJ1   
                ID_ZJB_12V_GSDJ2   
                ID_ZJB_12V_XGHTM   
                ID_ZJB_28V_KF      
    @param[in]  : 
            handler         spi数据处理句柄
    @param[out] : none
    @retval     : 0成功，1失败
*/
int do_reqcmd13(_spi_t *handler)
{
    int ret = 1;
    
    /* 1、发送指令 */
    handler->resquest.header = SPI_HEADER;
    handler->resquest.cmd    = SPI_CMD13;
    handler->resquest.length = SPI_HEADER_SIZE;
    ret = spi_protocol_transmit(&handler->resquest, handler->spi);

    /* 间隔1s后读取数据，并处理数据 */
    //usleep(100000);  // sleep 100ms

    ret = spi_protocol_receive_loop(handler);   // 接收循环，限时1s

    return ret;
}

/* 
    @brief      : cmd22指令发送函数
    @param[in]  : 
            handler         spi数据处理句柄
            sensorid        sensor ID,详见datatype.h
            status          电源是否使能/风扇转速等级
                                POWER_OFF
                                POWER_ON
                                FAN_LEVEL_1
                                FAN_LEVEL_2
                                FAN_LEVEL_3
                                FAN_LEVEL_4
    @param[out] : none
    @retval     : 0成功；1失败
*/
int do_reqcmd22(_spi_t *handler, u8 sensorid, u8 status)
{
    int ret = 1;

    TRACE_OUT(TEST_ENABLE, "do_reqcmd22: sensorID=%02x onoff=%d\r\n", sensorid, status);

    /* 1、发送指令 */
    handler->resquest.header = SPI_HEADER;
    handler->resquest.cmd    = SPI_CMD22;
    handler->resquest.data[0] = sensorid;
    handler->resquest.data[1] = status;
    handler->resquest.length = 2+SPI_HEADER_SIZE;
    ret = spi_protocol_transmit(&handler->resquest, handler->spi);
    
    /* 2、打印命令结果 */
    print_power_control(sensorid, status);

    /* 间隔1s后读取数据，并处理数据 */
    //usleep(230000);  // sleep 230ms

    ret = spi_protocol_receive_loop(handler);   // 接收循环，限时1s

    return ret;
}


/* 
    @brief      : cmd22指令发送函数
    @param[in]  : 
            handler     spi数据处理句柄
            busid       ipmi总线ID:
                            IPMI_BUSA
                            IPMI_BUSB
    @param[out] : none
    @retval     : 0成功，1失败
*/
int do_reqcmd23(_spi_t *handler, u8 busid)
{
    int ret = 1;

    TRACE_OUT(TEST_ENABLE, "do_reqcmd23: busid=%d\r\n", busid);

    /* 1、发送指令 */
    handler->resquest.header = SPI_HEADER;
    handler->resquest.cmd    = SPI_CMD23;
    handler->resquest.data[0] = busid;
    handler->resquest.length = 1+SPI_HEADER_SIZE;
    ret = spi_protocol_transmit(&handler->resquest, handler->spi);

    /* 间隔1s后读取数据，并处理数据 */
    //usleep(10000);  // sleep 10ms

    ret = spi_protocol_receive_loop(handler);   // 接收循环，限时1s

    return ret;
}


/* 
    @brief      : cmd25指令发送函数,配置转接板默认启动上电 
    @param[in]  : 
        handler     spi数据处理句柄
        onoff_buff  电源开关控制buff首地址，电源顺序固定不可修改，如下所示：
                    ID_ZJB_28V_JCXJ    
                    ID_ZJB_12V_WAOXJ   
                    ID_ZJB_28V_BQXJ    
                    ID_ZJB_12V_DYGY_B  
                    ID_ZJB_28V_FFXJ    
                    ID_ZJB_12V28V_BF1  
                    ID_ZJB_12V28V_BF2  
                    ID_ZJB_28V_TSGY    
                    ID_ZJB_28V_QGSJ    
                    ID_ZJB_12V28V_HJJC1
                    ID_ZJB_12V28V_HJJC2
                    ID_ZJB_12V28V_HJJC3
                    ID_ZJB_12V_GSDJ1   
                    ID_ZJB_12V_GSDJ2   
                    ID_ZJB_12V_XGHTM   
                    ID_ZJB_28V_KF   

        len         电源开关控制buff长度    
    @param[out] : none
    @retval     : 0成功，1失败
*/
int do_reqcmd25(_spi_t *handler, u8 *onoff_buff, u16 len)
{
    int ret = 1, index = 0;
    u8 power_ctrl[32] = {0};
    TRACE_OUT(TEST_ENABLE, "do_reqcmd25: len=%d\r\n", len);

    if(len != 16)
    {
        pr_err("%s() Line%d: len is not 16\r\n", __FUNCTION__, __LINE__);
        return ret;
    }

    /* 1、发送指令 */
    handler->resquest.header = SPI_HEADER;
    handler->resquest.cmd    = SPI_CMD25;
    for(index = 0; index < len; index++)
    {
        zjb_power_ctrl_set[index].onoff_ctrl= onoff_buff[index];
        handler->resquest.data[index*2]     = zjb_power_ctrl_set[index].zjb_power_id;
        handler->resquest.data[index*2+1]   = zjb_power_ctrl_set[index].onoff_ctrl;
    }
    handler->resquest.length = 32+SPI_HEADER_SIZE;
    ret = spi_protocol_transmit(&handler->resquest, handler->spi);

    /* 间隔1s后读取数据，并处理数据 */
    //usleep(10000);  // sleep 10ms

    ret = spi_protocol_receive_loop(handler);   // 接收循环，限时1s

    return ret;
}
#endif

void *spi_protocol_unframe_thread(void *arg)
{
    int ret = 0;
    _spi_t *handler = (_spi_t *)arg;;
    pr_info("%s() Line%d: none\r\n", __FUNCTION__, __LINE__);

    while(1)
    {
        /* 1、正常状态 */
        if(handler->status == SPI_STATUS_IDLE)
        {
            sleep(1);
        }
        else if(handler->status == SPI_STATUS_RECEIVE) /* 2、接收状态处理 */
        {
            pthread_mutex_lock(&g_mutex);
            ret = spi_recv_deal(handler);
            if(ret == 0)
            {
                pr_info("%s() Line%d: change HANDLER[%p] as status[%d]\r\n", __FUNCTION__, __LINE__, handler, SPI_STATUS_IDLE);
                handler->status = SPI_STATUS_IDLE;
            }
            pthread_mutex_unlock(&g_mutex);
            usleep(1000);   // 休眠1ms
        }
        else if(handler->status == SPI_STATUS_ERROR)    /* 3、超时后错误状态处理 */
        {
            pr_info("%s() Line%d: HANDLER[%p] in error status[%d], and modify to idle status\r\n", \
                __FUNCTION__, __LINE__, handler, handler->status);
            handler->status = SPI_STATUS_IDLE;
        }
    }
}

/*
    @brief      : SPI协议栈初始化
    @retval     : 0,成功； -1失败
*/
int spi_protocol_init(_spi_t *handler)
{
    int ret = 0;
    pr_info("%s() Line%d: none\r\n", __FUNCTION__, __LINE__);

    /* 1、分配RX缓冲区 */
    #if 0
    handler->spi->rxbuff = (u8*)malloc(sizeof(u8)*SPI_DATA_SIZE*10);
    memset(handler->spi->rxbuff, 0, SPI_DATA_SIZE*10);
    //memset(handler->spi->rxbuff, 0, SPI_DATA_SIZE);
    handler->spi->rxbuff = rxbuffer;
    pr_info("%s() Line%d: ringbuffer[%p]\r\n", __FUNCTION__, __LINE__, rxbuffer);
    #endif
    
    /* 2、SPI设备打开并初始化 */
    spi_dev.fd = spi_dev_open(SPI_DEVICE);
    if(spi_dev.fd < 0)
    {
        pabort("can't open device");
        return -1;
    }

    handler->spi = &spi_dev;
    handler->do_cmd = do_spicmd_dispatch;
    
    /* 3、初始化ringbuffer */
    ring_buffer_init(&handler->spi->rb_handler, (char *)handler->spi->rxbuff, SPI_DATA_SIZE*10);

    /* 4、创建解析线程 */
    handler->status == SPI_STATUS_IDLE;
    ret = pthread_mutex_init(&g_mutex, NULL);
	if(ret != 0) {
		perror("pthread_mutex_init");
		return -1;
	}
    #if 0
	ret = pthread_create(&g_spi_tid,NULL, spi_protocol_unframe_thread, (void *)handler);
	if(ret != 0)
    {
		perror("pthread_create");
		return -1;
	}
    #endif
    return 0;
}

/*
    @brief      : SPI协议栈关闭
*/
int spi_protocol_close(_spi_t *handler)
{
    int ret = 0;
    pr_info("%s() Line%d: none\r\n", __FUNCTION__, __LINE__);

    //free(handler->spi->rxbuff);     // 释放接收缓冲区
    spi_dev_close(handler->spi->fd);  // 关闭spi设备
    pthread_mutex_destroy(&g_mutex);
    //pthread_join(g_spi_tid, NULL);

    return ret;
}

