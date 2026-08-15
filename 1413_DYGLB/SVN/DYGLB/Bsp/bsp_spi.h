#ifndef __BSP_SPI_H_
#define __BSP_SPI_H_

#include "types_def.h"
#include "stm32f4xx.h"

/*
    @brief      : SPI1 FPGA 主机通信接口 (电源控制板 <-> FPGA)
                  - 引脚: CS=PA4(GPIO输出) SCK=PA5 MISO=PA6 MOSI=PA7 (AF5)
                  - 模式: 主模式, Mode 0 (CPOL=0/CPHA=0), 8bit, MSB, 软件NSS
                  - 速率: 10.5MHz (APB2 84MHz / 8)
    @note       : 引脚宏定义见 board_map.h 第四节 (外设复用 AF 组)
*/

/* 待确认 #4: FPGA 目标速率 10MHz, APB2=84MHz 无法整除,
   取 /8 = 10.5MHz (最接近且不超过); 若需调整仅改此宏一行 */
#define BSP_SPI_PRESCALER   SPI_BaudRatePrescaler_8

#define BSP_SPI_CS_LOW      0   /* 片选低: 选中 FPGA */
#define BSP_SPI_CS_HIGH     1   /* 片选高: 释放 FPGA */

void bsp_spi_init(void);
void bsp_spi_cs(u8 level);
u8   bsp_spi_write_byte(u8 data);
void bsp_spi_transfer(const u8 *tx, u8 *rx, u16 len);

#endif /* __BSP_SPI_H_ */
