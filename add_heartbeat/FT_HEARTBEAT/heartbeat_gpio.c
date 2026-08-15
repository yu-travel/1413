#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

// 腾锐D2000 GPIO1控制器基地址（手册规范）
#define GPIO1_BASE       0x28005000
// 寄存器偏移：方向控制/输出电平
#define GPIO_DIR_OFFSET  0x0c
#define GPIO_OUT_OFFSET  0x10
// GPIO1_B4 对应的bit位（bit4）
#define GPIO1_B4_BIT     (1 << 4)  // 等价于0x10
// 心跳周期：1秒（1000ms），下降沿持续时间：100ms（可调整）
#define HEARTBEAT_PERIOD 1000000    // 单位：微秒（1秒=1000000微秒）
#define FALL_EDGE_DELAY  100000     // 下降沿持续时间（100ms）

// 全局变量：保存映射后的虚拟地址，用于退出时释放
void *gpio1_virt_base = NULL;
int mem_fd = -1;

// 信号处理函数：捕获Ctrl+C，安全退出并恢复GPIO状态
void sigint_handler(int sig) {
    printf("\n捕获到退出信号，正在恢复GPIO状态...\n");
    
    // 恢复GPIO1_B4为高电平（避免引脚异常）
    if (gpio1_virt_base != NULL) {
        volatile unsigned int *gpio_out = (volatile unsigned int *)(gpio1_virt_base + GPIO_OUT_OFFSET);
        *gpio_out |= GPIO1_B4_BIT;  // 置bit4为1（高电平）
        printf("GPIO1_B4 已恢复为高电平\n");
        
        // 释放内存映射
        munmap(gpio1_virt_base, 4096);
        gpio1_virt_base = NULL;
    }
    
    // 关闭/dev/mem文件
    if (mem_fd >= 0) {
        close(mem_fd);
        mem_fd = -1;
    }
    
    printf("心跳程序已安全退出\n");
    exit(0);
}

// GPIO初始化函数：配置GPIO1_B4为输出模式，初始高电平
int gpio1_b4_init(void) {
    // 1. 打开物理内存设备（必须root权限）
    mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_fd < 0) {
        perror("打开/dev/mem失败（需root权限）");
        return -1;
    }

    // 2. 映射物理地址到虚拟地址（4096字节=1页）
    gpio1_virt_base = mmap(
        NULL,                    // 不指定虚拟地址，由系统分配
        4096,                    // 映射大小：1页
        PROT_READ | PROT_WRITE,  // 可读可写
        MAP_SHARED,              // 共享映射（修改会同步到物理地址）
        mem_fd,                  // /dev/mem文件描述符
        GPIO1_BASE               // 要映射的物理地址
    );
    
    if (gpio1_virt_base == MAP_FAILED) {
        perror("内存映射失败");
        close(mem_fd);
        mem_fd = -1;
        return -1;
    }

    // 3. 配置GPIO1_B4为输出模式（仅修改bit4，保留其他位）
    volatile unsigned int *gpio_dir = (volatile unsigned int *)(gpio1_virt_base + GPIO_DIR_OFFSET);
    *gpio_dir |= GPIO1_B4_BIT;  // bit4置1（输出模式）
    printf("GPIO1_B4 方向已设为输出模式\n");

    // 4. 设置初始电平为高电平（仅修改bit4，保留其他位）
    volatile unsigned int *gpio_out = (volatile unsigned int *)(gpio1_virt_base + GPIO_OUT_OFFSET);
    *gpio_out |= GPIO1_B4_BIT;  // bit4置1（高电平）
    printf("GPIO1_B4 初始电平设为高电平\n");

    return 0;
}

// 心跳函数：每1秒产生一个下降沿
void heartbeat_gpio1_b4(void) {
    volatile unsigned int *gpio_out = (volatile unsigned int *)(gpio1_virt_base + GPIO_OUT_OFFSET);
    int count = 0;

    printf("心跳GPIO程序已启动，每1秒输出一个下降沿（按Ctrl+C退出）\n");
    printf("当前计数 | GPIO1_B4 状态\n");
    printf("------------------------\n");

    while (1) {
        count++;
        
        // 步骤1：输出低电平（产生下降沿）
        *gpio_out &= ~GPIO1_B4_BIT;  // 清bit4为0（低电平）
        printf("%8d | 低电平（下降沿）\n", count);
        usleep(FALL_EDGE_DELAY);     // 保持低电平100ms
        
        // 步骤2：恢复高电平
        *gpio_out |= GPIO1_B4_BIT;   // 置bit4为1（高电平）
        printf("%8d | 高电平\n", count);
        usleep(HEARTBEAT_PERIOD - FALL_EDGE_DELAY);  // 剩余时间保持高电平
    }
}

// 主函数：初始化+注册信号+启动心跳
int main(int argc, char *argv[]) {
    // 注册Ctrl+C信号处理函数（安全退出）
    signal(SIGINT, sigint_handler);

    // 初始化GPIO1_B4
    if (gpio1_b4_init() != 0) {
        printf("GPIO1_B4 初始化失败，程序退出\n");
        return -1;
    }

    // 启动心跳函数
    heartbeat_gpio1_b4();

    // 正常退出（实际不会执行到这里，因为心跳函数是死循环）
    sigint_handler(SIGINT);
    return 0;
}
