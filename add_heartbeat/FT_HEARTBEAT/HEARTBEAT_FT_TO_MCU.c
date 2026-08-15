#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

// ==================== 寄存器配置 ====================
// GPIO1配置
#define GPIO1_BASE          0x28005000
#define GPIO_SWPORTB_DR     0x0C    // B组输出数据寄存器
#define GPIO_SWPORTB_DDR    0x10    // B组方向寄存器
#define GPIO1_B4_BIT        (1 << 4)// B4对应bit4
#define MAP_SIZE            4096
#define HEARTBEAT_INTERVAL  1       // 心跳间隔：1秒

// 引脚复用配置
#define MUX_BASE            0x28180000  // 复用控制基地址
#define MUX_SD_DAT1_OFFSET  0x020C      // SD_DAT1复用控制偏移
#define MUX_SD_DAT1_FUNC_BITS (0x3 << 20) // [21:20]位：复用功能位
#define MUX_FUNC_GPIO1_B4   0x1 << 20   // func1：GPIO1_PORTB_4功能

// ==================== 全局资源 ====================
void *gpio_virt = NULL;
void *mux_virt = NULL;
int fd_mem = -1;

// ==================== 安全退出（无打印）====================
void sig_exit(int sig) {
    // 释放映射资源
    if (gpio_virt != NULL && gpio_virt != MAP_FAILED) {
        munmap(gpio_virt, MAP_SIZE);
    }
    if (mux_virt != NULL && mux_virt != MAP_FAILED) {
        munmap(mux_virt, MAP_SIZE);
    }
    if (fd_mem >= 0) {
        close(fd_mem);
    }
    exit(0);
}

// ==================== 配置SD_DAT1复用为GPIO（无打印）====================
int config_sd_dat1_mux() {
    // 映射复用寄存器
    mux_virt = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, MUX_BASE);
    if (mux_virt == MAP_FAILED) {
        return -1;
    }

    // 切换为GPIO1_B4功能
    volatile unsigned int *mux_reg = (volatile unsigned int *)(mux_virt + MUX_SD_DAT1_OFFSET);
    unsigned int reg_val = *mux_reg;
    *mux_reg = (reg_val & ~MUX_SD_DAT1_FUNC_BITS) | MUX_FUNC_GPIO1_B4;

    // 验证切换结果
    unsigned int new_func_bits = ((*mux_reg) & MUX_SD_DAT1_FUNC_BITS) >> 20;
    if (new_func_bits != 1) {
        return -1;
    }
    return 0;
}

// ==================== 心跳函数（核心：1秒下降沿）====================
void heartbeat_loop() {
    volatile unsigned int *dr = (volatile unsigned int *)(gpio_virt + GPIO_SWPORTB_DR);
    
    // 初始电平设为高（下降沿需要先高后低）
    *dr |= GPIO1_B4_BIT;
    usleep(1000); // 稳定初始电平

    // 无限循环生成下降沿（1秒一次）
    while (1) {
        // 步骤1：拉高（确保初始为高）
        *dr |= GPIO1_B4_BIT;
        usleep(500000); // 高电平保持500ms
        
        // 步骤2：拉低（生成下降沿）
        *dr &= ~GPIO1_B4_BIT;
        usleep(500000); // 低电平保持500ms
        // 整体周期1秒，下降沿每1秒触发一次
    }
}

int main(void) {
    // 注册退出信号（Ctrl+C）
    signal(SIGINT, sig_exit);

    // 1. 打开物理内存
    fd_mem = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd_mem < 0) {
        return -1;
    }

    // 2. 配置引脚复用（解除SD_DAT1复用）
    if (config_sd_dat1_mux() != 0) {
        close(fd_mem);
        return -1;
    }

    // 3. 映射GPIO1地址
    gpio_virt = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, GPIO1_BASE);
    if (gpio_virt == MAP_FAILED) {
        munmap(mux_virt, MAP_SIZE);
        close(fd_mem);
        return -1;
    }

    // 4. 配置GPIO1_B4为输出模式
    volatile unsigned int *ddr = (volatile unsigned int *)(gpio_virt + GPIO_SWPORTB_DDR);
    *ddr |= GPIO1_B4_BIT;

    // 5. 启动心跳循环
    heartbeat_loop();

    return 0;
}
