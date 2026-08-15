#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

// 自定义函数：将32位无符号整数转为二进制字符串
void uint32_to_bin(unsigned int val, char *buf, int len) {
    if (len < 33) return; // 确保缓冲区足够（32位+结束符）
    buf[32] = '\0';       // 字符串结束符
    for (int i = 31; i >= 0; i--) {
        buf[i] = (val & 1) ? '1' : '0'; // 从最低位到最高位逐位判断
        val >>= 1;                      // 右移一位
    }
}

// 腾锐D2000 引脚复用配置寄存器基地址（手册5.11.1节）
#define PINMUX_BASE    0x28040000
// GPIO1_B4（SD_DAT1）对应的复用寄存器偏移（参考手册，通常为0x10）
#define PINMUX_OFFSET  0x10
// 映射内存大小（1页=4096字节，足够覆盖寄存器）
#define MAP_SIZE       4096UL

int main() {
    int fd;
    void *map_base, *virt_addr;
    char bin_buf[33]; // 存储32位二进制字符串

    // 1. 打开物理内存设备（必须root权限）
    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        perror("打开/dev/mem失败（需sudo运行）");
        return -1;
    }

    // 2. 映射物理地址到虚拟地址
    map_base = mmap(
        NULL,                    // 系统自动分配虚拟地址
        MAP_SIZE,                // 映射大小
        PROT_READ | PROT_WRITE,  // 可读可写
        MAP_SHARED,              // 共享映射（同步到物理内存）
        fd,                      // /dev/mem文件描述符
        PINMUX_BASE              // 要映射的物理基地址
    );

    if (map_base == (void *)-1) {
        perror("mmap映射失败");
        close(fd);
        return -1;
    }

    // 3. 计算目标寄存器的虚拟地址（基地址+偏移）
    virt_addr = map_base + PINMUX_OFFSET;
    // 读取寄存器值（32位无符号整数）
    unsigned int reg_val = *(volatile unsigned int *)virt_addr;

    // 4. 转换为二进制字符串
    uint32_to_bin(reg_val, bin_buf, sizeof(bin_buf));

    // 5. 输出结果并解读（修复格式符警告）
    printf("=== GPIO1_B4 复用配置寄存器 ===\n");
    printf("物理地址: 0x%lx (0x%lx + 0x%x)\n", 
           (unsigned long)PINMUX_BASE + PINMUX_OFFSET, 
           (unsigned long)PINMUX_BASE, PINMUX_OFFSET);
    printf("寄存器值: 0x%x (二进制: %s)\n", reg_val, bin_buf); // 用自定义二进制字符串
    
    // 复用功能解读（低4位控制功能，参考腾锐D2000手册）
    unsigned int func_val = reg_val & 0x0F; // 取低4位
    printf("功能值（低4位）: 0x%x → ", func_val);
    if (func_val == 0x0) {
        printf("GPIO 模式（复用切换成功！）\n");
    } else if (func_val == 0x1) {
        printf("SD_DAT1 模式（未切换，仍为默认SD功能）\n");
    } else {
        printf("其他复用功能（需查手册确认）\n");
    }

    // 6. 释放资源
    munmap(map_base, MAP_SIZE);
    close(fd);
    return 0;
}
