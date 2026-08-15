#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define REG_BASE_PHYS   0x28180000
#define REG_OFFSET      0x0200
#define MAP_SIZE        0x1000

int main(void)
{
    int fd;
    void *map_base;
    volatile uint32_t *reg;
    uint32_t val;

    // 打开 /dev/mem
    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("open /dev/mem");
        return -1;
    }

    // 映射物理地址
    map_base = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, REG_BASE_PHYS);
    if (map_base == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return -1;
    }

    // 定位到 0x28180200
    reg = (volatile uint32_t *)((char *)map_base + REG_OFFSET);

    // 读取当前寄存器值
    val = *reg;
    printf("Before: 0x%08x\n", val);

    /*
     * 配置：
     * [31:30] all_pll_lock_pad pull-up/down，保持原值
     * [29:28] all_pll_lock_pad mux = Func2 (10b)
     * [27:26] cru_clk_obv_pad pull-up/down，保持原值
     * [25:24] cru_clk_obv_pad mux = Func2 (10b)
     */
    val &= ~((0x3 << 28) | (0x3 << 24));   // 清除复用位
    val |=  ((0x2 << 28) | (0x2 << 24));   // 设置 Func2

    *reg = val;

    // 读取验证
    val = *reg;
    printf("After : 0x%08x\n", val);

    munmap(map_base, MAP_SIZE);
    close(fd);

    return 0;
}