#ifndef __BOARD_MAP_H_
#define __BOARD_MAP_H_

#include "types_def.h"
#include "stm32f4xx.h"

/*
    @brief      : DYGLB 电源管理板全板引脚定义总表 (STM32F407IGTx, LQFP176)
    @note       : 数据来源: doc\原理图管脚连接关系.txt
                  g_dev_map[] / g_t_map[] / g_dac_pin_map[] / g_adc_pin_map[]
                  由 bsp_board.c 实例化, 本文件只做类型与声明
*/

/*============================================================
    一、15 路电源使能输出 (MAC5048/HQEF5016 EN, 推挽输出, GPIOD)
============================================================*/
#define KF_PWR_EN_PORT      GPIOD
#define KF_PWR_EN_PIN       GPIO_Pin_0

#define QGSJ_PWR_EN_PORT    GPIOD
#define QGSJ_PWR_EN_PIN     GPIO_Pin_1

#define HWXJ2_PWR_EN_PORT   GPIOD
#define HWXJ2_PWR_EN_PIN    GPIO_Pin_2

#define HJJC1_PWR_EN_PORT   GPIOD
#define HJJC1_PWR_EN_PIN    GPIO_Pin_3

#define SFXJ1_PWR_EN_PORT   GPIOD
#define SFXJ1_PWR_EN_PIN    GPIO_Pin_4

#define PD_PWR_EN_PORT      GPIOD
#define PD_PWR_EN_PIN       GPIO_Pin_5

#define HWXJ3_PWR_EN_PORT   GPIOD
#define HWXJ3_PWR_EN_PIN    GPIO_Pin_6

#define SFXJ2_PWR_EN_PORT   GPIOD
#define SFXJ2_PWR_EN_PIN    GPIO_Pin_7

#define WAOXJ_PWR_EN_PORT   GPIOD
#define WAOXJ_PWR_EN_PIN    GPIO_Pin_8

#define DYGY_PWR_EN_PORT    GPIOD
#define DYGY_PWR_EN_PIN     GPIO_Pin_9

#define HJJC2_PWR_EN_PORT   GPIOD
#define HJJC2_PWR_EN_PIN    GPIO_Pin_10

#define HJJC3_PWR_EN_PORT   GPIOD
#define HJJC3_PWR_EN_PIN    GPIO_Pin_11

#define GSDJ_PWR_EN_PORT    GPIOD
#define GSDJ_PWR_EN_PIN     GPIO_Pin_12

#define HWXJ1_PWR_EN_PORT   GPIOD
#define HWXJ1_PWR_EN_PIN    GPIO_Pin_13

#define DTJ_PWR_EN_PORT     GPIOD
#define DTJ_PWR_EN_PIN      GPIO_Pin_14

/*============================================================
    二、模块状态输入 (FAULT/GOK/GOC/ALERT, 上拉输入)
============================================================*/
#define GSDJ_GOK_PORT       GPIOA
#define GSDJ_GOK_PIN        GPIO_Pin_2

#define GSDJ_GOC_PORT       GPIOA
#define GSDJ_GOC_PIN        GPIO_Pin_3

#define KF2_FAULT_PORT      GPIOA
#define KF2_FAULT_PIN       GPIO_Pin_15

#define HWXJ3_FAULT_PORT    GPIOB
#define HWXJ3_FAULT_PIN     GPIO_Pin_0

#define PD_FAULT_PORT       GPIOB
#define PD_FAULT_PIN        GPIO_Pin_3

#define KF1_FAULT_PORT      GPIOB
#define KF1_FAULT_PIN       GPIO_Pin_4

#define HJJC1_FAULT_PORT    GPIOB
#define HJJC1_FAULT_PIN     GPIO_Pin_5

#define HJJC2_FAULT_PORT    GPIOB
#define HJJC2_FAULT_PIN     GPIO_Pin_6

#define HWXJ2_FAULT_PORT    GPIOB
#define HWXJ2_FAULT_PIN     GPIO_Pin_7

#define HWXJ1_FAULT_PORT    GPIOB
#define HWXJ1_FAULT_PIN     GPIO_Pin_8

#define QGSJ_FAULT_PORT     GPIOB
#define QGSJ_FAULT_PIN      GPIO_Pin_9

#define SFXJ2_FAULT_PORT    GPIOB
#define SFXJ2_FAULT_PIN     GPIO_Pin_10

