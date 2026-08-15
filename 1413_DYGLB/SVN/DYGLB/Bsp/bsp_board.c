#include "stm32f4xx.h"
#include "bsp_board.h"
#include "types_def.h"

/*============================================================
    板级引脚映射实例表 (声明见 board_map.h)
    数据来源: doc\原理图管脚连接关系.txt + board_map.h 设备映射表
============================================================*/

/*
    @brief      : SWJ_CFG 位域值 (SYSCFG_MEMRMP[26:24] = 0b010)
                  关闭 JTAG-DP, 保留 SW-DP, 释放 PA15 作普通 GPIO
    @note       : 本工程 stm32f4xx.h 的 SYSCFG_TypeDef 无 MAPR 成员
                  (F405/407 无 MAPR 寄存器, SWJ_CFG 位于 MEMRMP),
                  旧 mac5048.c 的 SYSCFG->MAPR 写法已不适用
*/
#define SWJ_CFG_JTAG_DISABLE    ((u32)0x02000000)

/*
    @brief      : 15 路电源设备映射表
    @note       : 索引 = 设备 ID (dev_id_e), [0] 为 DEV_INVALID 占位槽
                  ID  名称        EN   故障                  芯片      DAC(idx/ch) V/I/T AIN  FAULT_AIN
                  0x01 28V_KF    PD0   KF1=PB4 + KF2=PA15    MAC5048 x2 U17.VOUTB  8/14/0xFF  0xFF
                  0x02 28V_DTJ   PD14  PB15                  MAC5048    U14.VOUTD  0/1/5      0xFF
                  0x03 12V_DYGY  PD9   GOK=PH6 / GOC=PH7     HQEF5016   U15.VOUTA  5/6/1      10
                  0x04 28V_QGSJ  PD1   PB9                   MAC5048    U17.VOUTA  11/11/8    0xFF
                  0x05 28V_SFXJ1 PD4   PB12                  MAC5048    U14.VOUTB  1/4/2      0xFF
                  0x06 28V_SFXJ2 PD7   PB10                  MAC5048    U14.VOUTC  2/2/4      0xFF
                  0x07 12V_GSDJ  PD12  GOK=PA2 / GOC=PA3     HQEF5016   U15.VOUTC  7/7/14     11
                  0x08 28V_WAOXJ PD8   PB13                  MAC5048    U16.VOUTD  6/5/0      0xFF
                  0x09 28V_HWXJ1 PD13  PB8                   MAC5048    U15.VOUTB  13/9/10    0xFF
                  0x0A 28V_HWXJ2 PD2   PB7                   MAC5048    U17.VOUTC  12/10/11   0xFF
                  0x0B 28V_HWXJ3 PD6   PB0                   MAC5048    U16.VOUTB  3/0/7      0xFF
                  0x0C 28V_HJJC1 PD3   PB5                   MAC5048    U16.VOUTC  10/12/9    0xFF
                  0x0D 28V_HJJC2 PD10  PB6                   MAC5048    U15.VOUTD  14/8/12    0xFF
                  0x0E 28V_HJJC3 PD11  PB14                  MAC5048    U14.VOUTA  4/3/3      0xFF
                  0x0F 28V_PD    PD5   PB3                   MAC5048    U16.VOUTA  9/13/13    0xFF
*/
const dev_map_t g_dev_map[DEV_NUM] = {
    /* [0] DEV_INVALID 占位槽 (全 0 填充) */
    [DEV_INVALID] = {
        DEV_INVALID,        /* id */
        { 0 },              /* name */
        0,                  /* en_port */
        0,                  /* en_pin */
        0,                  /* fault_cnt */
        { 0, 0 },           /* fault_port */
        { 0, 0 },           /* fault_pin */
        0,                  /* dac_idx */
        0,                  /* dac_ch */
        0xFF,               /* adc_ain_v */
        0xFF,               /* adc_ain_i */
        0xFF,               /* adc_ain_t */
        0xFF,               /* adc_ain_fault */
        0,                  /* chip_type */
    },
    /* [1] DEV_KF 28V_KF: 双 MAC5048 (KF1/KF2), 使能共享 PD0 */
    [DEV_KF] = {
        DEV_KF,                     /* id */
        "28V_KF",                   /* name */
        KF_PWR_EN_PORT,             /* en_port */
        KF_PWR_EN_PIN,              /* en_pin */
        2,                          /* fault_cnt (KF1 + KF2) */
        { KF1_FAULT_PORT, KF2_FAULT_PORT },    /* fault_port */
        { KF1_FAULT_PIN, KF2_FAULT_PIN },      /* fault_pin */
        DAC_IDX_CH3,                /* dac_idx (U17) */
        DAC_CH_B,                   /* dac_ch (VOUTB) */
        8,                          /* adc_ain_v */
        14,                         /* adc_ain_i */
        0xFF,                       /* adc_ain_t (KF 用 g_t_map 子表) */
        0xFF,                       /* adc_ain_fault */
        CHIP_TYPE_MAC5048,          /* chip_type */
    },
    /* [2] DEV_DTJ 28V_DTJ */
    [DEV_DTJ] = {
        DEV_DTJ,                    /* id */
        "28V_DTJ",                  /* name */
        DTJ_PWR_EN_PORT,            /* en_port */
        DTJ_PWR_EN_PIN,             /* en_pin */
        1,                          /* fault_cnt */
        { DTJ_FAULT_PORT, 0 },      /* fault_port */
        { DTJ_FAULT_PIN, 0 },       /* fault_pin */
        DAC_IDX_CH0,                /* dac_idx (U14) */
        DAC_CH_D,                   /* dac_ch (VOUTD) */
        0,                          /* adc_ain_v */
        1,                          /* adc_ain_i */
        5,                          /* adc_ain_t */
        0xFF,                       /* adc_ain_fault */
        CHIP_TYPE_MAC5048,          /* chip_type */
    },
    /* [3] DEV_DYGY 12V_DYGY: HQEF5016, 数字状态引脚 GOK/GOC, FAULT 走模拟量 AIN10 */
    [DEV_DYGY] = {
        DEV_DYGY,                   /* id */
        "12V_DYGY",                 /* name */
        DYGY_PWR_EN_PORT,           /* en_port */
        DYGY_PWR_EN_PIN,            /* en_pin */
        2,                          /* fault_cnt (GOK/GOC) */
        { DYGY_GOK_PORT, DYGY_GOC_PORT },      /* fault_port: [0]=GOK [1]=GOC */
        { DYGY_GOK_PIN, DYGY_GOC_PIN },        /* fault_pin */
        DAC_IDX_CH1,                /* dac_idx (U15) */
        DAC_CH_A,                   /* dac_ch (VOUTA) */
        5,                          /* adc_ain_v */
        6,                          /* adc_ain_i */
        1,                          /* adc_ain_t */
        10,                         /* adc_ain_fault (ADC_CH3 AIN10) */
        CHIP_TYPE_HQEF5016,         /* chip_type */
    },
    /* [4] DEV_QGSJ 28V_QGSJ */
    [DEV_QGSJ] = {
        DEV_QGSJ,                   /* id */
        "28V_QGSJ",                 /* name */
        QGSJ_PWR_EN_PORT,           /* en_port */
        QGSJ_PWR_EN_PIN,            /* en_pin */
        1,                          /* fault_cnt */
        { QGSJ_FAULT_PORT, 0 },     /* fault_port */
        { QGSJ_FAULT_PIN, 0 },      /* fault_pin */
        DAC_IDX_CH3,                /* dac_idx (U17) */
        DAC_CH_A,                   /* dac_ch (VOUTA) */
        11,                         /* adc_ain_v */
        11,                         /* adc_ain_i */
        8,                          /* adc_ain_t */
        0xFF,                       /* adc_ain_fault */
        CHIP_TYPE_MAC5048,          /* chip_type */
    },
    /* [5] DEV_SFXJ1 28V_SFXJ1 */
    [DEV_SFXJ1] = {
        DEV_SFXJ1,                  /* id */
        "28V_SFXJ1",                /* name */
        SFXJ1_PWR_EN_PORT,          /* en_port */
        SFXJ1_PWR_EN_PIN,           /* en_pin */
        1,                          /* fault_cnt */
        { SFXJ1_FAULT_PORT, 0 },    /* fault_port */
        { SFXJ1_FAULT_PIN, 0 },     /* fault_pin */
        DAC_IDX_CH0,                /* dac_idx (U14) */
        DAC_CH_B,                   /* dac_ch (VOUTB) */
        1,                          /* adc_ain_v */
        4,                          /* adc_ain_i */
        2,                          /* adc_ain_t */
        0xFF,                       /* adc_ain_fault */
        CHIP_TYPE_MAC5048,          /* chip_type */
    },
    /* [6] DEV_SFXJ2 28V_SFXJ2 */
    [DEV_SFXJ2] = {
        DEV_SFXJ2,                  /* id */
        "28V_SFXJ2",                /* name */
        SFXJ2_PWR_EN_PORT,          /* en_port */
        SFXJ2_PWR_EN_PIN,           /* en_pin */
        1,                          /* fault_cnt */
        { SFXJ2_FAULT_PORT, 0 },    /* fault_port */
        { SFXJ2_FAULT_PIN, 0 },     /* fault_pin */
        DAC_IDX_CH0,                /* dac_idx (U14) */
        DAC_CH_C,                   /* dac_ch (VOUTC) */
        2,                          /* adc_ain_v */
        2,                          /* adc_ain_i */
        4,                          /* adc_ain_t */
        0xFF,                       /* adc_ain_fault */
        CHIP_TYPE_MAC5048,          /* chip_type */
    },
    /* [7] DEV_GSDJ 12V_GSDJ: HQEF5016, GOK/GOC, FAULT 走模拟量 AIN11 */
    [DEV_GSDJ] = {
        DEV_GSDJ,                   /* id */
        "12V_GSDJ",                 /* name */
        GSDJ_PWR_EN_PORT,           /* en_port */
        GSDJ_PWR_EN_PIN,            /* en_pin */
        2,                          /* fault_cnt (GOK/GOC) */
        { GSDJ_GOK_PORT, GSDJ_GOC_PORT },      /* fault_port: [0]=GOK [1]=GOC */
        { GSDJ_GOK_PIN, GSDJ_GOC_PIN },        /* fault_pin */
        DAC_IDX_CH1,                /* dac_idx (U15) */
        DAC_CH_C,                   /* dac_ch (VOUTC) */
        7,                          /* adc_ain_v */
        7,                          /* adc_ain_i */
        14,                         /* adc_ain_t */
        11,                         /* adc_ain_fault (ADC_CH3 AIN11) */
        CHIP_TYPE_HQEF5016,         /* chip_type */
    },
    /* [8] DEV_WAOXJ 28V_WAOXJ */
    [DEV_WAOXJ] = {
        DEV_WAOXJ,                  /* id */
        "28V_WAOXJ",                /* name */
        WAOXJ_PWR_EN_PORT,          /* en_port */
        WAOXJ_PWR_EN_PIN,           /* en_pin */
        1,                          /* fault_cnt */
        { WAOXJ_FAULT_PORT, 0 },    /* fault_port */
        { WAOXJ_FAULT_PIN, 0 },     /* fault_pin */
        DAC_IDX_CH2,                /* dac_idx (U16) */
        DAC_CH_D,                   /* dac_ch (VOUTD) */
        6,                          /* adc_ain_v */
        5,                          /* adc_ain_i */
        0,                          /* adc_ain_t */
        0xFF,                       /* adc_ain_fault */
        CHIP_TYPE_MAC5048,          /* chip_type */
    },
    /* [9] DEV_HWXJ1 28V_HWXJ1 */
    [DEV_HWXJ1] = {
        DEV_HWXJ1,                  /* id */
        "28V_HWXJ1",                /* name */
        HWXJ1_PWR_EN_PORT,          /* en_port */
        HWXJ1_PWR_EN_PIN,           /* en_pin */
        1,                          /* fault_cnt */
        { HWXJ1_FAULT_PORT, 0 },    /* fault_port */
        { HWXJ1_FAULT_PIN, 0 },     /* fault_pin */
        DAC_IDX_CH1,                /* dac_idx (U15) */
        DAC_CH_B,                   /* dac_ch (VOUTB) */
        13,                         /* adc_ain_v */
        9,                          /* adc_ain_i */
        10,                         /* adc_ain_t */
        0xFF,                       /* adc_ain_fault */
        CHIP_TYPE_MAC5048,          /* chip_type */
    },
    /* [10] DEV_HWXJ2 28V_HWXJ2 */
    [DEV_HWXJ2] = {
        DEV_HWXJ2,                  /* id */
        "28V_HWXJ2",                /* name */
        HWXJ2_PWR_EN_PORT,          /* en_port */
        HWXJ2_PWR_EN_PIN,           /* en_pin */
        1,                          /* fault_cnt */
        { HWXJ2_FAULT_PORT, 0 },    /* fault_port */
        { HWXJ2_FAULT_PIN, 0 },     /* fault_pin */
        DAC_IDX_CH3,                /* dac_idx (U17) */
        DAC_CH_C,                   /* dac_ch (VOUTC) */
        12,                         /* adc_ain_v */
        10,                         /* adc_ain_i */
        11,                         /* adc_ain_t */
        0xFF,                       /* adc_ain_fault */
        CHIP_TYPE_MAC5048,          /* chip_type */
    },
    /* [11] DEV_HWXJ3 28V_HWXJ3 */
    [DEV_HWXJ3] = {
        DEV_HWXJ3,                  /* id */
        "28V_HWXJ3",                /* name */
        HWXJ3_PWR_EN_PORT,          /* en_port */
        HWXJ3_PWR_EN_PIN,           /* en_pin */
        1,                          /* fault_cnt */
        { HWXJ3_FAULT_PORT, 0 },    /* fault_port */
        { HWXJ3_FAULT_PIN, 0 },     /* fault_pin */
        DAC_IDX_CH2,                /* dac_idx (U16) */
        DAC_CH_B,                   /* dac_ch (VOUTB) */
        3,                          /* adc_ain_v */
        0,                          /* adc_ain_i */
        7,                          /* adc_ain_t */
        0xFF,                       /* adc_ain_fault */
        CHIP_TYPE_MAC5048,          /* chip_type */
    },
    /* [12] DEV_HJJC1 28V_HJJC1 */
    [DEV_HJJC1] = {
        DEV_HJJC1,                  /* id */
        "28V_HJJC1",                /* name */
        HJJC1_PWR_EN_PORT,          /* en_port */
        HJJC1_PWR_EN_PIN,           /* en_pin */
        1,                          /* fault_cnt */
        { HJJC1_FAULT_PORT, 0 },    /* fault_port */
        { HJJC1_FAULT_PIN, 0 },     /* fault_pin */
        DAC_IDX_CH2,                /* dac_idx (U16) */
        DAC_CH_C,                   /* dac_ch (VOUTC) */
        10,                         /* adc_ain_v */
        12,                         /* adc_ain_i */
        9,                          /* adc_ain_t */
        0xFF,                       /* adc_ain_fault */
        CHIP_TYPE_MAC5048,          /* chip_type */
    },
    /* [13] DEV_HJJC2 28V_HJJC2 */
    [DEV_HJJC2] = {
        DEV_HJJC2,                  /* id */
        "28V_HJJC2",                /* name */
        HJJC2_PWR_EN_PORT,          /* en_port */
        HJJC2_PWR_EN_PIN,           /* en_pin */
        1,                          /* fault_cnt */
        { HJJC2_FAULT_PORT, 0 },    /* fault_port */
        { HJJC2_FAULT_PIN, 0 },     /* fault_pin */
        DAC_IDX_CH1,                /* dac_idx (U15) */
        DAC_CH_D,                   /* dac_ch (VOUTD) */
        14,                         /* adc_ain_v */
        8,                          /* adc_ain_i */
        12,                         /* adc_ain_t */
        0xFF,                       /* adc_ain_fault */
        CHIP_TYPE_MAC5048,          /* chip_type */
    },
    /* [14] DEV_HJJC3 28V_HJJC3 */
    [DEV_HJJC3] = {
        DEV_HJJC3,                  /* id */
        "28V_HJJC3",                /* name */
        HJJC3_PWR_EN_PORT,          /* en_port */
        HJJC3_PWR_EN_PIN,           /* en_pin */
        1,                          /* fault_cnt */
        { HJJC3_FAULT_PORT, 0 },    /* fault_port */
        { HJJC3_FAULT_PIN, 0 },     /* fault_pin */
        DAC_IDX_CH0,                /* dac_idx (U14) */
        DAC_CH_A,                   /* dac_ch (VOUTA) */
        4,                          /* adc_ain_v */
        3,                          /* adc_ain_i */
        3,                          /* adc_ain_t */
        0xFF,                       /* adc_ain_fault */
        CHIP_TYPE_MAC5048,          /* chip_type */
    },
    /* [15] DEV_PD 28V_PD */
    [DEV_PD] = {
        DEV_PD,                     /* id */
        "28V_PD",                   /* name */
        PD_PWR_EN_PORT,             /* en_port */
        PD_PWR_EN_PIN,              /* en_pin */
        1,                          /* fault_cnt */
        { PD_FAULT_PORT, 0 },       /* fault_port */
        { PD_FAULT_PIN, 0 },        /* fault_pin */
        DAC_IDX_CH2,                /* dac_idx (U16) */
        DAC_CH_A,                   /* dac_ch (VOUTA) */
        9,                          /* adc_ain_v */
        13,                         /* adc_ain_i */
        13,                         /* adc_ain_t */
        0xFF,                       /* adc_ain_fault */
        CHIP_TYPE_MAC5048,          /* chip_type */
    },
};

