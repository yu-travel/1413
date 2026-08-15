/*
***********************************************************************************************************************
    @brief          : spi访问ipmi数据类型定义
    @author         : xiongjinqi
    @date           : 2024/08/08
***********************************************************************************************************************
*/

/* add by lkx 25.7.10 */


#ifndef __DATATYPE_H__
#define __DATATYPE_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "types_def.h"

/* 主控sensor id定义 */
typedef enum {
    ID_ZK_TEMPRETURE = 0x50,
    ID_ZK_VDDQ,
    ID_ZK_VDD_CORE,
    ID_ZK_VCC1V8,
    ID_ZK_VCC2V5,
    ID_ZK_VCC3V3,
    ID_ZK_VCC5V0,
    ID_ZK_VPX12V_CURR = 0x57,
}_ZK_SENSOR_ID_T;

/* 电源板sensor id定义 */
typedef enum {
    ID_PWR_TEMPRETURE = 0x60,
    ID_PWR_VCC12V_1,
    ID_PWR_VCC12V_2,
    ID_PWR_VCC3V3	=0x63,
}_PWR_SENSOR_ID_T;

/* 接口扩展板sensor id定义 */
typedef enum {
    ID_JKKZ_TEMPRETURE = 0x30,
    ID_JKKZ_VCC1V2,
    ID_JKKZ_VCC1V5,
    ID_JKKZ_VCC1V8,
    ID_JKKZ_VCC1V0,
    ID_JKKZ_VCC2V5,
    ID_JKKZ_VCC3V3,
    ID_JKKZ_VPX12V_CURR	=0x37,
}_JKKZ_SENSOR_ID_T;

/* 信号处理板sensor id定义 */
typedef enum {
    ID_XHCL_TEMPRETURE = 0x21,
    ID_XHCL_VCC1V2,
    ID_XHCL_VCC1V5,
    ID_XHCL_VCC1V8,
    ID_XHCL_VCC1V0,
    ID_XHCL_VCC3V3,
    ID_XHCL_VPX12V_CURR	,
    ID_XHCL_SLOT_ADDR	=0x28,
}_XHCL_SENSOR_ID_T;

/* 转接板sensor id定义 */
typedef enum {
    ID_ZJB_FAN1 = 0x09,
    ID_ZJB_FAN2,
    ID_ZJB_FAN3,
    ID_ZJB_FAN4,
}_ZJB_SENSOR_ID_T;

/* 哈特曼板sensor id定义 */
typedef enum {
    ID_HTM_TEMPRETURE = 0x91,
    ID_HTM_VCC1V2,
    ID_HTM_VCC1V5,
    ID_HTM_VCC1V8,
    ID_HTM_VCC1V0,
    ID_HTM_VCC3V3,
    ID_HTM_VPX12V_CURR	,
    ID_HTM_SLOT_ADDR	=0x98,
}_HTM_SENSOR_ID_T;

/* 新增电源板sensor id定义 */
typedef enum {
    ID_PWR2_TEMPRETURE = 0xA0,
    ID_PWR2_VCC28V_1,
    ID_PWR2_VCC28V_2,
}_PWR2_SENSOR_ID_T;


/**************************************************************************************************************************/
typedef enum {
    POWER_OFF=0,
    POWER_ON,
}_POWER_CTRL_T;

typedef enum {
    FAN_LEVEL_1 = 1,
    FAN_LEVEL_2,
    FAN_LEVEL_3,
    FAN_LEVEL_4,
}_FAN_LEVEL_T;

typedef enum {
    IPMI_BUSA = 1,
    IPMI_BUSB,
}_IPMI_BUS_T;

/**************************************************************************************************************************/
/* 异常情况定义 */
/* 异常ID add by xjq */
typedef enum {
    //AB_ID_ZJB_EEPROM = 0x70,
    AB_ID_ZKB_EEPROM = 0x71,
    AB_ID_JKKZ_EEPROM,
    AB_ID_XHCL_EEPROM,
    AB_ID_HTM_EEPROM,
    AB_ID_PWR_EEPROM,
    AB_ID_ZJB_SUB_COMM,
    AB_ID_JKKZ_SUB_COMM,
    AB_ID_XHCL_SUB_COMM,
    AB_ID_HTM_SUB_COMM,
    AB_ID_PWR_SUB_COMM,
    AB_ID_MAX_NULL,
    AB_ID_PWR2_EEPROM,
    AB_ID_PWR2_SUB_COMM,
}_ABNORMAL_ID_T;

