/*
***********************************************************************************************************************
    @brief          : 板载spi初始化
    @author         : xiongjinqi
    @date           : 2024/07/21
***********************************************************************************************************************
*/
#ifndef __SPI_H_
#define __SPI_H_

#include "types_def.h"
#include "stm32f4xx.h"

#define SPIx                           SPI1
#define SPIx_CLK                       RCC_APB1Periph_SPI2
#define SPIx_CLK_INIT                  RCC_APB1PeriphClockCmd
#define SPIx_IRQn                      SPI1_IRQn
#define SPIx_IRQHANDLER                SPI1_IRQHandler

#define SPIx_SCK_PIN                   GPIO_Pin_3
#define SPIx_SCK_GPIO_PORT             GPIOB
#define SPIx_SCK_GPIO_CLK              RCC_AHB1Periph_GPIOA
#define SPIx_SCK_SOURCE                GPIO_PinSource1
#define SPIx_SCK_AF                    GPIO_AF_SPI1

#define SPIx_MISO_PIN                  GPIO_Pin_2
#define SPIx_MISO_GPIO_PORT            GPIOI
#define SPIx_MISO_GPIO_CLK             RCC_AHB1Periph_GPIOI
#define SPIx_MISO_SOURCE               GPIO_PinSource2
#define SPIx_MISO_AF                   GPIO_AF_SPI2

#define SPIx_MOSI_PIN                  GPIO_Pin_3
#define SPIx_MOSI_GPIO_PORT            GPIOI
#define SPIx_MOSI_GPIO_CLK             RCC_AHB1Periph_GPIOI
#define SPIx_MOSI_SOURCE               GPIO_PinSource3
#define SPIx_MOSI_AF                   GPIO_AF_SPI2

//以下是SPI模块的初始化代码，配置成主机模式
//SPI口初始化
//这里针是对SPI1的初始化
void SPI1_Init(void);

void SPI1_DMA_Init(u32 *txbuff, u16 txlen);
void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler);

#endif /* __SPI_H_ */