/*
    @brief      : 温度通道子表 (ADC_CH2/U8), 索引 = AIN
                  AIN: 0=WAOXJ 1=DYGY 2=SFXJ1 3=HJJC3 4=SFXJ2 5=DTJ 6=KF2 7=HWXJ3
                       8=QGSJ 9=HJJC1 10=HWXJ1 11=HWXJ2 12=HJJC2 13=PD 14=GSDJ 15=KF1
                  sub: 0=主通道 1=KF1 2=KF2
*/
const t_map_t g_t_map[16] = {
    [0]  = { DEV_WAOXJ, 0 },   /* AIN0  */
    [1]  = { DEV_DYGY,  0 },   /* AIN1  */
    [2]  = { DEV_SFXJ1, 0 },   /* AIN2  */
    [3]  = { DEV_HJJC3, 0 },   /* AIN3  */
    [4]  = { DEV_SFXJ2, 0 },   /* AIN4  */
    [5]  = { DEV_DTJ,   0 },   /* AIN5  */
    [6]  = { DEV_KF,    2 },   /* AIN6  KF2 温度 */
    [7]  = { DEV_HWXJ3, 0 },   /* AIN7  */
    [8]  = { DEV_QGSJ,  0 },   /* AIN8  */
    [9]  = { DEV_HJJC1, 0 },   /* AIN9  */
    [10] = { DEV_HWXJ1, 0 },   /* AIN10 */
    [11] = { DEV_HWXJ2, 0 },   /* AIN11 */
    [12] = { DEV_HJJC2, 0 },   /* AIN12 */
    [13] = { DEV_PD,    0 },   /* AIN13 */
    [14] = { DEV_GSDJ,  0 },   /* AIN14 */
    [15] = { DEV_KF,    1 },   /* AIN15 KF1 温度 */
};