#define SFXJ1_FAULT_PORT    GPIOB
#define SFXJ1_FAULT_PIN     GPIO_Pin_12

#define WAOXJ_FAULT_PORT    GPIOB
#define WAOXJ_FAULT_PIN     GPIO_Pin_13

#define HJJC3_FAULT_PORT    GPIOB
#define HJJC3_FAULT_PIN     GPIO_Pin_14

#define DTJ_FAULT_PORT      GPIOB
#define DTJ_FAULT_PIN       GPIO_Pin_15

#define VCC_3V3_ALERT_PORT  GPIOC
#define VCC_3V3_ALERT_PIN   GPIO_Pin_4

#define VCC_5V0_ALERT_PORT  GPIOC
#define VCC_5V0_ALERT_PIN   GPIO_Pin_5

#define VCC_28V0_ALERT_PORT GPIOC
#define VCC_28V0_ALERT_PIN  GPIO_Pin_12

#define VCC_12V0_ALERT_PORT GPIOC
#define VCC_12V0_ALERT_PIN  GPIO_Pin_13

#define DYGY_GOK_PORT       GPIOH
#define DYGY_GOK_PIN        GPIO_Pin_6

#define DYGY_GOC_PORT       GPIOH
#define DYGY_GOC_PIN        GPIO_Pin_7

/*============================================================
    三、电源复位输出 (XCA4001 RESET, 推挽输出)
============================================================*/
#define VCC_12V0_RST_PORT   GPIOC
#define VCC_12V0_RST_PIN    GPIO_Pin_0

#define VCC_3V3_RST_PORT    GPIOC
#define VCC_3V3_RST_PIN     GPIO_Pin_1

#define VCC_5V0_RST_PORT    GPIOC
#define VCC_5V0_RST_PIN     GPIO_Pin_2

#define VCC_28V0_RST_PORT   GPIOC
#define VCC_28V0_RST_PIN    GPIO_Pin_3

/*============================================================
    四、外设复用 AF 引脚 (SPI1/USART1)
        由外设初始化配置复用, 不进普通 GPIO 初始化表
============================================================*/
#define PWR_SPI_NSS_EX_PORT     GPIOA
#define PWR_SPI_NSS_EX_PIN      GPIO_Pin_4

#define PWR_SPI_CLK_EX_PORT     GPIOA
#define PWR_SPI_CLK_EX_PIN      GPIO_Pin_5

#define PWR_SPI_MISO_EX_PORT    GPIOA
#define PWR_SPI_MISO_EX_PIN     GPIO_Pin_6

#define PWR_SPI_MOSI_EX_PORT    GPIOA
#define PWR_SPI_MOSI_EX_PIN     GPIO_Pin_7

#define PWR_UART_TX_PORT        GPIOA
#define PWR_UART_TX_PIN         GPIO_Pin_9

#define PWR_UART_RX_PORT        GPIOA
#define PWR_UART_RX_PIN         GPIO_Pin_10

/* AF 复用功能号 (GPIO_PinAFConfig 使用) */
#define PWR_SPI1_AF             GPIO_AF_SPI1
#define PWR_USART1_AF           GPIO_AF_USART1

/*============================================================
    五、特殊保留引脚 (禁止 GPIO_Init)
============================================================*/
#define PWR_SWDIO_PORT      GPIOA
#define PWR_SWDIO_PIN       GPIO_Pin_13

#define PWR_SWCLK_PORT      GPIOA
#define PWR_SWCLK_PIN       GPIO_Pin_14

#define PWR_BOOT1_PORT      GPIOB
#define PWR_BOOT1_PIN       GPIO_Pin_1

#define OSC32_IN_PORT       GPIOC
#define OSC32_IN_PIN        GPIO_Pin_14

#define OSC32_OUT_PORT      GPIOC
#define OSC32_OUT_PIN       GPIO_Pin_15

/*============================================================
    六、DAC 通道组引脚 (GDA6641 x4, 位号 U14/U15/U16/U17 = CH0~CH3)
============================================================*/
/* DAC_CH0 (U14) */
#define DAC_CH0_DIN_PORT    GPIOI
#define DAC_CH0_DIN_PIN     GPIO_Pin_0

#define DAC_CH0_CLR_PORT    GPIOI
#define DAC_CH0_CLR_PIN     GPIO_Pin_1

