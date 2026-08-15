#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

// 物理内存映射相关定义
#define PHYS_MEM_BASE 0x28005000  // GPIO1基地址
#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

int main() {
    int fd;
    void *map_base, *virt_addr;
    
    // 打开物理内存设备
    if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
        perror("open /dev/mem failed");
        return -1;
    }

    // 映射物理地址到虚拟地址
    map_base = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, PHYS_MEM_BASE & ~MAP_MASK);
    if (map_base == (void *) -1) {
        perror("mmap failed");
        close(fd);
        return -1;
    }

    // 1. 配置方向寄存器（偏移0x00）：GPIO1_B4设为输出
    virt_addr = map_base + (PHYS_MEM_BASE & MAP_MASK) + 0x00;
    printf("方向寄存器原数值: 0x%x\n", *(unsigned int *)virt_addr);
    *(unsigned int *)virt_addr = 0x10;  // bit4=1 → 输出模式
    printf("方向寄存器新数值: 0x%x\n", *(unsigned int *)virt_addr);

    // 2. 配置输出电平寄存器（偏移0x04）：GPIO1_B4设为高电平
    virt_addr = map_base + (PHYS_MEM_BASE & MAP_MASK) + 0x04;
    printf("电平寄存器原数值: 0x%x\n", *(unsigned int *)virt_addr);
    *(unsigned int *)virt_addr = 0x10;  // bit4=1 → 高电平
    printf("电平寄存器新数值: 0x%x\n", *(unsigned int *)virt_addr);

    // 释放映射、关闭文件
    munmap(map_base, MAP_SIZE);
    close(fd);
    return 0;
}
