/*
***********************************************************************************************************************
    @brief          : 飞腾spi通信协议处理
    @author         : xiongjinqi
    @date           : 2024/07/25
***********************************************************************************************************************
*/

/* add by lkx 25.7.10 */


#ifndef __SPI_PROTOCOL_H_
#define __SPI_PROTOCOL_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "ringbuffer.h"
#include "types_def.h"


#define SPI_HEADER          (0xAA)
#define SPI_HEADER_SIZE     (5)

#define SPI_DATA_SIZE       (512)

#define SPI_CMD11           (0x11)
#define SPI_CMD12           (0x12)
//#define SPI_CMD13           (0x13)
//#define SPI_CMD22           (0x22)
//#define SPI_CMD23           (0x23)
//#define SPI_CMD25           (0x25)


#define SPI_RECEIVE_WAIT	10000 			/*单位us*/
#define SPI_RECEIVE_CYCLE	(500000/SPI_RECEIVE_WAIT)		/*500ms超时计数*/

typedef enum {
    SPI_STATUS_IDLE = 1,
    SPI_STATUS_WAIT,
    SPI_STATUS_RECEIVE,
    SPI_STATUS_SEND,
    SPI_STATUS_ERROR
}_SPI_STATUS_T;

typedef int(*do_data_func) (void *protocol, u8 *buff, u16 len);
typedef int(*do_cmd_func) (void *protocol, u8 *buff, u16 len);

typedef struct {
    u16 cmd;
    do_data_func do_data;
}_do_data_t;

typedef struct {
    int fd;            // spi设备描述符
    ring_buffer_t rb_handler;
    u8 rxbuff[SPI_DATA_SIZE*10];
    u8 txbuff[SPI_DATA_SIZE];
    u16 rxindex;     // 接收中计数
    u16 rxcount;     // 接收总字节数
    u16 txindex;     // 发送计数
    u16 txcount;     // 发送总字节数
}_spi_buffer_t;

/* 响应数据类型定义 */
typedef struct {
    u8 header;
    u16 length;
    u8 cmd;
    u8 data[SPI_DATA_SIZE];
    u8 sum;
}_spi_protocol_t;


typedef struct {
    u8 status;
    _spi_buffer_t *spi;
    _spi_protocol_t resquest;   // 响应数据帧
    do_cmd_func do_cmd;
}_spi_t;

/* 供外部使用 */
extern _spi_t           spi_protocol;
extern _ipmi_data_t     ipmi_data;
extern _ipmi_abnormal_t ipmi_abnormal;
extern _zjb_power_ctrl_t zjb_power_ctrl_get[16]; // 获取电源控制启动上电句柄

/* 
    @brief      : cmd11指令发送函数,查询ipmi数据
        @note   : 定时执行，查询ipmi数据
    @param[in]  : 
            handler         spi数据处理句柄
    @param[out] : none
    @retval     : 0成功，1失败
*/
int do_reqcmd11(_spi_t *handler);

/* 
    @brief      : cmd12指令发送函数,查询异常情况
        @note   : 定时执行，查询是否有异常
    @param[in]  : 
            handler         spi数据处理句柄
    @param[out] : none
    @retval     : 0成功，1失败
*/
int do_reqcmd12(_spi_t *handler);


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
int do_reqcmd13(_spi_t *handler);

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
int do_reqcmd22(_spi_t *handler, u8 sensorid, u8 status);

/* 
    @brief      : cmd23指令发送函数
    @param[in]  : 
            handler     spi数据处理句柄
            busid       ipmi总线ID:
                            IPMI_BUSA
                            IPMI_BUSB
    @param[out] : none
    @retval     : 0成功，1失败
*/
int do_reqcmd23(_spi_t *handler, u8 busid);

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
        len         电源开关控制buff长度，固定值16
    @param[out] : none
    @retval     : 0成功，1失败
*/
int do_reqcmd25(_spi_t *handler, u8 *onoff_buff, u16 len);

#endif

/*
    @brief      : SPI协议栈初始化
*/
int spi_protocol_init(_spi_t *handler);

/*
    @brief      : SPI协议栈关闭
*/
int spi_protocol_close(_spi_t *handler);

#ifdef __cplusplus
}
#endif


#endif /* __SPI_PROTOCOL_H_ */

