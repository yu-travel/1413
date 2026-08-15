/*
**************************************************************************************************************
    @brief          : 飞腾spi设备接口
    @author         : xiongjinqi
    @date           : 2024/08/08
**************************************************************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <assert.h>
#include <signal.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "types_def.h"
#include "spi_dev.h"

/*
******************************************************************************************************************
	变量定义
******************************************************************************************************************
*/
static u8 mode = SPI_CPHA | SPI_CPOL;
static u8 bits = 8;
static uint32_t speed = 1000000;		// 默认速率1MHZ
static uint16_t delay = 0;

void pabort(const char *s)
{
	perror(s);
	//abort();
}

/**
* 功 能：发送数据
* 入口参数 ：
*           TxBuf -> 发送数据首地址
*           len ->  发送与长度
*返回值：0 成功
* 开发人员：xiongjinqi 24/08/08
*/
int spi_write_bytes(int fd, u8 *txbuff, int len)
{
    int ret;
 
    ret = write(fd, txbuff, len);
    if (ret < 0)
        pr_err("SPI Write error\n");
    else
    {
		#if DEBUG_OUT
		PRINT_ARRAY(txbuff, len, SPI_WRITE_ARRY_OPT);
		#endif
    }
    return ret;
}

/**
* 功 能：发送数据
* 入口参数 ：
*           TxBuf -> 接收数据首地址
*           len ->  接收的长度
*返回值：0 成功
* 开发人员: xiongjinqi
*/
int spi_read_bytes(int fd, u8 *rxbuff, int len)
{
    int ret;
 
    ret = read(fd, rxbuff, len);
    if (ret < 0)
        pr_err("SPI Read error\n");
    else
    {
		#if DEBUG_OUT
		//PRINT_ARRAY(rxbuff, len, SPI_READ_ARRY_OPT);
		#endif
    }
    return ret;
}

/*
	@brief : TESAM设备打开接口
	@param[in]  : 
			name -> 字符设备名称
	@param[out]	: none
	@retval		:  fd文件描述符
*/
int spi_dev_open(char *name)
{
	int ret, fd;
	/* spi 设备初始化 */
	fd = open(name, O_RDWR);
	if (fd < 0) {
        pr_err("Unable to open spi device\n");
        return -1;
    }

	/* spi mode */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1){
		pabort("can't set spi mode");
		return ret;
	}
	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1){
		pabort("can't get spi mode");
		return ret;
	}

	/* bits per word */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1){
		pabort("can't set bits per word");
		return ret;
	}
	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1){
		pabort("can't get bits per word");
		return ret;
	}

	/* max speed hz */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1){
		pabort("can't set max speed hz");
		return ret;
	}
	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1){
		pabort("can't get max speed hz");
		return ret;
	}

	pr_info("spi device: %s \r\n", name);
    pr_info("spi mode: %d \r\n", mode);
    pr_info("spi per word: %d \r\n", bits);
    pr_info("spi max speed: %d Hz (%d KHz) \r\n", speed, speed/1000);

	return (fd);
}

/*
	@brief : spi设备关闭
	@param[in]  : 
			fd -> 文件描述符
	@param[out]	: none
	@retval		: 
*/
void spi_dev_close(int fd)
{
    close(fd);
}