#define DAC_CH0_POR_PORT    GPIOI
#define DAC_CH0_POR_PIN     GPIO_Pin_2

#define DAC_CH0_LDAC_PORT   GPIOI
#define DAC_CH0_LDAC_PIN    GPIO_Pin_3

#define DAC_CH0_SYNC_PORT   GPIOI
#define DAC_CH0_SYNC_PIN    GPIO_Pin_4

#define DAC_CH0_SCLK_PORT   GPIOI
#define DAC_CH0_SCLK_PIN    GPIO_Pin_5

#define DAC_CH0_SDO_PORT    GPIOI
#define DAC_CH0_SDO_PIN     GPIO_Pin_6

/* DAC_CH1 (U15) */
#define DAC_CH1_SCLK_PORT   GPIOG
#define DAC_CH1_SCLK_PIN    GPIO_Pin_9

#define DAC_CH1_SYNC_PORT   GPIOG
#define DAC_CH1_SYNC_PIN    GPIO_Pin_10

#define DAC_CH1_CLR_PORT    GPIOG
#define DAC_CH1_CLR_PIN     GPIO_Pin_11

#define DAC_CH1_POR_PORT    GPIOG
#define DAC_CH1_POR_PIN     GPIO_Pin_12

#define DAC_CH1_DIN_PORT    GPIOG
#define DAC_CH1_DIN_PIN     GPIO_Pin_13

#define DAC_CH1_LDAC_PORT   GPIOG
#define DAC_CH1_LDAC_PIN    GPIO_Pin_14

#define DAC_CH1_SDO_PORT    GPIOG
#define DAC_CH1_SDO_PIN     GPIO_Pin_15

/* DAC_CH2 (U16) */
#define DAC_CH2_SDO_PORT    GPIOG
#define DAC_CH2_SDO_PIN     GPIO_Pin_1

#define DAC_CH2_CLR_PORT    GPIOG
#define DAC_CH2_CLR_PIN     GPIO_Pin_2

#define DAC_CH2_SCLK_PORT   GPIOG
#define DAC_CH2_SCLK_PIN    GPIO_Pin_3

#define DAC_CH2_POR_PORT    GPIOG
#define DAC_CH2_POR_PIN     GPIO_Pin_4

#define DAC_CH2_DIN_PORT    GPIOG
#define DAC_CH2_DIN_PIN     GPIO_Pin_5

#define DAC_CH2_SYNC_PORT   GPIOG
#define DAC_CH2_SYNC_PIN    GPIO_Pin_6

#define DAC_CH2_LDAC_PORT   GPIOG
#define DAC_CH2_LDAC_PIN    GPIO_Pin_7

/* DAC_CH3 (U17) */
#define DAC_CH3_CLR_PORT    GPIOE
#define DAC_CH3_CLR_PIN     GPIO_Pin_0

#define DAC_CH3_DIN_PORT    GPIOE
#define DAC_CH3_DIN_PIN     GPIO_Pin_1

#define DAC_CH3_POR_PORT    GPIOE
#define DAC_CH3_POR_PIN     GPIO_Pin_2

#define DAC_CH3_LDAC_PORT   GPIOE
#define DAC_CH3_LDAC_PIN    GPIO_Pin_3

#define DAC_CH3_SDO_PORT    GPIOE
#define DAC_CH3_SDO_PIN     GPIO_Pin_4

#define DAC_CH3_SYNC_PORT   GPIOE
#define DAC_CH3_SYNC_PIN    GPIO_Pin_5

#define DAC_CH3_SCLK_PORT   GPIOE
#define DAC_CH3_SCLK_PIN    GPIO_Pin_6

/*============================================================
    七、ADC 通道组引脚 (LC1258 x4, 位号 U2/U5/U8/U11 = CH0~CH3)
============================================================*/
/* ADC_CH0 (U2, 电压监测) */
#define ADC_CH0_DIN_PORT    GPIOH
#define ADC_CH0_DIN_PIN     GPIO_Pin_8

#define ADC_CH0_SCLK_PORT   GPIOH
#define ADC_CH0_SCLK_PIN    GPIO_Pin_9

#define ADC_CH0_START_PORT  GPIOH
#define ADC_CH0_START_PIN   GPIO_Pin_10

#define ADC_CH0_DRDY_PORT   GPIOH
#define ADC_CH0_DRDY_PIN    GPIO_Pin_11

