/*
**************************************************************************************************************
    @brief          : 飞腾spi设备接口
    @author         : xiongjinqi
    @date           : 2024/08/08
**************************************************************************************************************
*/
#ifndef __SPI_DEV_H_
#define __SPI_DEV_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define SPI_DEVICE    "/dev/spidev0.0"

#define SPI_READ_ARRY_OPT			0
#define SPI_WRITE_ARRY_OPT			1

#define PRINT_ARRAY(arr, len, opt)		\
	do{									\
		int i = 0;						\
		printf("\nSPI %s [Len:%d]: ", opt?"write":"read", len);	\
		for (i = 0; i < len; i++) {	    \
			if (i%32 == 0 && i != 0)    \
				printf("\n\t");		    \
			printf("%02X ", arr[i]);	\
		}							    \
		printf("\r\n");				    \
	} while(0)

void pabort(const char *s);	// 异常情况，挂起

/**
* 功 能：发送数据
* 入口参数 ：
*           TxBuf -> 发送数据首地址
*           len ->  发送与长度
*返回值：0 成功
* 开发人员：xiongjinqi 24/08/08
*/
int spi_write_bytes(int fd, u8 *txbuff, int len);

/**
* 功 能：发送数据
* 入口参数 ：
*           TxBuf -> 接收数据首地址
*           len ->  接收的长度
*返回值：0 成功
* 开发人员: xiongjinqi
*/
int spi_read_bytes(int fd, u8 *rxbuff, int len);

/*
	@brief : TESAM设备打开接口
	@param[in]  : 
			name -> 字符设备名称
	@param[out]	: none
	@retval		:  fd文件描述符
*/
int spi_dev_open(char *name);

/*
	@brief : spi设备关闭
	@param[in]  : 
			fd -> 文件描述符
	@param[out]	: none
	@retval		: 
*/
void spi_dev_close(int fd);

#ifdef __cplusplus
}
#endif
#endif /* __SPI_DEV_H_ */