/* 异常状态 add by xjq */
typedef enum {
                                    /* 暂无过流异常，过流硬件会保护 */
    AB_STA_DEV_OK         = 0x0F,
    AB_STA_OVERVOLTAGE    = 0x10,   /* 过压 */
    AB_STA_UNDERVOLTAGE   = 0x11,   /* 欠压 */
    AB_STA_OPT_FAIL       = 0x12,   /* 操作失败, 指风扇、电源控制操作 */
    AB_STA_EEPROM         = 0x13,   /* EEPROM设备异常 */
    AB_STA_TEMPRETURE     = 0x14,   /* 温度传感器设备异常 */
    AB_STA_DEV_OVER_TEMP  = 0x15,   /* 设备过温(由主控板监控)，风扇最大，温度超过主控控制范围, 严重等级最高 */
    AB_STA_SUB_COMM       = 0x16,   /* 子卡通信异常(IPMI通信) */
    AB_STA_OVERCURRENT    = 0x20,   /* 过流 1<<5		*/
    AB_STA_OVERCURANDOVERVOL  = 0x30,		/* 过压过流，0x10|0x20*/	
    AB_STA_OVERCURANDUNVOL	  = 0x31,		/* 欠压过流 0x11|0x20*/
}_ABNORMAL_STATUS_T;

/* 异常情况数据定义 */
typedef struct {
    u8 AB_PWR_SUB_COMM;         // 电源板通信异常
    u8 AB_PWR_TEMPRETURE;       // 电源板温度设备异常
    u8 AB_PWR_EEPROM;           // 电源板EEPROM异常
    u8 AB_PWR_VCC12V_1;         // 电源板电压异常
    u8 AB_PWR_VCC12V_2;         // 同上
    u8 AB_PWR_VCC3V3;           // 同上

    u8 AB_JKKZ_SUB_COMM;        // 接口扩展板通信异常
    u8 AB_JKKZ_TEMPRETURE;      // 接口扩展板温度设备异常
    u8 AB_JKKZ_EEPROM;          // 接口扩展板EEPROM异常
    u8 AB_JKKZ_VCC1V2;          // 接口扩展板电压异常
    u8 AB_JKKZ_VCC1V5;          // 同上
    u8 AB_JKKZ_VCC1V8;          // 同上
    u8 AB_JKKZ_VCC1V0;          // 同上
    u8 AB_JKKZ_VCC2V5;          // 同上
    u8 AB_JKKZ_VCC3V3;          // 同上

    u8 AB_XHCL_SUB_COMM;        // 信号处理板通信异常
    u8 AB_XHCL_TEMPRETURE;      // 信号处理板温度设备异常
    u8 AB_XHCL_EEPROM;          // 信号处理板EEPROM异常
    u8 AB_XHCL_VCC1V2;          // 信号处理板电压异常
    u8 AB_XHCL_VCC1V5;          // 同上
    u8 AB_XHCL_VCC1V8;          // 同上
    u8 AB_XHCL_VCC1V0;          // 同上
    u8 AB_XHCL_VCC3V3;          // 同上

    u8 AB_HTM_SUB_COMM;         // 哈特曼板通信异常
    u8 AB_HTM_TEMPRETURE;       // 哈特曼板温度设备异常
    u8 AB_HTM_EEPROM;           // 哈特曼板EEPROM异常
    u8 AB_HTM_VCC1V2;           // 哈特曼板电压异常
    u8 AB_HTM_VCC1V5;           // 同上
    u8 AB_HTM_VCC1V8;           // 同上
    u8 AB_HTM_VCC1V0;           // 同上
    u8 AB_HTM_VCC3V3;           // 同上

    u8 AB_ZJB_SUB_COMM      ;   // 转接板通信异常 实际为背板

    u8 AB_ZK_VDDQ;              // 主控板电压异常
    u8 AB_ZK_VDD_CORE;          // 同上
    u8 AB_ZK_VCC1V8;            // 同上
    u8 AB_ZK_VCC2V5;            // 同上
    u8 AB_ZK_VCC3V3;            // 同上
    u8 AB_ZK_VCC5V0;            // 同上

    u8 AB_BOX_OVER_TEMPRETURE; // 机箱过温，严重等级最高

	u8 AB_PWR2_SUB_COMM;         // 新增电源板通信异常
    u8 AB_PWR2_TEMPRETURE;       // 电源板温度设备异常
    u8 AB_PWR2_EEPROM;           // 电源板EEPROM异常
    u8 AB_PWR2_VCC28V_1;         // 电源板电压异常
    u8 AB_PWR2_VCC28V_2;         // 同上
}_ipmi_abnormal_t;


