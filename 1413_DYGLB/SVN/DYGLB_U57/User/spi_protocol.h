/*
***********************************************************************************************************************
    @brief          : 飞腾spi通信协议处理
    @author         : xiongjinqi
    @date           : 2024/07/25
***********************************************************************************************************************
*/
#ifndef __SPI_PROTOCOL_H_
#define __SPI_PROTOCOL_H_

#include "spi.h"
#include "main.h"
#include "ringbuffer_u8.h"
#include "ipmi_protocol.h"


#define SPI_TRANS_USE_INT   1       // 1使用中断发送；0使用DMA(暂未调通)

#define SPI_HEADER          (0xAA)
#define SPI_HEADER_SIZE     (5)

#define SPI_DATA_SIZE       (512)

#define IPMI_BUSA           (0x01)
#define IPMI_BUSB           (0x02)

#define SPI_CMD11           (0x11)
#define SPI_CMD12           (0x12)
#define SPI_CMD13           (0x13)

#define SPI_CMD22           (0x22)
#define SPI_CMD23           (0x23)
#define SPI_CMD25           (0x25)


typedef struct {
    SPI_TypeDef *dev;
    u8_ring_buffer_t rb_handler;
    u8 rxbuff[SPI_DATA_SIZE];
    u8 txbuff[SPI_DATA_SIZE*2];
    u16 rxindex;     // 接收中计数
    u16 rxcount;     // 接收总字节数
    u16 txindex;     // 发送计数
    u16 txcount;     // 发送总字节数
}_spi_interrupt_t;


/* 响应数据类型定义 */
typedef struct {
    u8 header;
    u16 length;
    u8 cmd;
    u8 data[SPI_DATA_SIZE];
    u8 sum;
}_spi_protocol_t    ;


/* 暂存数据 */
typedef struct {
    u8 *buffer;
    u16 index;
    u16 count;
}_resp_buff_t;

typedef enum {
    PROCESS_CMD11 = AB_MAX_NULL + 1,
    PROCESS_CMD12,
    PROCESS_CMD13,
    PROCESS_CMD22,
    PROCESS_CMD25,
}_SPI_PROCESS_STATUS_T;

typedef struct {
    u8               status;    // 为cmd22设计
    _spi_interrupt_t *spi;
    _spi_protocol_t  response;   // 响应数据帧
    do_cmd_func      do_cmd;
}_spi_t;

extern _spi_t spi_protocol;
extern _resp_buff_t respcmd12;
extern _resp_buff_t respcmd11;


void respcmd12_data_set(_resp_buff_t *buffer, u8 *data, u16 datalen);
void respcmd11_data_set(_resp_buff_t *buffer);

/* 
    @brief      : cmd12指令发送函数，在ipmi协议处理中调用
    @param[in]  : 
            handler         spi数据处理句柄
            buff            数据首地址
            len             数据长度
    @param[out] : none
    @retval     : 检验和
*/
int do_respcmd12_transmit (_spi_t *handler, u8 *buff, u16 len);

/* 
    @brief      : cmd13指令发送函数，在ipmi协议处理中调用
    @param[in]  : 
            handler         spi数据处理句柄
            buff            数据首地址
            len             数据长度
    @param[out] : none
    @retval     : 检验和
*/
int do_respcmd13_transmit (_spi_t *handler, u8 *buff, u16 len);


/* 
    @brief      : cmd22指令发送函数，在ipmi协议处理中调用
    @param[in]  : 
            handler         spi数据处理句柄
            buff            数据首地址
            len             数据长度
    @param[out] : none
    @retval     : 检验和
*/
int do_respcmd22_transmit (_spi_t *handler, u8 *buff, u16 len);

/* 
    @brief      : cmd25指令发送函数，在ipmi协议处理中调用
    @param[in]  : 
            handler         spi数据处理句柄
            buff            数据首地址
            len             数据长度
    @param[out] : none
    @retval     : 检验和
*/
int do_respcmd25_transmit (_spi_t *handler, u8 *buff, u16 len);

/*
    @brief      : SPI协议栈初始化
*/
void spi_protocol_init(void);

/*
    @brief      : SPI协议栈初始化
*/
void spi_protocol_close(void);

/*
    @brief      : spi协议栈处理
*/
int spi_protocol_loop(void);



#endif /* __SPI_PROTOCOL_H_ */

