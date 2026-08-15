#include "mac5048.h"
#include <stddef.h>
#include "stm32f4xx_syscfg.h"


void MAC5048_Fault_GPIO_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    /* 使能GPIOA GPIOB时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE);

    // 完全关闭 SWD + JTAG，全部释放PA13 PA14 PA15为普通GPIO
    // ?警告：执行之后，仿真器再也不能连接芯片，只能靠烧录器重新下载程序！
    // ===== 直接寄存器关闭JTAG，释放PA15，保留SWD，无库函数、无未知宏 =====
    SYSCFG->MAPR |= SYSCFG_MAPR_SWJ_CFG_JTAGDISABLE;
	
    /************ KF2_FAULT PA15 ************/
    GPIO_InitStructure.GPIO_Pin     = KF2_FAULT_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
    GPIO_Init(KF2_FAULT_GPIO_PORT, &GPIO_InitStructure);

    /************ GPIOB 批量FAULT引脚 ************/
    GPIO_InitStructure.GPIO_Pin  = HWXJ3_FAULT_GPIO_PIN
                                 | PD_FAULT_GPIO_PIN
                                 | KF1_FAULT_GPIO_PIN
                                 | HJJC1_FAULT_GPIO_PIN
                                 | HJJC2_FAULT_GPIO_PIN
                                 | HWXJ2_FAULT_GPIO_PIN
                                 | HWXJ1_FAULT_GPIO_PIN
                                 | QGSJ_FAULT_GPIO_PIN
                                 | SFXJ2_FAULT_GPIO_PIN
                                 | SFXJ1_FAULT_GPIO_PIN
                                 | WAOXJ_FAULT_GPIO_PIN
                                 | HJJC3_FAULT_GPIO_PIN
                                 | DTJ_FAULT_GPIO_PIN;

    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
    GPIO_Init(HWXJ3_FAULT_GPIO_PORT, &GPIO_InitStructure);
}


typedef struct
{
    GPIO_TypeDef* port;
    uint16_t pin;
    uint8_t *p_sta;
}mac5048_fault_t;

void MAC5048_ReadAllFault(_board_monitor_t *sysmon)
{
    if(sysmon == NULL) return;

    sysmon->kf2_fault    = (READ_KF2_FAULT())    ? 0U : 1U;
    sysmon->hwxj3_fault  = (READ_HWXJ3_FAULT())  ? 0U : 1U;
    sysmon->pd_fault     = (READ_PD_FAULT())     ? 0U : 1U;
    sysmon->kf1_fault    = (READ_KF1_FAULT())    ? 0U : 1U;
    sysmon->hjjc1_fault  = (READ_HJJC1_FAULT())  ? 0U : 1U;
    sysmon->hjjc2_fault  = (READ_HJJC2_FAULT())  ? 0U : 1U;
    sysmon->hwxj2_fault  = (READ_HWXJ2_FAULT())  ? 0U : 1U;
    sysmon->hwxj1_fault  = (READ_HWXJ1_FAULT())  ? 0U : 1U;
    sysmon->qgsj_fault   = (READ_QGSJ_FAULT())   ? 0U : 1U;
    sysmon->sfxj2_fault  = (READ_SFXJ2_FAULT())  ? 0U : 1U;
    sysmon->sfxj1_fault  = (READ_SFXJ1_FAULT())  ? 0U : 1U;
    sysmon->waoxj_fault  = (READ_WAOXJ_FAULT())  ? 0U : 1U;
    sysmon->hjjc3_fault  = (READ_HJJC3_FAULT())  ? 0U : 1U;
    sysmon->dtj_fault    = (READ_DTJ_FAULT())    ? 0U : 1U;
}

void sysmon_data_get(_board_monitor_t *sysmon)
{
    //...
    MAC5048_ReadAllFault(sysmon);
}


