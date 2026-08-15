#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define GPIO1_BASE          0x28005000
#define GPIO_SWPORTB_DR     0x0C
#define GPIO_SWPORTB_DDR    0x10
#define GPIO_EXT_PORTB      0x14
#define GPIO1_B4_BIT        (1 << 4)
#define MAP_SIZE            4096

void *gpio_virt = NULL;
int fd_mem = -1;

void sig_exit(int sig) {
    if (gpio_virt != NULL) munmap(gpio_virt, MAP_SIZE);
    if (fd_mem >= 0) close(fd_mem);
    exit(0);
}

int main(void) {
    signal(SIGINT, sig_exit);

    fd_mem = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd_mem < 0) { perror("open /dev/mem"); return -1; }

    gpio_virt = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, GPIO1_BASE);
    if (gpio_virt == MAP_FAILED) { perror("mmap"); close(fd_mem); return -1; }

    // 读取初始寄存器值（关键验证）
    volatile unsigned int *ddr = (volatile unsigned int *)(gpio_virt + GPIO_SWPORTB_DDR);
    volatile unsigned int *dr  = (volatile unsigned int *)(gpio_virt + GPIO_SWPORTB_DR);
    volatile unsigned int *ext = (volatile unsigned int *)(gpio_virt + GPIO_EXT_PORTB);

    printf("初始状态：\n");
    printf("  DDR(B组方向寄存器)：0x%08X (bit4=%d)\n", *ddr, (*ddr & GPIO1_B4_BIT) ? 1 : 0);
    printf("  DR(B组输出寄存器)：0x%08X (bit4=%d)\n", *dr, (*dr & GPIO1_B4_BIT) ? 1 : 0);
    printf("  EXT(B组输入寄存器)：0x%08X (bit4=%d)\n", *ext, (*ext & GPIO1_B4_BIT) ? 1 : 0);

    // 配置输出 + 拉低
//    *ddr |= GPIO1_B4_BIT;
 //   *dr &= ~GPIO1_B4_BIT;
    usleep(1000); // 等待硬件同步

    // 读取配置后的值
    printf("\n配置后状态：\n");
    printf("  DDR(B组方向寄存器)：0x%08X (bit4=%d) [1=输出]\n", *ddr, (*ddr & GPIO1_B4_BIT) ? 1 : 0);
    printf("  DR(B组输出寄存器)：0x%08X (bit4=%d) [0=低电平]\n", *dr, (*dr & GPIO1_B4_BIT) ? 1 : 0);
    printf("  EXT(B组输入寄存器)：0x%08X (bit4=%d) [实际硬件电平]\n", *ext, (*ext & GPIO1_B4_BIT) ? 1 : 0);

    printf("\nGPIO1_B4 已配置为输出+低电平（按Ctrl+C退出）\n");
    while (1) { sleep(1); }

    return 0;
}
