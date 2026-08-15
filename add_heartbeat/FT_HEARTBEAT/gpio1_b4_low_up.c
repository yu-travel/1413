#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

// 严格匹配腾锐D2000手册
#define GPIO1_BASE          0x28005000
#define GPIO_SWPORTB_DR     0x0C    // B组输出数据寄存器
#define GPIO_SWPORTB_DDR    0x10    // B组方向寄存器
#define GPIO_EXT_PORTB      0x14    // B组输入寄存器（实际硬件电平）
#define GPIO1_B4_BIT        (1 << 4)// B4对应bit4
#define MAP_SIZE            4096
#define SWITCH_INTERVAL     5       // 切换间隔：5秒

void *gpio_virt = NULL;
int fd_mem = -1;

// 安全退出资源释放
void sig_exit(int sig) {
    printf("\n\n=== 开始释放资源 ===");
    if (gpio_virt != NULL && gpio_virt != MAP_FAILED) {
        munmap(gpio_virt, MAP_SIZE);
        printf("\n✓ 内存映射已释放");
    }
    if (fd_mem >= 0) {
        close(fd_mem);
        printf("\n✓ /dev/mem已关闭");
    }
    printf("\n程序已退出\n");
    exit(0);
}

// 打印寄存器状态
void print_reg_status(const char *step, int target_level) {
    volatile unsigned int *ddr = (volatile unsigned int *)(gpio_virt + GPIO_SWPORTB_DDR);
    volatile unsigned int *dr  = (volatile unsigned int *)(gpio_virt + GPIO_SWPORTB_DR);
    volatile unsigned int *ext = (volatile unsigned int *)(gpio_virt + GPIO_EXT_PORTB);

    printf("\n=== %s（目标电平：%d）===\n", step, target_level);
    printf("  方向寄存器(DDR)：0x%08X → bit4=%d (1=输出模式)\n", 
           *ddr, (*ddr & GPIO1_B4_BIT) ? 1 : 0);
    printf("  输出寄存器(DR) ：0x%08X → bit4=%d (软件值，可写)\n", 
           *dr, (*dr & GPIO1_B4_BIT) ? 1 : 0);
    printf("  硬件电平(EXT) ：0x%08X → bit4=%d (硬件值，锁定)\n", 
           *ext, (*ext & GPIO1_B4_BIT) ? 1 : 0);
}

int main(void) {
    // 注册Ctrl+C退出信号
    signal(SIGINT, sig_exit);
    printf("=== 腾锐D2000 GPIO1_B4 5秒电平循环测试 ===\n");
    printf("流程：初始化 → 写0(%d秒) → 写1(%d秒) → 循环切换0/1（每%d秒一次）\n\n",
           SWITCH_INTERVAL, SWITCH_INTERVAL, SWITCH_INTERVAL);

    // 1. 打开物理内存（必须root）
    fd_mem = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd_mem < 0) {
        fprintf(stderr, "✗ 打开/dev/mem失败：%s\n", strerror(errno));
        fprintf(stderr, "  请用sudo运行！\n");
        return -1;
    }

    // 2. 映射GPIO1物理地址到用户空间
    gpio_virt = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, GPIO1_BASE);
    if (gpio_virt == MAP_FAILED) {
        fprintf(stderr, "✗ 内存映射失败：%s\n", strerror(errno));
        close(fd_mem);
        return -1;
    }
    printf("✓ GPIO1物理地址映射成功（虚拟地址：%p）\n", gpio_virt);

    // 3. 初始化：设置为输出模式
    volatile unsigned int *ddr = (volatile unsigned int *)(gpio_virt + GPIO_SWPORTB_DDR);
    volatile unsigned int *dr  = (volatile unsigned int *)(gpio_virt + GPIO_SWPORTB_DR);
    *ddr |= GPIO1_B4_BIT; // bit4置1 → 输出模式
    print_reg_status("初始化完成", -1);

    // 4. 初始写0（拉低），保持5秒
    printf("\n>>> 写入0（拉低），保持%d秒...\n", SWITCH_INTERVAL);
    *dr &= ~GPIO1_B4_BIT; // 清bit4 → 写0
    print_reg_status("写入0后", 0);
    sleep(SWITCH_INTERVAL);

    // 5. 初始写1（拉高），保持5秒
    printf("\n>>> 写入1（拉高），保持%d秒...\n", SWITCH_INTERVAL);
    *dr |= GPIO1_B4_BIT; // 置bit4 → 写1
    print_reg_status("写入1后", 1);
    sleep(SWITCH_INTERVAL);

    // 6. 循环切换0/1（每5秒一次）
    printf("\n=== 进入循环切换模式（按Ctrl+C退出）===\n");
    int level = 0; // 初始切换为0
    int count = 0;
    while (1) {
        count++;
        printf("\n--- 循环第%d次 ---", count);
        // 切换电平
        if (level) {
            *dr |= GPIO1_B4_BIT;  // 写1（拉高）
        } else {
            *dr &= ~GPIO1_B4_BIT; // 写0（拉低）
        }
        print_reg_status(level ? "写1（拉高）" : "写0（拉低）", level);
        sleep(SWITCH_INTERVAL); // 保持当前电平5秒
        level = !level; // 翻转电平（0→1，1→0）
    }

    return 0;
}