/* 编译期检查: 表长度与设备 ID 枚举一致, 防止 ID 直索引越界/错位 */
typedef char g_dev_map_size_check[(sizeof(g_dev_map) / sizeof(g_dev_map[0]) == DEV_NUM &&
                                   DEV_PD == DEV_NUM - 1) ? 1 : -1];
typedef char g_t_map_size_check[(sizeof(g_t_map) / sizeof(g_t_map[0]) == 16) ? 1 : -1];

/*
    @brief      : DAC (GDA6641 x4) 通道组引脚表, 索引 = DAC_IDX_xxx
    @note       : 字段顺序与 gda6641_pin_t 一致:
                  din / clr / por / ldac / sync / sclk / sdo
*/
const gda6641_pin_t g_dac_pin_map[4] = {
    /* [DAC_IDX_CH0] U14, GPIOI */
    {
        { DAC_CH0_DIN_PORT,  DAC_CH0_DIN_PIN  },   /* din  OUT_PP */
        { DAC_CH0_CLR_PORT,  DAC_CH0_CLR_PIN  },   /* clr  OUT_PP */
        { DAC_CH0_POR_PORT,  DAC_CH0_POR_PIN  },   /* por  OUT_PP */
        { DAC_CH0_LDAC_PORT, DAC_CH0_LDAC_PIN },   /* ldac OUT_PP */
        { DAC_CH0_SYNC_PORT, DAC_CH0_SYNC_PIN },   /* sync OUT_PP */
        { DAC_CH0_SCLK_PORT, DAC_CH0_SCLK_PIN },   /* sclk OUT_PP */
        { DAC_CH0_SDO_PORT,  DAC_CH0_SDO_PIN  },   /* sdo  IN */
    },
    /* [DAC_IDX_CH1] U15, GPIOG */
    {
        { DAC_CH1_DIN_PORT,  DAC_CH1_DIN_PIN  },   /* din  OUT_PP */
        { DAC_CH1_CLR_PORT,  DAC_CH1_CLR_PIN  },   /* clr  OUT_PP */
        { DAC_CH1_POR_PORT,  DAC_CH1_POR_PIN  },   /* por  OUT_PP */
        { DAC_CH1_LDAC_PORT, DAC_CH1_LDAC_PIN },   /* ldac OUT_PP */
        { DAC_CH1_SYNC_PORT, DAC_CH1_SYNC_PIN },   /* sync OUT_PP */
        { DAC_CH1_SCLK_PORT, DAC_CH1_SCLK_PIN },   /* sclk OUT_PP */
        { DAC_CH1_SDO_PORT,  DAC_CH1_SDO_PIN  },   /* sdo  IN */
    },
    /* [DAC_IDX_CH2] U16, GPIOG */
    {
        { DAC_CH2_DIN_PORT,  DAC_CH2_DIN_PIN  },   /* din  OUT_PP */
        { DAC_CH2_CLR_PORT,  DAC_CH2_CLR_PIN  },   /* clr  OUT_PP */
        { DAC_CH2_POR_PORT,  DAC_CH2_POR_PIN  },   /* por  OUT_PP */
        { DAC_CH2_LDAC_PORT, DAC_CH2_LDAC_PIN },   /* ldac OUT_PP */
        { DAC_CH2_SYNC_PORT, DAC_CH2_SYNC_PIN },   /* sync OUT_PP */
        { DAC_CH2_SCLK_PORT, DAC_CH2_SCLK_PIN },   /* sclk OUT_PP */
        { DAC_CH2_SDO_PORT,  DAC_CH2_SDO_PIN  },   /* sdo  IN */
    },
    /* [DAC_IDX_CH3] U17, GPIOE */
    {
        { DAC_CH3_DIN_PORT,  DAC_CH3_DIN_PIN  },   /* din  OUT_PP */
        { DAC_CH3_CLR_PORT,  DAC_CH3_CLR_PIN  },   /* clr  OUT_PP */
        { DAC_CH3_POR_PORT,  DAC_CH3_POR_PIN  },   /* por  OUT_PP */
        { DAC_CH3_LDAC_PORT, DAC_CH3_LDAC_PIN },   /* ldac OUT_PP */
        { DAC_CH3_SYNC_PORT, DAC_CH3_SYNC_PIN },   /* sync OUT_PP */
        { DAC_CH3_SCLK_PORT, DAC_CH3_SCLK_PIN },   /* sclk OUT_PP */
        { DAC_CH3_SDO_PORT,  DAC_CH3_SDO_PIN  },   /* sdo  IN */
    },
};

