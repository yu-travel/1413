#include "stm32f4xx.h"
#include "bsp_spi.h"
#include "board_map.h"

/*
    @brief      : SPI1 主机 (FPGA 通信) GPIO 初始化与 SPI 外设配置
    @note       : PA5/PA6/PA7 = AF5 (SPI1), PA4 = GPIO 推挽输出作软件片选
                  (PA4 不配置 AF, 避免硬件 NSS 干扰)
                  bsp_board.c 故意不初始化本组引脚, 由本文件全权负责
*/

/*
    @brief      : SPI1 引脚 GPIO + AF 配置
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
static void bsp_spi_gpio_init(void)
{
    GPIO_InitTypeDef gpio_init;

    /* 时钟: GPIOA + SPI1 (APB2) + SYSCFG (GPIO_PinAFConfig 需要) */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* SCK(PA5)/MOSI(PA7): 复用推挽, 无上下拉 */
    gpio_init.GPIO_Pin = PWR_SPI_CLK_EX_PIN | PWR_SPI_MOSI_EX_PIN;
    gpio_init.GPIO_Mode = GPIO_Mode_AF;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(PWR_SPI_CLK_EX_PORT, &gpio_init);

    /* MISO(PA6): 复用推挽, 上拉 (FPGA 驱动, 上拉防浮空且无害) */
    gpio_init.GPIO_Pin = PWR_SPI_MISO_EX_PIN;
    gpio_init.GPIO_Mode = GPIO_Mode_AF;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(PWR_SPI_MISO_EX_PORT, &gpio_init);

    /* AF5 复用选择: PA5/PA6/PA7 -> SPI1 (PA4 片选为 GPIO, 不配 AF) */
    GPIO_PinAFConfig(PWR_SPI_CLK_EX_PORT, GPIO_PinSource5, PWR_SPI1_AF);
    GPIO_PinAFConfig(PWR_SPI_MISO_EX_PORT, GPIO_PinSource6, PWR_SPI1_AF);
    GPIO_PinAFConfig(PWR_SPI_MOSI_EX_PORT, GPIO_PinSource7, PWR_SPI1_AF);

    /* CS(PA4): 推挽输出, 初始高电平 (空闲释放 FPGA) */
    gpio_init.GPIO_Pin = PWR_SPI_NSS_EX_PIN;
    gpio_init.GPIO_Mode = GPIO_Mode_OUT;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(PWR_SPI_NSS_EX_PORT, &gpio_init);
    GPIO_SetBits(PWR_SPI_NSS_EX_PORT, PWR_SPI_NSS_EX_PIN);
}

/*
    @brief      : SPI1 外设配置 (主模式, Mode 0, 8bit, MSB, 软件NSS)
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
static void bsp_spi_cfg_init(void)
{
    SPI_InitTypeDef spi_init;

    spi_init.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spi_init.SPI_Mode = SPI_Mode_Master;
    spi_init.SPI_DataSize = SPI_DataSize_8b;
    spi_init.SPI_CPOL = SPI_CPOL_Low;               /* Mode 0 */
    spi_init.SPI_CPHA = SPI_CPHA_1Edge;             /* 第1边沿采样 */
    spi_init.SPI_NSS = SPI_NSS_Soft;                /* SSM 置位 */
    spi_init.SPI_BaudRatePrescaler = BSP_SPI_PRESCALER;
    spi_init.SPI_FirstBit = SPI_FirstBit_MSB;
    spi_init.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &spi_init);

    /* 软件 NSS 模式下 SSI 需显式置位, 否则主机模式判忙卡死 */
    SPI_NSSInternalSoftwareConfig(SPI1, SPI_NSSInternalSoft_Set);

    SPI_Cmd(SPI1, ENABLE);
}

/*
    @brief      : SPI1 初始化 (GPIO + 外设)
    @param[in]  : none
    @param[out] : none
    @retval     : none
    @note       : 尚未在 main 中调用 (Task 12 集成), 未调用不产生副作用
*/
void bsp_spi_init(void)
{
    bsp_spi_gpio_init();
    bsp_spi_cfg_init();
}

/*
    @brief      : 软件片选控制
    @param[in]  : level - BSP_SPI_CS_LOW(0)选中 / BSP_SPI_CS_HIGH(1)释放
    @param[out] : none
    @retval     : none
*/
void bsp_spi_cs(u8 level)
{
    if (level == BSP_SPI_CS_LOW) {
        GPIO_ResetBits(PWR_SPI_NSS_EX_PORT, PWR_SPI_NSS_EX_PIN);
    } else {
        GPIO_SetBits(PWR_SPI_NSS_EX_PORT, PWR_SPI_NSS_EX_PIN);
    }
}

/*
    @brief      : SPI1 全双工收发 1 字节
    @param[in]  : data - 发送字节
    @param[out] : none
    @retval     : MISO 收到的字节
*/
u8 bsp_spi_write_byte(u8 data)
{
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) {
        ;
    }
    SPI_I2S_SendData(SPI1, data);
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) {
        ;
    }
    return (u8)SPI_I2S_ReceiveData(SPI1);
}

/*
    @brief      : SPI1 块传输 (逐字节全双工)
    @param[in]  : tx  - 发送缓冲区 (必须非空)
                  len - 传输字节数
    @param[out] : rx  - 接收缓冲区 (可为 NULL, 丢弃接收数据)
    @retval     : none
    @note       : 片选由调用方通过 bsp_spi_cs 管理
*/
void bsp_spi_transfer(u8 *tx, u8 *rx, u16 len)
{
    u16 i;

    if (tx == NULL || len == 0) {
        return;
    }

    for (i = 0; i < len; i++) {
        u8 data = bsp_spi_write_byte(tx[i]);
        if (rx != NULL) {
            rx[i] = data;
        }
    }
}
