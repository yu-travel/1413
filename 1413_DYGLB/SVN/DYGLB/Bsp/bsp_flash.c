/*
***********************************************************************************************************************
    @brief          : 校准数据 Flash 读写 (Sector11 @ 0x080E0000, 128KB)
                     布局: [u32 魔数][n × float k][n × float b]
***********************************************************************************************************************
*/
#include "bsp_flash.h"
#include "stm32f4xx_flash.h"

/* float <-> u32 按位转换 */
typedef union {
    float   f;
    u32     w;
} float_word_t;

/*
    @brief      : 读校准数据
    @note       : 校验魔数, 失败填充默认 k=1, b=0 并返回0
*/
u8 bsp_flash_cal_read(float *k, float *b, u8 n)
{
    const float *pk;
    const float *pb;
    u32 magic;
    u8 i;

    if (k == NULL || b == NULL || n == 0 || n > CAL_NUM) {
        return 0;
    }

    magic = *(volatile u32 *)CAL_FLASH_ADDR;
    if (magic != CAL_FLASH_MAGIC) {
        /* 校准区无效: 填充默认 k=1, b=0 */
        for (i = 0; i < n; i++) {
            k[i] = 1.0f;
            b[i] = 0.0f;
        }
        return 0;
    }

    pk = (const float *)(CAL_FLASH_ADDR + 4);
    pb = (const float *)(CAL_FLASH_ADDR + 4 + (u32)n * 4);

    for (i = 0; i < n; i++) {
        k[i] = pk[i];
        b[i] = pb[i];
    }
    return 1;
}

/*
    @brief      : 写校准数据 (擦除整个 Sector11 后编程)
    @retval     : 1 成功, 0 失败
*/
u8 bsp_flash_cal_write(const float *k, const float *b, u8 n)
{
    FLASH_Status status;
    float_word_t fw;
    u32 addr;
    u8 i;

    if (k == NULL || b == NULL || n == 0 || n > CAL_NUM) {
        return 0;
    }

    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                    FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    status = FLASH_EraseSector(FLASH_Sector_11, VoltageRange_3);
    if (status != FLASH_COMPLETE) {
        FLASH_Lock();
        return 0;
    }

    status = FLASH_ProgramWord(CAL_FLASH_ADDR, CAL_FLASH_MAGIC);
    if (status != FLASH_COMPLETE) {
        FLASH_Lock();
        return 0;
    }

    addr = CAL_FLASH_ADDR + 4;
    for (i = 0; i < n; i++) {
        fw.f = k[i];
        status = FLASH_ProgramWord(addr, fw.w);
        if (status != FLASH_COMPLETE) {
            FLASH_Lock();
            return 0;
        }
        addr += 4;
    }
    for (i = 0; i < n; i++) {
        fw.f = b[i];
        status = FLASH_ProgramWord(addr, fw.w);
        if (status != FLASH_COMPLETE) {
            FLASH_Lock();
            return 0;
        }
        addr += 4;
    }

    FLASH_Lock();
    return 1;
}
