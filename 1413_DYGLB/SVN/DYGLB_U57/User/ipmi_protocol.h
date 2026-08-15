/*
***********************************************************************************************************************
    @brief          : ipmi通信协议处理
    @author         : xiongjinqi
    @date           : 2024/07/25
***********************************************************************************************************************
*/
#ifndef __IPMI_PROTOCOL_H_
#define __IPMI_PROTOCOL_H_

#include "types_def.h"
#include "periph_i2c.h"
#include "softtimer.h"


#define IPMI_DATA_SIZE          128  // 数据段最大长度

#define IPMI_SENSOR_NUM         8   // 当前板卡，传感器数量

#define REQUEST_HEADER_SIZE     6   // 发送请求帧头长度 + 校验和
#define RESPONSE_HEADER_SIZE    7   // 响应数据帧头长度 + 校验和


#define IPMI_CMD14              (0x14)
#define IPMI_CMD15              (0x15)
#define IPMI_CMD16              (0x16)
#define IPMI_CMD17              (0x17)
#define IPMI_CMD18              (0x18)
#define IPMI_CMD19              (0x19)
#define IPMI_CMD20              (0x20)
#define IPMI_CMD2D              (0x2D)

#define VSO_ORGANIZATION        (0x03)

#define CMD2D_START             (0x01)
#define CMD19_START             (0x02)

/* 电源控制指令 */
#define IPMI_PWR_DISABLE        (0x00)      // 取消供电
#define IPMI_PWR_ENABLE         (0xFF)      // 开始供电

/* 电压、电流的状态 */
#define JKKZ_VOL_STATUS(x)          (x?(1<<4):0)    // x==1表示存在; x==0表示不存在
#define JKKZ_CURR_STATUS(x)         (x?1:0)         // x==1表示存在; x==0表示不存在

/*cmd14状态*/
extern int cmd14SeniorId,cmd14TurnStatus,cmd14I2cAddr,cmd14TimeroutNum;