/**************************************************************************************************************************/
/* 数据结构体定义 */
/**************************************************************************************************************************/
/* 转接板启动后电源上电结构体 */
typedef struct {
    u8 zjb_power_id;
    u8 onoff_ctrl;
}_zjb_power_ctrl_t;

typedef struct {
    u8 slot_addr;
    s16 tempreture;
    u16 vcc1v2; // vcc1v2电压扩大100倍
    u16 vcc1v5; // vcc1v5电压扩大100倍
    u16 vcc1v8; // vcc1v5电压扩大100倍
    u16 vcc1v0; // vcc1v5电压扩大100倍
    u16 vcc3v3; // vcc1v5电压扩大100倍
    u16 vpx12v_curr;    // VPX12V供电电流扩大100倍
}_htm_monitor_t; //哈特曼板卡


typedef struct {
    s16 tempreture;
    u16 vddq;       // 电压扩大100倍
    u16 vdd_core;   // 电压扩大100倍
    u16 vcc1v8;     // 电压扩大100倍
    u16 vcc2v5;     // 电压扩大100倍
    u16 vcc3v3;     // 电压扩大100倍
    u16 vcc5v0;     // 电压扩大100倍
    u16 vpx12v_curr;    // VPX12V供电电流扩大100倍
}_zkb_monitor_t;

typedef struct {
    s16 tempreture;
    u16 VOL_VCC12V_1;   // 电压扩大100倍
    u16 VOL_VCC12V_2;   // 电压扩大100倍
    u16 VOL_VCC3V3;     // 电压扩大100倍
    u16 CURR_VCC12V_1;  // 电流扩大100倍
    u16 CURR_VCC12V_2;  // 电流扩大100倍
    u16 CURR_VCC3V3;    // 电流扩大100倍
    u8  ONOFF_VCC12V_1;
    u8  ONOFF_VCC12V_2;
    u8  ONOFF_VCC3V3;
}_pwr_monitor_t;

typedef struct {
    s16 tempreture;
    u16 VOL_VCC28V_1;   // 电压扩大100倍
    u16 VOL_VCC28V_2;   // 电压扩大100倍
    u16 CURR_VCC28V_1;  // 电流扩大100倍
    u16 CURR_VCC28V_2;  // 电流扩大100倍

    u8  ONOFF_VCC28V_1;
    u8  ONOFF_VCC28V_2;
}_pwr2_monitor_t;


typedef struct {
    s16 tempreture;
    u16 vcc1v2; // 电压扩大100倍
    u16 vcc1v5; // 电压扩大100倍
    u16 vcc1v8; // 电压扩大100倍
    u16 vcc1v0; // 电压扩大100倍
    // u16 vcc2v5;  实际没有使用故去掉
    u16 vcc3v3;  // 电压扩大100倍
    u16 vpx12v_curr;    // VPX12V供电电流扩大100倍
}_jkkz_monitor_t;

typedef struct {
    u8 slot_addr;
    s16 tempreture;
    u16 vcc1v2; // vcc1v2电压扩大100倍
    u16 vcc1v5; // vcc1v5电压扩大100倍
    u16 vcc1v8; // vcc1v5电压扩大100倍
    u16 vcc1v0; // vcc1v5电压扩大100倍
    u16 vcc3v3; // vcc1v5电压扩大100倍
    u16 vpx12v_curr;    // VPX12V供电电流扩大100倍
}_xhcl_monitor_t;

/* 转接板ipmi数据定义 */
typedef struct {
    u16 FAN1_speed;     // 风扇
    u16 FAN2_speed;
    u16 FAN3_speed;
    u16 FAN4_speed;

    /* 电源的开关状态：0-关闭,1-打开 */
    u8 ONOFF_FAN1;
    u8 ONOFF_FAN2;
    u8 ONOFF_FAN3;
    u8 ONOFF_FAN4;
}_zjb_monitor_t;

typedef struct {
    _pwr_monitor_t      power;  // 电源板数据
    _jkkz_monitor_t     jkkz;   // 接口扩展板数据
    _xhcl_monitor_t     xhcl;   // 信号处理板数据
    _zjb_monitor_t      zjb;    // 转接板数据
    _zkb_monitor_t      zkb;    // 主控板数据
    _htm_monitor_t      htm;    // 哈特曼数据
    _pwr2_monitor_t     power2;  // 新增电源板数据
}_ipmi_data_t;

#ifdef __cplusplus
}
#endif

#endif /* __DATATYPE_H__ */
