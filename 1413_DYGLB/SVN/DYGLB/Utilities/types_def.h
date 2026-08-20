/*
***********************************************************************************************
    @brief      : 通用类型定义
***********************************************************************************************
*/
#ifndef __TYPES_DEF_H_
#define __TYPES_DEF_H_

#include <stdio.h>
#include <string.h>
#include "SEGGER_RTT.h"

//typedef char            s8;
typedef signed short    s16;
typedef signed int      s32;
typedef long long       s64;

    /* exact-width unsigned integer types */
typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;
typedef unsigned long long u64;


#ifndef TRUE
#define TRUE        1U
#endif

#ifndef FALSE
#define FALSE       0U
#endif

#ifndef NULL
#define NULL        0U
#endif


#define CODE_PART(x)                        1   // 代码阅读宏

#define ARRAY_SIZE(Array)                   (sizeof(Array) / sizeof((Array)[0]))

#define MIN(i, j)                           (((i) < (j)) ? (i) : (j))
#define MAX(i, j)                           (((i) > (j)) ? (i) : (j))


#define BIT(x)                              (1<<x)
#define SET_BITS(data, bits, shift)         (data|(bits)<<shift)
#define MY_CLEAR_BIT(x , bit)               (x &= ~(1<<bit))
#define MY_SET_BIT(x, bit)                  (x |= (1 << bit))
#define MY_GET_BIT(x, bit)                  (((x) >> (bit))&0x01 )

#define H16_GET(val)                        ((val>>16)&0xFFFF)
#define L16_GET(val)                        (val&0xFFFF)

#define H8_GET(val)                         ((val>>8)&0xFF)
#define L8_GET(val)                         (val&0xFF)

#define U8_TO_U16(u8_h, u8_l)               ((u8_h<<8)|u8_l)

#define CMD22RUNING		1	/*cmd22执行中*/
#define CMD22RUNEND		0	/*cmd22未在执行*/


#define TEST_ENABLE         1
#define DEBUG_OUT           1               /* 使能（1）/ 禁止（0）DEBUG输出 */
#define LYSDEBUG			0				/*输出均值相关的数据*/
#define LYSDEBUG1			0				/*打印AD7606采集到的模拟数据*/
#define LYSDEBUG2			1				/*从fpga接收数据的打印开关*/
#define LYSDEBUG3			1				/*5542相关数据*/
#define LYSDEBUG4			1				/*程序应该永远执行不到的打印，假如有这行打印，程序执行有问题*/
#define LYSDEBUG5			0				/*4001相关数据*/



#define myprintf(sFormat, ...)              SEGGER_RTT_printf(0, sFormat, ##__VA_ARGS__)
#define myprintf_1(sFormat, ...)              SEGGER_RTT_printf(1, sFormat, ##__VA_ARGS__)
#define myprintf_2(sFormat, ...)              SEGGER_RTT_printf(2, sFormat, ##__VA_ARGS__)



#define PRINTF(...)         \
    do {                        \
        myprintf(__VA_ARGS__);    \
    } while(0)

#if 1
#define TRACE_OUT(flag, ...)       \
    do {                           \
        if(flag) {                 \
            myprintf(##__VA_ARGS__);   \
        }                          \
    } while(0)

#define TRACE_OUT_2(flag, ...)       \
    do {                           \
        if(flag) {                 \
            myprintf_2(##__VA_ARGS__);   \
        }                          \
    } while(0)
		
#else
#define TRACE_OUT(flag, ...)        
#endif

#define LYSprintf(flag, ...)       \
    do {                           \
        if(flag) {                 \
            myprintf(##__VA_ARGS__);   \
        }                          \
    } while(0)

#define ttprintf(flag, ...)       \
			do {						   \
				if(flag) {				   \
					printf(##__VA_ARGS__);   \
				}						   \
			} while(0)


#endif /* __TYPES_DEF_H_ */


