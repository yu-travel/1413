#ifndef TRANSF_JKKZB_H
#define TRANSF_JKKZB_H

// 定义ADC通道数量
#define ADC_CHANNEL_NUM     8
//开关状态和告警信息
typedef union {
    uint16_t raw;
    struct {
        uint16_t sfxj_stu:1;
        uint16_t sfpsd_stu:1;
		uint16_t bqxj_stu:1;
		uint16_t qgsj_stu:1;		
        uint16_t reseive_1:4;
        uint16_t sfxj_warning:1;
        uint16_t sfpsd_warning:1;
		uint16_t bqxj_warning:1;
		uint16_t qgsj_warning:1;		
        uint16_t reseive_2:4;
    } bits;
} STUSIG_PACKED;

typedef struct 
{
	uint16_t 	dataHead;		//帧头
	uint16_t	data_len;		//帧长度
	uint16_t 	messageData[DEVNUM*3];	//数据
	STUSIG_PACKED 		stuAndwarning;	//开关状态和告警信息
	uint16_t 	sum;			//校验和
	uint16_t 	dataEnd;		//帧结束
} SEND_MESSAGE;

typedef struct 
{
	uint16_t 	dataHead;		//帧头
	uint16_t	data_len;		//帧长度
	uint16_t 	messageData[DEVNUM*3];	//数据
	STUSIG_PACKED 		stuAndwarning;	//开关状态和告警信息
	uint16_t 	sum;			//校验和
	uint16_t 	dataEnd;		//帧结束
} RECIVE_MESSAGE;

void Spi1_Gpio_Init(void) ;
void Spi1_Configuration(void);
uint16_t SPI1_ReadWrite(uint16_t data);

void data_Packet_Creat(void);
void messageData_Send(void);
int messageData_Recive(void);

#endif

