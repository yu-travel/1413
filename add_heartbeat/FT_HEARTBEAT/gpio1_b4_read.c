// 创建 gpio1_b4_read.c
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#define PHYS_MEM_BASE 0x28005000
#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

int main() {
    int fd;
    void *map_base, *virt_addr;

    if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
        perror("open /dev/mem failed");
        return -1;
    }

    map_base = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, PHYS_MEM_BASE & ~MAP_MASK);
    if (map_base == (void *) -1) {
        perror("mmap failed");
        close(fd);
        return -1;
    }

    // 读取方向寄存器
    virt_addr = map_base + (PHYS_MEM_BASE & MAP_MASK) + 0x00;
    printf("GPIO1_B4 方向: 0x%x (0x10=输出，0x00=输入)\n", *(unsigned int *)virt_addr);

    // 读取电平寄存器
    virt_addr = map_base + (PHYS_MEM_BASE & MAP_MASK) + 0x04;
    printf("GPIO1_B4 电平: 0x%x (0x10=高电平，0x00=低电平)\n", *(unsigned int *)virt_addr);

    munmap(map_base, MAP_SIZE);
    close(fd);
    return 0;
}
