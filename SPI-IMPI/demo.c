/*
**************************************************************************************************************
    @brief          : libspi库测试demo
    @author         : xiongjinqi
    @date           : 2024/08/08
**************************************************************************************************************
*/

/* add by lkx 25.7.10 */


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
#include <sys/time.h>

#include "datatype.h"
#include "spi_protocol.h"

#define DEBUG                           printf
#define _STRING(x)                      #x
#define STR(s)                          _STRING(s)
#define _SENSORID_GET(name, sensor)     (ID_##name##_##sensor)
#define SENSORID_GET(name, sensor)      _SENSORID_GET(name, sensor)

typedef enum {
    INDEX_ZJB_28V_JCXJ    = 0,
    INDEX_ZJB_12V_WAOXJ   ,
    INDEX_ZJB_28V_BQXJ    ,
    INDEX_ZJB_12V_DYGY_B  ,
    INDEX_ZJB_28V_FFXJ    ,
    INDEX_ZJB_12V28V_BF1  ,
    INDEX_ZJB_12V28V_BF2  ,
    INDEX_ZJB_28V_TSGY    ,
    INDEX_ZJB_28V_QGSJ    ,
    INDEX_ZJB_12V28V_HJJC1,
    INDEX_ZJB_12V28V_HJJC2,
    INDEX_ZJB_12V28V_HJJC3,
    INDEX_ZJB_12V_GSDJ1   ,
    INDEX_ZJB_12V_GSDJ2   ,
    INDEX_ZJB_12V_XGHTM   ,
    INDEX_ZJB_28V_KF      ,
}_ZJB_POWER_ONOFF_INDEX;

char *ZJB_POWER_NAME[] = {
    "ZJB_28V_JCXJ    ",
    "ZJB_12V_WAOXJ   ",
    "ZJB_28V_BQXJ    ",
    "ZJB_12V_DYGY_B  ",
    "ZJB_28V_FFXJ    ",
    "ZJB_12V28V_BF1  ",
    "ZJB_12V28V_BF2  ",
    "ZJB_28V_TSGY    ",
    "ZJB_28V_QGSJ    ",
    "ZJB_12V28V_HJJC1",
    "ZJB_12V28V_HJJC2",
    "ZJB_12V28V_HJJC3",
    "ZJB_12V_GSDJ1   ",
    "ZJB_12V_GSDJ2   ",
    "ZJB_12V_XGHTM   ",
    "ZJB_28V_KF      ",
};

u8 zjb_sensor_id[] = {
    ID_ZJB_FAN1,
    ID_ZJB_FAN2,
    ID_ZJB_FAN3,
    ID_ZJB_FAN4,
};



void usage_print(char *filename)
{
    DEBUG("Usage: %s [options] [sensorid] \r\n", filename);
    DEBUG("\tOptions:\r\n");
    DEBUG(" \t -c \t CMD name, contain of 11 / 12 \r\n");
    DEBUG(" \t\t cmd11 / cmd12 no parameters\r\n");
}

u64 time_getms()
{
    struct timeval now;
    gettimeofday(&now, NULL);

    return (now.tv_sec*1000 + now.tv_usec/1000);
}

/*
    usage:
        ./demo cmdxx board_name sensorid 

    example:
        ./demo -c 11
        ./demo -c 12
*/
int main(int argc, char* argv[])
{
    int ret = 0,index = 0;
    int cmd = 0, bus = 0, setval = 0, sensor_id = 0, power_onoff = 0;
    char *board_name;
    u64 start_time = 0, end_time = 0;

    switch(argc)
    {
        case 3:
        break;
        default:
            DEBUG("argc : %d \r\n", argc);
            usage_print(argv[0]);  // 
            return -1;
        break;
    }
    /* 1、第一步初始化 */
    DEBUG("open spi device \r\n");
    spi_protocol_init(&spi_protocol);

    if(argc == 3)   // cmd11
    {
        if(strcmp(argv[1], "-c") == 0) 
        {
            cmd = atoi(argv[2]);
        }
    }

    /* 2、第二步执行相关指令 */
/*-------------------------------------------------------------------------------------*/
    if(cmd == 11)
    {
        start_time = time_getms();
        ret = do_reqcmd11(&spi_protocol);
        end_time = time_getms();
        DEBUG("Line%d : cmd11 execute time : %llu ms\n", __LINE__, end_time - start_time);
        if(ret == 0)
            DEBUG("Line%d : cmd11 success\n", __LINE__);
        else
            DEBUG("Line%d : cmd11 failed\n", __LINE__);
    }
/*-------------------------------------------------------------------------------------*/
    if(cmd == 12)
    {
        start_time = time_getms();
        ret = do_reqcmd12(&spi_protocol);
        end_time = time_getms();
        DEBUG("Line%d : cmd12 execute time : %llu ms\n", __LINE__, end_time - start_time);

        if(ret == 0)
            DEBUG("Line%d : cmd12 success\n", __LINE__);
        else
            DEBUG("Line%d : cmd12 failed\n", __LINE__);
    }
/*-------------------------------------------------------------------------------------*/
    #if 0
    static int count = 0;
    while (1)
    {
        //do_reqcmd11(&spi_protocol);
        DEBUG("Running %d\r\n", count++);
        sleep(1);
        // if(count > 10)
        //     break;
    }
    #endif

    /* 第3步释放相关资源 */
    spi_protocol_close(&spi_protocol);
    
    exit(0);
    return 0;
}




