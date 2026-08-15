/*
***********************************************************************************************
    @brief      : 通用类型定义
***********************************************************************************************
*/
#ifndef __TYPES_DEF_H_
#define __TYPES_DEF_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <string.h>

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


#define TEST_ENABLE         1
#define DEBUG_OUT           1               /* 使能（1）/ 禁止（0）DEBUG输出 */

#define myprintf(sFormat, ...)              SEGGER_RTT_printf(0, sFormat, ##__VA_ARGS__)
/**
* 错误信息打印
* 自动打印发生错误时代码所在的位置
*/
#if DEBUG_OUT
#define pr_err(fmt,args...) printf("[Error]:File:<%s> Fun:[%s] Line:%d\n "fmt, __FILE__, __FUNCTION__, __LINE__, ##args)
#define pr_info(fmt,args...) printf("[Info]: "fmt, ##args)
#define pr_debug(fmt,args...) printf("[Debug]:File:<%s> Fun:[%s] Line:%d\n "fmt, __FILE__, __FUNCTION__, __LINE__, ##args)
#else
#define pr_err(fmt,args...)     /*do nothing */
#define pr_info(fmt,args...)    
#define pr_debug(fmt,args...)   
#endif

#define PRINTF(...)         \
    do {                        \
        printf(__VA_ARGS__);    \
    } while(0)

#define TRACE_OUT(flag, ...)       \
    do {                           \
        if(flag) {                 \
            printf(__VA_ARGS__);   \
        }                          \
    } while(0)

#ifdef __cplusplus
}
#endif

#endif /* __TYPES_DEF_H_ */