/* OUT 为 SPI DOUT 数据线, 按输入定义 (待确认 #3: 原理图标注"模拟, 跳过 GPIO") */
#define ADC_CH0_OUT_PORT    GPIOH
#define ADC_CH0_OUT_PIN     GPIO_Pin_12

#define ADC_CH0_RST_PORT    GPIOH
#define ADC_CH0_RST_PIN     GPIO_Pin_13

#define ADC_CH0_CS_PORT     GPIOH
#define ADC_CH0_CS_PIN      GPIO_Pin_14

/* ADC_CH1 (U5, 电流监测) */
#define ADC_CH1_CS_PORT     GPIOE
#define ADC_CH1_CS_PIN      GPIO_Pin_8

#define ADC_CH1_DRDY_PORT   GPIOE
#define ADC_CH1_DRDY_PIN    GPIO_Pin_9

#define ADC_CH1_START_PORT  GPIOE
#define ADC_CH1_START_PIN   GPIO_Pin_10

#define ADC_CH1_SCLK_PORT   GPIOE
#define ADC_CH1_SCLK_PIN    GPIO_Pin_11

#define ADC_CH1_DIN_PORT    GPIOE
#define ADC_CH1_DIN_PIN     GPIO_Pin_12

/* PE13 空置 (待确认 #8) */

/* OUT 为 SPI DOUT 数据线, 按输入定义 (待确认 #3: 原理图标注"模拟, 跳过 GPIO") */
#define ADC_CH1_OUT_PORT    GPIOE
#define ADC_CH1_OUT_PIN     GPIO_Pin_14

#define ADC_CH1_RST_PORT    GPIOE
#define ADC_CH1_RST_PIN     GPIO_Pin_15

/* ADC_CH2 (U8, 温度监测) */
#define ADC_CH2_SCLK_PORT   GPIOF
#define ADC_CH2_SCLK_PIN    GPIO_Pin_1

#define ADC_CH2_DIN_PORT    GPIOF
#define ADC_CH2_DIN_PIN     GPIO_Pin_2

#define ADC_CH2_DRDY_PORT   GPIOF
#define ADC_CH2_DRDY_PIN    GPIO_Pin_3

#define ADC_CH2_START_PORT  GPIOF
#define ADC_CH2_START_PIN   GPIO_Pin_4

#define ADC_CH2_RST_PORT    GPIOF
#define ADC_CH2_RST_PIN     GPIO_Pin_5

#define ADC_CH2_CS_PORT     GPIOF
#define ADC_CH2_CS_PIN      GPIO_Pin_6

/* OUT 为 SPI DOUT 数据线, 按输入定义 (待确认 #3: 原理图标注"模拟, 跳过 GPIO") */
#define ADC_CH2_OUT_PORT    GPIOF
#define ADC_CH2_OUT_PIN     GPIO_Pin_7

/* ADC_CH3 (U11, 电源/辅助监测) */
/* OUT 为 SPI DOUT 数据线, 按输入定义 (待确认 #3: 原理图标注"模拟, 跳过 GPIO") */
#define ADC_CH3_OUT_PORT    GPIOF
#define ADC_CH3_OUT_PIN     GPIO_Pin_8

#define ADC_CH3_DRDY_PORT   GPIOF
#define ADC_CH3_DRDY_PIN    GPIO_Pin_9

#define ADC_CH3_CS_PORT     GPIOF
#define ADC_CH3_CS_PIN      GPIO_Pin_10

#define ADC_CH3_SCLK_PORT   GPIOF
#define ADC_CH3_SCLK_PIN    GPIO_Pin_11

#define ADC_CH3_START_PORT  GPIOF
#define ADC_CH3_START_PIN   GPIO_Pin_12

/* PF13 空置 (待确认 #8) */

#define ADC_CH3_DIN_PORT    GPIOF
#define ADC_CH3_DIN_PIN     GPIO_Pin_14

#define ADC_CH3_RST_PORT    GPIOF
#define ADC_CH3_RST_PIN     GPIO_Pin_15

/*============================================================
    八、引脚句柄类型 (Dev 层驱动共用)
        Dev/gda6641.h 与 Dev/lc1258.h 的句柄结构体可从以下类型派生
============================================================*/
typedef struct {
    GPIO_TypeDef *port;
    u16           pin;
} gpio_pin_t;