/*
    @brief      : ADC (LC1258 x4) 通道组引脚表, 索引 = ADC_IDX_xxx
    @note       : 字段顺序与 lc1258_pin_t 一致:
                  din / sclk / start / drdy / out / rst / cs
                  OUT (DOUT) 按输入定义 (待确认 #3: 原理图标注"模拟, 跳过 GPIO")
*/
const lc1258_pin_t g_adc_pin_map[4] = {
    /* [ADC_IDX_CH0] U2 电压监测, GPIOH */
    {
        { ADC_CH0_DIN_PORT,   ADC_CH0_DIN_PIN   },  /* din   OUT_PP */
        { ADC_CH0_SCLK_PORT,  ADC_CH0_SCLK_PIN  },  /* sclk  OUT_PP */
        { ADC_CH0_START_PORT, ADC_CH0_START_PIN },  /* start OUT_PP */
        { ADC_CH0_DRDY_PORT,  ADC_CH0_DRDY_PIN  },  /* drdy  IN */
        { ADC_CH0_OUT_PORT,   ADC_CH0_OUT_PIN   },  /* out   IN */
        { ADC_CH0_RST_PORT,   ADC_CH0_RST_PIN   },  /* rst   OUT_PP */
        { ADC_CH0_CS_PORT,    ADC_CH0_CS_PIN    },  /* cs    OUT_PP */
    },
    /* [ADC_IDX_CH1] U5 电流监测, GPIOE (PE13 空置) */
    {
        { ADC_CH1_DIN_PORT,   ADC_CH1_DIN_PIN   },  /* din   OUT_PP */
        { ADC_CH1_SCLK_PORT,  ADC_CH1_SCLK_PIN  },  /* sclk  OUT_PP */
        { ADC_CH1_START_PORT, ADC_CH1_START_PIN },  /* start OUT_PP */
        { ADC_CH1_DRDY_PORT,  ADC_CH1_DRDY_PIN  },  /* drdy  IN */
        { ADC_CH1_OUT_PORT,   ADC_CH1_OUT_PIN   },  /* out   IN */
        { ADC_CH1_RST_PORT,   ADC_CH1_RST_PIN   },  /* rst   OUT_PP */
        { ADC_CH1_CS_PORT,    ADC_CH1_CS_PIN    },  /* cs    OUT_PP */
    },
    /* [ADC_IDX_CH2] U8 温度监测, GPIOF */
    {
        { ADC_CH2_DIN_PORT,   ADC_CH2_DIN_PIN   },  /* din   OUT_PP */
        { ADC_CH2_SCLK_PORT,  ADC_CH2_SCLK_PIN  },  /* sclk  OUT_PP */
        { ADC_CH2_START_PORT, ADC_CH2_START_PIN },  /* start OUT_PP */
        { ADC_CH2_DRDY_PORT,  ADC_CH2_DRDY_PIN  },  /* drdy  IN */
        { ADC_CH2_OUT_PORT,   ADC_CH2_OUT_PIN   },  /* out   IN */
        { ADC_CH2_RST_PORT,   ADC_CH2_RST_PIN   },  /* rst   OUT_PP */
        { ADC_CH2_CS_PORT,    ADC_CH2_CS_PIN    },  /* cs    OUT_PP */
    },
    /* [ADC_IDX_CH3] U11 电源/辅助监测, GPIOF (PF13 空置) */
    {
        { ADC_CH3_DIN_PORT,   ADC_CH3_DIN_PIN   },  /* din   OUT_PP */
        { ADC_CH3_SCLK_PORT,  ADC_CH3_SCLK_PIN  },  /* sclk  OUT_PP */
        { ADC_CH3_START_PORT, ADC_CH3_START_PIN },  /* start OUT_PP */
        { ADC_CH3_DRDY_PORT,  ADC_CH3_DRDY_PIN  },  /* drdy  IN */
        { ADC_CH3_OUT_PORT,   ADC_CH3_OUT_PIN   },  /* out   IN */
        { ADC_CH3_RST_PORT,   ADC_CH3_RST_PIN   },  /* rst   OUT_PP */
        { ADC_CH3_CS_PORT,    ADC_CH3_CS_PIN    },  /* cs    OUT_PP */
    },
};

