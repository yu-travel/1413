#ifndef __BSP_FLASH_H_
#define __BSP_FLASH_H_

#include "types_def.h"

#define CAL_FLASH_ADDR   0x080E0000u   /* 校准数据扇区 (1MB Flash 的 Sector11) */
#define CAL_FLASH_MAGIC  0x14130141u   /* 校准数据魔数 */
#define CAL_NUM          45             /* 15路 × V/I/T, 待确认#7 */

/*
    @brief      : 读校准数据
    @note       : 校验魔数, 成功返回1; 校验失败返回0并填充默认 k=1, b=0
                  布局: [u32 魔数][n × float k][n × float b]
    @param[out] : k  斜率数组 (长度n)
    @param[out] : b  截距数组 (长度n)
    @param[in]  : n  组数 (0 < n <= CAL_NUM)
    @retval     : 1 成功, 0 失败
*/
u8 bsp_flash_cal_read(float *k, float *b, u8 n);

/*
    @brief      : 写校准数据 (擦除扇区 + 编程)
    @param[in]  : k  斜率数组 (长度n)
    @param[in]  : b  截距数组 (长度n)
    @param[in]  : n  组数 (0 < n <= CAL_NUM)
    @retval     : 1 成功, 0 失败
*/
u8 bsp_flash_cal_write(const float *k, const float *b, u8 n);

#endif /* __BSP_FLASH_H_ */