/* GDA6641 单实例引脚集合 */
typedef struct {
    gpio_pin_t din;     /* 数据输入          OUT_PP */
    gpio_pin_t clr;     /* 清零              OUT_PP */
    gpio_pin_t por;     /* 上电复位          OUT_PP */
    gpio_pin_t ldac;    /* 加载 DAC 输出     OUT_PP */
    gpio_pin_t sync;    /* 帧同步            OUT_PP */
    gpio_pin_t sclk;    /* 串行时钟          OUT_PP */
    gpio_pin_t sdo;     /* 数据回读          IN */
} gda6641_pin_t;

/* LC1258 单实例引脚集合 */
typedef struct {
    gpio_pin_t din;     /* 配置数据输入      OUT_PP */
    gpio_pin_t sclk;    /* 串行时钟          OUT_PP */
    gpio_pin_t start;   /* 启动转换          OUT_PP */
    gpio_pin_t drdy;    /* 数据就绪          IN */
    gpio_pin_t out;     /* DOUT 数据输出     IN (待确认 #3) */
    gpio_pin_t rst;     /* 复位              OUT_PP */
    gpio_pin_t cs;      /* 片选              OUT_PP */
} lc1258_pin_t;

/* Dev 层驱动句柄类型: Task 6/7 的 Dev 头文件 (gda6641.h/lc1258.h) 直接复用
   以下 typedef 消费引脚表, 不再重复定义引脚结构,
   board_map.h 是 Dev 与 Bsp 的唯一耦合点 */
typedef gda6641_pin_t gda6641_handle_t;
typedef lc1258_pin_t  lc1258_handle_t;

/* 由 bsp_board.c 实例化 (Task 3) */
extern const gda6641_pin_t g_dac_pin_map[4];    /* 索引 = DAC_IDX_xxx */
extern const lc1258_pin_t g_adc_pin_map[4];     /* 索引 = ADC_IDX_xxx */

/*============================================================
    九、电源设备映射 (15 路)
============================================================*/
/* GDA6641 实例索引 (功能名 = 位号名) */
#define DAC_IDX_CH0     0   /* U14 */
#define DAC_IDX_CH1     1   /* U15 */
#define DAC_IDX_CH2     2   /* U16 */
#define DAC_IDX_CH3     3   /* U17 */

/* 位号别名 (兼容保留) */
#define DAC_IDX_U14     DAC_IDX_CH0
#define DAC_IDX_U15     DAC_IDX_CH1
#define DAC_IDX_U16     DAC_IDX_CH2
#define DAC_IDX_U17     DAC_IDX_CH3

/* GDA6641 输出通道 (VOUTA/B/C/D) */
#define DAC_CH_A        0
#define DAC_CH_B        1
#define DAC_CH_C        2
#define DAC_CH_D        3

/* LC1258 实例索引 (功能名 = 位号名) */
#define ADC_IDX_CH0     0   /* U2  电压监测 */
#define ADC_IDX_CH1     1   /* U5  电流监测 */
#define ADC_IDX_CH2     2   /* U8  温度监测 */
#define ADC_IDX_CH3     3   /* U11 电源/辅助监测 */

/* 功能别名 (兼容保留) */
#define ADC_IDX_V       ADC_IDX_CH0
#define ADC_IDX_I       ADC_IDX_CH1
#define ADC_IDX_T       ADC_IDX_CH2
#define ADC_IDX_AUX     ADC_IDX_CH3

/* dev_map_t.chip_type 取值 */
#define CHIP_TYPE_MAC5048   0
#define CHIP_TYPE_HQEF5016  1

/* 设备 ID 即外部协议 ID (0x01~0x0F) */
typedef enum {
    DEV_INVALID = 0,    /* 无效设备 (占位/未连接) */
    DEV_KF = 1, DEV_DTJ, DEV_DYGY, DEV_QGSJ, DEV_SFXJ1, DEV_SFXJ2,
    DEV_GSDJ, DEV_WAOXJ, DEV_HWXJ1, DEV_HWXJ2, DEV_HWXJ3,
    DEV_HJJC1, DEV_HJJC2, DEV_HJJC3, DEV_PD, DEV_NUM = 16
} dev_id_e;