/*
    @brief      : 板级 GPIO 初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void bsp_board_init(void)
{
    GPIO_InitTypeDef gpio_init;

    /* 1. 使能 GPIOA~GPIOI 时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB |
                           RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD |
                           RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF |
                           RCC_AHB1Periph_GPIOG | RCC_AHB1Periph_GPIOH |
                           RCC_AHB1Periph_GPIOI, ENABLE);

    /* 2. 关闭 JTAG 保留 SWD, 释放 PA15 作 KF2_FAULT 输入
          注意: 本工程 stm32f4xx.h 的 SYSCFG_TypeDef 无 MAPR 成员
          (F405/407 无 MAPR 寄存器), SWJ_CFG 位域位于 SYSCFG_MEMRMP[26:24],
          写入 0b010 = JTAG-DP 禁用, SW-DP 保留 (参照旧 mac5048.c 思路) */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    SYSCFG->MEMRMP |= SWJ_CFG_JTAG_DISABLE;

    /* 3. 15 路电源使能 EN 输出 (GPIOD PD0~PD14), 推挽输出
          初始全部拉低 (上电默认全断电, 由 App 层按需开启) */
    gpio_init.GPIO_Pin   = KF_PWR_EN_PIN    | QGSJ_PWR_EN_PIN | HWXJ2_PWR_EN_PIN |
                           HJJC1_PWR_EN_PIN | SFXJ1_PWR_EN_PIN | PD_PWR_EN_PIN |
                           HWXJ3_PWR_EN_PIN | SFXJ2_PWR_EN_PIN | WAOXJ_PWR_EN_PIN |
                           DYGY_PWR_EN_PIN  | HJJC2_PWR_EN_PIN | HJJC3_PWR_EN_PIN |
                           GSDJ_PWR_EN_PIN  | HWXJ1_PWR_EN_PIN | DTJ_PWR_EN_PIN;
    gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &gpio_init);
    GPIO_ResetBits(GPIOD, gpio_init.GPIO_Pin);

    /* 4. 模块状态输入 (FAULT/GOK/GOC/ALERT), 上拉输入, 100MHz */
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_UP;

    /* GPIOA: GSDJ_GOK / GSDJ_GOC / KF2_FAULT
       注意: 跳过 PA13/PA14 (SWD 调试口, 保留调试功能) */
    gpio_init.GPIO_Pin   = GSDJ_GOK_PIN | GSDJ_GOC_PIN | KF2_FAULT_PIN;
    GPIO_Init(GPIOA, &gpio_init);

    /* GPIOB: 13 路 MAC5048 FAULT 输入
       注意: 跳过 PB1 (BOOT1, 硬件上电 BOOT 采样, 软件不操作) */
    gpio_init.GPIO_Pin   = HWXJ3_FAULT_PIN | PD_FAULT_PIN   | KF1_FAULT_PIN |
                           HJJC1_FAULT_PIN | HJJC2_FAULT_PIN | HWXJ2_FAULT_PIN |
                           HWXJ1_FAULT_PIN | QGSJ_FAULT_PIN | SFXJ2_FAULT_PIN |
                           SFXJ1_FAULT_PIN | WAOXJ_FAULT_PIN | HJJC3_FAULT_PIN |
                           DTJ_FAULT_PIN;
    GPIO_Init(GPIOB, &gpio_init);

    /* GPIOC: 4 路 XCA4001 ALERT 输入
       注意: 跳过 PC14/PC15 (LSE 32.768K 晶振 OSC32_IN/OUT) */
    gpio_init.GPIO_Pin   = VCC_3V3_ALERT_PIN  | VCC_5V0_ALERT_PIN |
                           VCC_28V0_ALERT_PIN | VCC_12V0_ALERT_PIN;
    GPIO_Init(GPIOC, &gpio_init);

    /* GPIOH: DYGY_GOK / DYGY_GOC (HQEF5016 电源状态) */
    gpio_init.GPIO_Pin   = DYGY_GOK_PIN | DYGY_GOC_PIN;
    GPIO_Init(GPIOH, &gpio_init);

    /* 5. 电源复位输出 XCA4001 RESET (GPIOC PC0~PC3), 推挽输出
          初始高电平 (RESET 低有效, 锁存模式不动作) */
    gpio_init.GPIO_Pin   = VCC_12V0_RST_PIN | VCC_3V3_RST_PIN |
                           VCC_5V0_RST_PIN  | VCC_28V0_RST_PIN;
    gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOC, &gpio_init);
    GPIO_SetBits(GPIOC, gpio_init.GPIO_Pin);

    /* 6. DAC 通道组 (GDA6641 x4)
          SCLK/SYNC/DIN/LDAC/POR/CLR 输出, SDO 输入
          初始电平: SCLK=0, SYNC=1, LDAC=1, POR=1, CLR=1, DIN=0 */

    /* DAC_CH0 (U14, GPIOI) */
    gpio_init.GPIO_Pin   = DAC_CH0_DIN_PIN  | DAC_CH0_CLR_PIN | DAC_CH0_POR_PIN |
                           DAC_CH0_LDAC_PIN | DAC_CH0_SYNC_PIN | DAC_CH0_SCLK_PIN;
    GPIO_Init(DAC_CH0_DIN_PORT, &gpio_init);
    GPIO_SetBits(DAC_CH0_DIN_PORT, DAC_CH0_CLR_PIN | DAC_CH0_POR_PIN |
                                   DAC_CH0_LDAC_PIN | DAC_CH0_SYNC_PIN);
    GPIO_ResetBits(DAC_CH0_DIN_PORT, DAC_CH0_DIN_PIN | DAC_CH0_SCLK_PIN);
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    gpio_init.GPIO_Pin   = DAC_CH0_SDO_PIN;
    GPIO_Init(DAC_CH0_SDO_PORT, &gpio_init);

    /* DAC_CH1 (U15, GPIOG) */
    gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    gpio_init.GPIO_Pin   = DAC_CH1_DIN_PIN  | DAC_CH1_CLR_PIN | DAC_CH1_POR_PIN |
                           DAC_CH1_LDAC_PIN | DAC_CH1_SYNC_PIN | DAC_CH1_SCLK_PIN;
    GPIO_Init(DAC_CH1_DIN_PORT, &gpio_init);
    GPIO_SetBits(DAC_CH1_DIN_PORT, DAC_CH1_CLR_PIN | DAC_CH1_POR_PIN |
                                   DAC_CH1_LDAC_PIN | DAC_CH1_SYNC_PIN);
    GPIO_ResetBits(DAC_CH1_DIN_PORT, DAC_CH1_DIN_PIN | DAC_CH1_SCLK_PIN);
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    gpio_init.GPIO_Pin   = DAC_CH1_SDO_PIN;
    GPIO_Init(DAC_CH1_SDO_PORT, &gpio_init);

    /* DAC_CH2 (U16, GPIOG) */
    gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    gpio_init.GPIO_Pin   = DAC_CH2_DIN_PIN  | DAC_CH2_CLR_PIN | DAC_CH2_POR_PIN |
                           DAC_CH2_LDAC_PIN | DAC_CH2_SYNC_PIN | DAC_CH2_SCLK_PIN;
    GPIO_Init(DAC_CH2_DIN_PORT, &gpio_init);
    GPIO_SetBits(DAC_CH2_DIN_PORT, DAC_CH2_CLR_PIN | DAC_CH2_POR_PIN |
                                   DAC_CH2_LDAC_PIN | DAC_CH2_SYNC_PIN);
    GPIO_ResetBits(DAC_CH2_DIN_PORT, DAC_CH2_DIN_PIN | DAC_CH2_SCLK_PIN);
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    gpio_init.GPIO_Pin   = DAC_CH2_SDO_PIN;
    GPIO_Init(DAC_CH2_SDO_PORT, &gpio_init);

    /* DAC_CH3 (U17, GPIOE) */
    gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    gpio_init.GPIO_Pin   = DAC_CH3_DIN_PIN  | DAC_CH3_CLR_PIN | DAC_CH3_POR_PIN |
                           DAC_CH3_LDAC_PIN | DAC_CH3_SYNC_PIN | DAC_CH3_SCLK_PIN;
    GPIO_Init(DAC_CH3_DIN_PORT, &gpio_init);
    GPIO_SetBits(DAC_CH3_DIN_PORT, DAC_CH3_CLR_PIN | DAC_CH3_POR_PIN |
                                   DAC_CH3_LDAC_PIN | DAC_CH3_SYNC_PIN);
    GPIO_ResetBits(DAC_CH3_DIN_PORT, DAC_CH3_DIN_PIN | DAC_CH3_SCLK_PIN);
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    gpio_init.GPIO_Pin   = DAC_CH3_SDO_PIN;
    GPIO_Init(DAC_CH3_SDO_PORT, &gpio_init);

    /* 7. ADC 通道组 (LC1258 x4)
          CS/SCLK/DIN/START/RST 输出, DRDY/OUT (DOUT) 输入
          初始电平: CS=1 (空闲), SCLK=0, DIN=0, START=0, RST=1
          DRDY 低有效/开漏输出, 上拉防止浮空误判;
          OUT (DOUT) 芯片推挽输出, 保持 NOPULL (待确认 #3: 原理图标注"模拟, 跳过 GPIO") */

    /* ADC_CH0 (U2 电压监测, GPIOH) */
    gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    gpio_init.GPIO_Pin   = ADC_CH0_DIN_PIN  | ADC_CH0_SCLK_PIN | ADC_CH0_START_PIN |
                           ADC_CH0_RST_PIN  | ADC_CH0_CS_PIN;
    GPIO_Init(ADC_CH0_DIN_PORT, &gpio_init);
    GPIO_SetBits(ADC_CH0_DIN_PORT, ADC_CH0_RST_PIN | ADC_CH0_CS_PIN);
    GPIO_ResetBits(ADC_CH0_DIN_PORT, ADC_CH0_DIN_PIN | ADC_CH0_SCLK_PIN |
                                     ADC_CH0_START_PIN);
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_UP;
    gpio_init.GPIO_Pin   = ADC_CH0_DRDY_PIN;
    GPIO_Init(ADC_CH0_DRDY_PORT, &gpio_init);
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    gpio_init.GPIO_Pin   = ADC_CH0_OUT_PIN;
    GPIO_Init(ADC_CH0_OUT_PORT, &gpio_init);

    /* ADC_CH1 (U5 电流监测, GPIOE, PE13 空置) */
    gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    gpio_init.GPIO_Pin   = ADC_CH1_DIN_PIN  | ADC_CH1_SCLK_PIN | ADC_CH1_START_PIN |
                           ADC_CH1_RST_PIN  | ADC_CH1_CS_PIN;
    GPIO_Init(ADC_CH1_DIN_PORT, &gpio_init);
    GPIO_SetBits(ADC_CH1_DIN_PORT, ADC_CH1_RST_PIN | ADC_CH1_CS_PIN);
    GPIO_ResetBits(ADC_CH1_DIN_PORT, ADC_CH1_DIN_PIN | ADC_CH1_SCLK_PIN |
                                     ADC_CH1_START_PIN);
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_UP;
    gpio_init.GPIO_Pin   = ADC_CH1_DRDY_PIN;
    GPIO_Init(ADC_CH1_DRDY_PORT, &gpio_init);
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    gpio_init.GPIO_Pin   = ADC_CH1_OUT_PIN;
    GPIO_Init(ADC_CH1_OUT_PORT, &gpio_init);

    /* ADC_CH2 (U8 温度监测, GPIOF) */
    gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    gpio_init.GPIO_Pin   = ADC_CH2_DIN_PIN  | ADC_CH2_SCLK_PIN | ADC_CH2_START_PIN |
                           ADC_CH2_RST_PIN  | ADC_CH2_CS_PIN;
    GPIO_Init(ADC_CH2_DIN_PORT, &gpio_init);
    GPIO_SetBits(ADC_CH2_DIN_PORT, ADC_CH2_RST_PIN | ADC_CH2_CS_PIN);
    GPIO_ResetBits(ADC_CH2_DIN_PORT, ADC_CH2_DIN_PIN | ADC_CH2_SCLK_PIN |
                                     ADC_CH2_START_PIN);
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_UP;
    gpio_init.GPIO_Pin   = ADC_CH2_DRDY_PIN;
    GPIO_Init(ADC_CH2_DRDY_PORT, &gpio_init);
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    gpio_init.GPIO_Pin   = ADC_CH2_OUT_PIN;
    GPIO_Init(ADC_CH2_OUT_PORT, &gpio_init);

    /* ADC_CH3 (U11 电源/辅助监测, GPIOF, PF13 空置) */
    gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    gpio_init.GPIO_Pin   = ADC_CH3_DIN_PIN  | ADC_CH3_SCLK_PIN | ADC_CH3_START_PIN |
                           ADC_CH3_RST_PIN  | ADC_CH3_CS_PIN;
    GPIO_Init(ADC_CH3_DIN_PORT, &gpio_init);
    GPIO_SetBits(ADC_CH3_DIN_PORT, ADC_CH3_RST_PIN | ADC_CH3_CS_PIN);
    GPIO_ResetBits(ADC_CH3_DIN_PORT, ADC_CH3_DIN_PIN | ADC_CH3_SCLK_PIN |
                                     ADC_CH3_START_PIN);
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_UP;
    gpio_init.GPIO_Pin   = ADC_CH3_DRDY_PIN;
    GPIO_Init(ADC_CH3_DRDY_PORT, &gpio_init);
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    gpio_init.GPIO_Pin   = ADC_CH3_OUT_PIN;
    GPIO_Init(ADC_CH3_OUT_PORT, &gpio_init);

    /* 8. 外设复用 AF 引脚 (SPI1: PA4~PA7, USART1: PA9/PA10)
          由 bsp_spi / bsp_usart 外设模块初始化 (Task 4/5), 此处不配置 */
}
