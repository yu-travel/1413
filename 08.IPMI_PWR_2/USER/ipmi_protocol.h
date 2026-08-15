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


#define IPMI_DATA_SIZE          70  // 数据段最大长度

#define IPMI_SENSOR_NUM         4   // 当前板卡，传感器数量

#define REQUEST_HEADER_SIZE     6
#define RESPONSE_HEADER_SIZE    7

#define IPMI_CMD14              (0x14)
#define IPMI_CMD15              (0x15)
#define IPMI_CMD16              (0x16)
#define IPMI_CMD17              (0x17)
#define IPMI_CMD18              (0x18)
#define IPMI_CMD19              (0x19)
#define IPMI_CMD20              (0x20)
#define IPMI_CMD2D              (0x2D)

#define VSO_ORGANIZATION        (0X03)

/* 电源控制指令 */
#define IPMI_PWR_DISABLE        (0x00)      // 取消供电
#define IPMI_PWR_ENABLE         (0xFF)      // 开始供电

/* 电压、电流的状态 */
#define CMD2D_ONOFF_STATUS(x)       (x?(1<<4):0)
#define CMD2D_VOL_STATUS(x)         (x?(1<<2):0)    // 
#define CMD2D_CURR_STATUS(x)        (x?1:0)         // 

typedef int(*do_data_func) (void *protocol, u8 *buff, u16 len);
typedef int(*do_cmd_func) (void *protocol, u8 *buff, u16 len);


/* 请求数据类型定义 */
typedef struct {
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
    u8 dest_addr;
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
    _protocol_resp_t response;
    do_cmd_func do_cmd;
}_protocol_t;


/*
    @brief      : IPMI协议栈初始化
*/
void ipmi_protocol_init(void);

/*
    @brief      : ipmi协议循环
*/
void ipmi_loop(void);



#endif /* __IPMI_PROTOCOL_H_ */