#define GET_ZJB_VAL(name, board, buff)        \
    do{                                         \
        board->VOL_##name = U8_TO_U16(buff[1], buff[2]); \
        board->CURR_##name = U8_TO_U16(buff[3], buff[4]);    \
        board->ONOFF_##name = buff[5]>>4;       \
        TRACE_OUT(DEBUG_OUT, "Line%d: %s voltage[%d] current[%d] onoff[%d]\r\n", __LINE__, (char *)#name, \
                    board->VOL_##name, board->CURR_##name, board->ONOFF_##name); \
    }while(0)

#define GET_PWR_CMD2DVAL(name, board, buff)        \
    do{                                         \
        board->VOL_##name = U8_TO_U16(buff[1], buff[2]); \
        board->CURR_##name = U8_TO_U16(buff[3], buff[4]);    \
        board->ONOFF_##name = buff[5]>>4;       \
        TRACE_OUT(DEBUG_OUT, "Line%d: %s voltage[%d] current[%d] onoff[%d]\r\n", __LINE__, (char *)#name, \
                    board->VOL_##name, board->CURR_##name, board->ONOFF_##name); \
    }while(0)

#define GET_XHCL_CMD2DVAL(name, board, buff)        \
    do{                                         \
        board->VOL_##name = U8_TO_U16(buff[1], buff[2]); \
        board->ONOFF_##name = buff[5]>>4;       \
        TRACE_OUT(DEBUG_OUT, "Line%d: %s voltage[%d]   onoff[%d]\r\n", __LINE__, (char *)#name, \
                    board->VOL_##name, board->ONOFF_##name); \
    }while(0)

#define GET_JKKZ_CMD2DVAL(name, board, buff)        \
    do{                                         \
        board->VOL_##name = U8_TO_U16(buff[1], buff[2]); \
        board->ONOFF_##name = buff[5]>>4;       \
        TRACE_OUT(DEBUG_OUT, "***JKKZB: %s voltage[%d]  onoff[%d]\r\n",(char *)#name, \
                    board->VOL_##name, board->ONOFF_##name); \
    }while(0)




#define GET_ZJB_CMD14VAL(name, board, buff)        \
        do{                                         \
            board->VOL_##name = U8_TO_U16(buff[2], buff[3]); \
            board->CURR_##name = U8_TO_U16(buff[4], buff[5]);    \
            if((board->VOL_##name)>300){           \
                board->ONOFF_##name=1;            \
            }else{                                    \
                board->ONOFF_##name=0;              \
            }	\
        }while(0)
/*            (board->ONOFF_##name)=(board->VOL_##name)>300 ? 1 : 0;	\	
			if(((board->ONOFF_##name)!=cmd14TurnStatus)&&(cmd14TimeroutNum<4))\

			*/

#define GET_ZJB_FAN(name, board, speed, buff)        \
    do{                                         \
        speed = (buff[2]<<8) | buff[3];     \
        if(speed){                          \
            board->name##_speed = speed;    \
            board->ONOFF_##name = 1;        \
        }                                   \
        else {                              \
            board->ONOFF_##name = 0;        \
        }                                   \
    }while(0)

#define SET_ZJB_FAN_DATA(name, board, data, index)   \
    do{                                     \
        data[index++] = ID_ZJB_##name;      \
        data[index++] = board->ONOFF_##name;\
        data[index++] = 0;                  \
        data[index++] = 0;                  \
        data[index++] = H8_GET(board->name##_speed);\
        data[index++] = L8_GET(board->name##_speed);\
    }while(0)

#define SET_ZJB_PWR_DATA(name, board, data, index)     \
    do{                                     \
        data[index++] = ID_ZJB_##name;      \
        data[index++] = board->ONOFF_##name;        \
        data[index++] = H8_GET(board->VOL_##name);  \
        data[index++] = L8_GET(board->VOL_##name);  \
        data[index++] = H8_GET(board->CURR_##name); \
        data[index++] = L8_GET(board->CURR_##name); \
    }while(0)

#define SET_XHCL_VOL_DATA(name, board, data, index)     \
    do{                                 \
        data[index++] = ID_XHCL_##name; \
        data[index++] = (board->VOL_##name?1:0);  \
        data[index++] = H8_GET(board->VOL_##name);\
        data[index++] = L8_GET(board->VOL_##name);\
        data[index++] = 0;                        \
        data[index++] = 0;                        \
    }while(0)

#define SET_JKKZ_VOL_DATA(name, board, data, index)     \
    do{                                 \
        data[index++] = ID_JKKZ_##name; \
        data[index++] = (board->VOL_##name?1:0);  \
        data[index++] = H8_GET(board->VOL_##name);\
        data[index++] = L8_GET(board->VOL_##name);\
        data[index++] = 0;                        \
        data[index++] = 0;                        \
    }while(0)

#define SET_ZK_VOL_DATA(name, board, data, index)     \
    do{                                 \
        data[index++] = ID_ZK_##name;   \
        data[index++] = (board->VOL_##name?1:0);  \
        data[index++] = H8_GET(board->VOL_##name);\
        data[index++] = L8_GET(board->VOL_##name);\
        data[index++] = 0;                        \
        data[index++] = 0;                        \
    }while(0)

/* 设置异常查询的值 */
#define SET_ZK_AB_DATA(name, ab_handler, data, index)     \
    do{                                 \
        data[index++] = AB_ZKB_##name;   \
        data[index++] = H8_GET(ab_handler->AB_ZKB_##name);\
    }while(0)

#define SET_ZJB_AB_DATA(name, ab_handler, data, index)     \
    do{                                 \
        data[index++] = ID_ZJB_##name;   \
        data[index++] = H8_GET(ab_handler->AB_ZJB_##name);\
    }while(0)



#define SET_PWR_VOL_DATA(name, board, data, index)     \
    do{                                             \
        data[index++] = ID_PWR_##name;              \
        data[index++] = 1;                          \
        data[index++] = H8_GET(board->VOL_##name);  \
        data[index++] = L8_GET(board->VOL_##name);  \
        data[index++] = H8_GET(board->CURR_##name); \
        data[index++] = L8_GET(board->CURR_##name); \
    }while(0)


#define REQ_NET_FUNC            0x08
#define IPMI_SENSOR_GET_BUS(ipmi_sensor)     (ipmi_sensor->ipmi->i2cbus->dev)   // 需要传入_ipmi_sensor_t类型指针


typedef int(*do_data_func) (void *protocol, u8 *buff, u16 len);
typedef int(*do_cmd_func) (void *protocol, u8 *buff, u16 len);

typedef struct {
    s16 tempreture;
    u8  duty;
}_fan_list_t;




/* 请求数据类型定义 */
typedef struct {
    u8 dest_addr;
    u8 netfn;
    u8 frame_len;
    u8 src_addr;
    u8 req_num;
    u8 cmd;
    u8 data[IPMI_DATA_SIZE];
    u8 sum;
}_protocol_req_t    ;

/* 响应数据类型定义 */
typedef struct {
    u8 netfn;
    u8 frame_len;
    u8 src_addr;
    u8 req_num;
    u8 cmd;
    u8 is_complete; // 完成状态
    u8 data[IPMI_DATA_SIZE];
    u8 sum;
}_protocol_resp_t    ;

typedef struct {
    u16 cmd;
    do_data_func do_data;
}_do_data_t;


typedef struct {
    _i2c_interrpt_t *i2cbus;
    _protocol_req_t  request;
    _protocol_resp_t response;
    do_cmd_func do_cmd;
}_protocol_t;


typedef enum {
    IPMI_CMD_START,
    IPMI_CMD_OK,        // IPMI指令接收处理完成
    IPMI_CMD_TIMEOUT,   // IPMI指令接收超时
    IPMI_SUB_OK,        // IPMI子板完成标志
    IPMI_TOTAL_OK,
    IPMI_AB_TRANS,      // 查询异常发送完成
}_IPMI_SENSOR_STA;

/* 每个单板协议sensor轮询处理 */
typedef struct {
    _protocol_t *ipmi;  // 指定ipmi协议栈指针
    _time_t timer;
    u16     timeout;    // 超时时间
    u16     status;     // sensor 当前的状态
    u8 i2ca_addr;
    u8 i2cb_addr;
    u8 req_num;         // 帧序列号
    u8 sensor_num;      // 传感器数量
    u8 sensor_index;    // 传感器当前值
    u8 sensor_id[24];   // 传感器ID
}_ipmi_sensor_t;


/* 不同单板轮询处理 */
typedef struct {
    u8 is_startloop;    // 启用查询循环
    u8 index;      // ipmi当前指针
    u8 sub_count;  // 子卡计数
    _ipmi_sensor_t *ipmi_arry[4];   // 如果增加板卡，需要修改此数据，否则可能造成溢出
}_ipmi_app_t;

extern _protocol_t ipmi_i2ca;
extern _protocol_t ipmi_i2cb;

extern _ipmi_sensor_t ipmi_pwr;
extern _ipmi_sensor_t ipmi_jkkz;
extern _ipmi_sensor_t ipmi_xhcl;
extern _ipmi_sensor_t ipmi_zjb;

extern _ipmi_app_t ipmi_app;


/*
    @brief      : 计算接收校验和
    @param[in]  : 
            buff        数据场首地址
            len         数据场长度
    @param[out] : none
    @retval     : 检验和
*/
u8 check_recvsum(u8 *buff, u8 len);

/*
    @brief      : 计算发送校验和
    @param[in]  : 
            buff        数据场首地址
            len         数据场长度
    @param[out] : none
    @retval     : 检验和
*/
u8 check_transmitsum(u8 *buff, u8 len);


/*
    @brief      : 设置板卡是否供电
    @param[in]  :
        protocol        ipmi协议句柄指针
        slaveaddr       从机地址
        pwr_id          电源ID
        is_poweron      供电使能，0x00取消供电， 0xFF打开供电
*/
void do_reqcmd14(_ipmi_sensor_t *sensor, u8 slaveaddr, u8 pwr_id, u8 is_poweron);

/*
    @brief      : 设置风扇速度
    @param[in]  :
        protocol        ipmi协议句柄指针
        slaveaddr       从机地址
        fan_id          风扇ID
        fan_level       设置风扇转速等级(1-4)
*/
void do_reqcmd15(_ipmi_sensor_t *sensor, u8 slaveaddr, u8 fan_id, u8 fan_level);

/*
    @brief      : 获取风扇速度
    @param[in]  :
        protocol        ipmi协议句柄指针
        slaveaddr       从机地址
        pwr_id          电源ID
        is_poweron      供电使能，0x00取消供电， 0xFF打开供电
*/
void do_reqcmd16(_ipmi_sensor_t *sensor, u8 slaveaddr, u8 fan_id);

/*
    @brief      : 配置转接板电源启动上电是否打开
    @param[in]  :
        protocol        ipmi协议句柄指针
        slaveaddr       从机地址
        config          配置数组首地址
        len             数组长度
*/
void do_reqcmd17(_ipmi_sensor_t *sensor, u8 slaveaddr, u8 *config, u16 len);

/*
    @brief      : 获取转接板电源启动上电配置
    @param[in]  :
        protocol        ipmi协议句柄指针
        slaveaddr       从机地址
        config          配置数组首地址
        len             数组长度
*/
void do_reqcmd18(_ipmi_sensor_t *sensor, u8 slaveaddr);

/*
    @brief      : 获取转接板电源启动上电配置
    @param[in]  :
        protocol        ipmi协议句柄指针
        slaveaddr       从机地址
        config          配置数组首地址
        len             数组长度
*/
void do_reqcmd19(_ipmi_sensor_t *sensor, u8 slaveaddr);

/*
    @brief      : 读取设备传感器个数
        @note   : 每个传感器数量基本都是已知的,此条指令基本无用
    @param[in]  :
        protocol        ipmi协议句柄指针
        slaveaddr       从机地址
        pwr_id          电源ID
        is_poweron      供电使能，0x00取消供电， 0xFF打开供电
*/
void do_reqcmd20(_ipmi_sensor_t *sensor, u8 slaveaddr, u8 req_num);

/*
    @brief      : 循环获取温度，设置风扇占空比
                    在softtimer定时任务中调用
*/
void fan_auto_setting_loop(void);

/*
    @brief      : IPMI协议栈初始化
*/
void ipmi_protocol_init(void);

/*
    @brief      : ipmi协议循环
*/
void ipmi_loop(void);

void ipmi_handler_update(_ipmi_sensor_t *sensor);


#endif /* __IPMI_PROTOCOL_H_ */

