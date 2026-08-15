#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

// 严格按你给的飞腾腾锐D2000手册
#define GPIO1_BASE          0x28005000
#define GPIO_SWPORTB_DR     0x0C    // B组输出数据寄存器
#define GPIO_SWPORTB_DDR    0x10    // B组方向寄存器
#define GPIO1_B4_BIT        (1 << 4)
#define MAP_SIZE            4096

void *gpio_virt = NULL;
int fd_mem = -1;

void sig_exit(int sig) {
    printf("\n退出，释放资源...\n");
    if (gpio_virt != NULL && gpio_virt != MAP_FAILED)
        munmap(gpio_virt, MAP_SIZE);
    if (fd_mem >= 0)
        close(fd_mem);
    exit(0);
}

int main(void) {
    signal(SIGINT, sig_exit);

    fd_mem = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd_mem < 0) {
        perror("open /dev/mem 失败");
        return -1;
    }

    gpio_virt = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, GPIO1_BASE);
    if (gpio_virt == MAP_FAILED) {
        perror("mmap 失败");
        close(fd_mem);
        return -1;
    }

    // 1. 设置为输出模式
    volatile unsigned int *ddr = (volatile unsigned int *)(gpio_virt + GPIO_SWPORTB_DDR);
    *ddr |= GPIO1_B4_BIT;
    printf("GPIO1_B4 已设为输出\n");

    // 2. 直接拉低（核心）
    volatile unsigned int *dr  = (volatile unsigned int *)(gpio_virt + GPIO_SWPORTB_DR);
    *dr &= GPIO1_B4_BIT;
    printf("GPIO1_B4 已拉低 → 保持低电平\n");

    // 保持程序运行，电平就一直是低
    while (1) {
        sleep(1);
    }

    return 0;
}