/* 单路电源设备完整映射, g_dev_map[] 由 bsp_board.c 实例化:
   ID  名称       EN   故障信号               芯片          CLREF DAC    FAULT_AIN
   0x01 28V_KF    PD0  KF1=PB4 + KF2=PA15     MAC5048 x2    U17.VOUTB    0xFF
   0x02 28V_DTJ   PD14 PB15                   MAC5048       U14.VOUTD    0xFF
   0x03 12V_DYGY  PD9  GOK=PH6 / GOC=PH7      HQEF5016      U15.VOUTA    10
   0x04 28V_QGSJ  PD1  PB9                    MAC5048       U17.VOUTA    0xFF
   0x05 28V_SFXJ1 PD4  PB12                   MAC5048       U14.VOUTB    0xFF
   0x06 28V_SFXJ2 PD7  PB10                   MAC5048       U14.VOUTC    0xFF
   0x07 12V_GSDJ  PD12 GOK=PA2 / GOC=PA3      HQEF5016      U15.VOUTC    11
   0x08 28V_WAOXJ PD8  PB13                   MAC5048       U16.VOUTD    0xFF
   0x09 28V_HWXJ1 PD13 PB8                    MAC5048       U15.VOUTB    0xFF
   0x0A 28V_HWXJ2 PD2  PB7                    MAC5048       U17.VOUTC    0xFF
   0x0B 28V_HWXJ3 PD6  PB0                    MAC5048       U16.VOUTB    0xFF
   0x0C 28V_HJJC1 PD3  PB5                    MAC5048       U16.VOUTC    0xFF
   0x0D 28V_HJJC2 PD10 PB6                    MAC5048       U15.VOUTD    0xFF
   0x0E 28V_HJJC3 PD11 PB14                   MAC5048       U14.VOUTA    0xFF
   0x0F 28V_PD    PD5  PB3                    MAC5048       U16.VOUTA    0xFF

   注: HQEF5016 (DYGY/GSDJ) 无数字 FAULT 引脚, FAULT 为模拟量,
   由 ADC_CH3 (U11) AIN10(DYGY)/AIN11(GSDJ) 采集;
   其 fault_port[] 填数字状态引脚 GOK/GOC (fault_cnt=2,
   fault_port[0]=GOK, fault_port[1]=GOC) */
typedef struct {
    dev_id_e      id;
    u8            name[12];
    GPIO_TypeDef *en_port;
    u16           en_pin;
    u8            fault_cnt;                      /* 1 或 2; HQEF5016 时=2 (GOK/GOC) */
    GPIO_TypeDef *fault_port[2];                  /* HQEF5016: [0]=GOK, [1]=GOC */
    u16           fault_pin[2];
    u8            dac_idx;                        /* GDA6641 实例 (DAC_IDX_xxx, 0~3) */
    u8            dac_ch;                         /* GDA6641 通道 (DAC_CH_x, 0~3) */
    u8            adc_ain_v;                      /* ADC_CH0 (U2) 电压 AIN */
    u8            adc_ain_i;                      /* ADC_CH1 (U5) 电流 AIN */
    u8            adc_ain_t;                      /* ADC_CH2 (U8) 温度 AIN (KF 用子表, 此字段填 0xFF) */
    u8            adc_ain_fault;                  /* ADC_CH3 (U11) FAULT 模拟量 AIN (仅 HQEF5016, 0xFF=无) */
    u8            chip_type;                      /* CHIP_TYPE_xxx */
} dev_map_t;

/* 温度通道子表 (g_t_map, ADC_CH2/U8): AIN -> (设备, 子通道), KF 拆分 KF1/KF2
   AIN: 0=WAOXJ 1=DYGY 2=SFXJ1 3=HJJC3 4=SFXJ2 5=DTJ 6=KF2 7=HWXJ3
        8=QGSJ 9=HJJC1 10=HWXJ1 11=HWXJ2 12=HJJC2 13=PD 14=GSDJ 15=KF1 */
typedef struct {
    dev_id_e dev;
    u8       sub;        /* 0=主通道 1=KF1 2=KF2 */
} t_map_t;

/* 由 bsp_board.c 实例化 (Task 3)
   索引约定: 索引 = 设备 ID, g_dev_map[DEV_KF]..g_dev_map[DEV_PD] 按 ID 直索引,
   无需 -1 换算; g_dev_map[0] 为 DEV_INVALID 占位槽 (全 0 填充), 与 dev_id_e 一一对应 */
extern const t_map_t  g_t_map[16];
extern const dev_map_t g_dev_map[DEV_NUM];

#endif /* __BOARD_MAP_H_ */
