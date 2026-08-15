#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

// ==================== GPIO1寄存器配置（原有）====================
#define GPIO1_BASE          0x28005000
#define GPIO_SWPORTB_DR     0x0C    // B组输出数据寄存器
#define GPIO_SWPORTB_DDR    0x10    // B组方向寄存器
#define GPIO_EXT_PORTB      0x14    // B组输入寄存器（实际硬件电平）
#define GPIO1_B4_BIT        (1 << 4)// B4对应bit4
#define MAP_SIZE            4096
#define SWITCH_INTERVAL     5       // 切换间隔：5秒

// ==================== 引脚复用寄存器配置（新增）====================
#define MUX_BASE            0x28180000  // 复用控制基地址
#define MUX_SD_DAT1_OFFSET  0x020C      // SD_DAT1复用控制偏移
#define MUX_SD_DAT1_FUNC_BITS (0x3 << 20) // [21:20]位：SD_DAT1复用功能位
#define MUX_FUNC_SD_DAT1    0x0 << 20   // func0：SD_DAT1功能
#define MUX_FUNC_GPIO1_B4   0x1 << 20   // func1：GPIO1_PORTB_4功能
#define MUX_SD_DAT1_PULL_BITS (0x3 << 22) // [23:22]位：上下拉控制（仅读取）

// ==================== 全局资源 ====================
void *gpio_virt = NULL;
void *mux_virt = NULL;   // 复用寄存器虚拟地址
int fd_mem = -1;

// ==================== 安全退出资源释放 ====================
void sig_exit(int sig) {
    printf("\n\n=== 开始释放资源 ===");
    // 释放GPIO映射
    if (gpio_virt != NULL && gpio_virt != MAP_FAILED) {
        munmap(gpio_virt, MAP_SIZE);
        printf("\n✓ GPIO1内存映射已释放");
    }
    // 释放复用寄存器映射
    if (mux_virt != NULL && mux_virt != MAP_FAILED) {
        munmap(mux_virt, MAP_SIZE);
        printf("\n✓ 复用寄存器内存映射已释放");
    }
    // 关闭文件
    if (fd_mem >= 0) {
        close(fd_mem);
        printf("\n✓ /dev/mem已关闭");
    }
    printf("\n程序已退出\n");
    exit(0);
}

// ==================== 打印GPIO寄存器状态（原有）====================
void print_gpio_status(const char *step, int target_level) {
    volatile unsigned int *ddr = (volatile unsigned int *)(gpio_virt + GPIO_SWPORTB_DDR);
    volatile unsigned int *dr  = (volatile unsigned int *)(gpio_virt + GPIO_SWPORTB_DR);
    volatile unsigned int *ext = (volatile unsigned int *)(gpio_virt + GPIO_EXT_PORTB);

    printf("\n=== %s（目标电平：%d）===\n", step, target_level);
    printf("  方向寄存器(DDR)：0x%08X → bit4=%d (1=输出模式)\n", 
           *ddr, (*ddr & GPIO1_B4_BIT) ? 1 : 0);
    printf("  输出寄存器(DR) ：0x%08X → bit4=%d (软件值，可写)\n", 
           *dr, (*dr & GPIO1_B4_BIT) ? 1 : 0);
    printf("  硬件电平(EXT) ：0x%08X → bit4=%d (硬件值)\n", 
           *ext, (*ext & GPIO1_B4_BIT) ? 1 : 0);
}

// ==================== 检测并配置SD_DAT1复用状态（新增核心逻辑）====================
int config_sd_dat1_mux() {
    // 1. 映射复用寄存器物理地址
    mux_virt = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, MUX_BASE);
    if (mux_virt == MAP_FAILED) {
        fprintf(stderr, "✗ 复用寄存器内存映射失败：%s\n", strerror(errno));
        return -1;
    }
    printf("✓ 复用寄存器映射成功（虚拟地址：%p）\n", mux_virt);

    // 2. 获取SD_DAT1复用控制寄存器值
    volatile unsigned int *mux_reg = (volatile unsigned int *)(mux_virt + MUX_SD_DAT1_OFFSET);
    unsigned int reg_val = *mux_reg;

    // 3. 解析复用状态
    unsigned int func_bits = (reg_val & MUX_SD_DAT1_FUNC_BITS) >> 20;
    unsigned int pull_bits = (reg_val & MUX_SD_DAT1_PULL_BITS) >> 22;
    
    printf("\n=== SD_DAT1引脚复用状态检测 ===\n");
    printf("  复用控制寄存器(0x%08X + 0x%04X)：0x%08X\n", MUX_BASE, MUX_SD_DAT1_OFFSET, reg_val);
    printf("  上下拉配置[23:22]：0x%X（复位值0x2）\n", pull_bits);
    printf("  复用功能[21:20]：0x%X → ", func_bits);
    if (func_bits == 0) {
        printf("当前为SD_DAT1功能（需要切换为GPIO）\n");
    } else if (func_bits == 1) {
        printf("当前为GPIO1_PORTB_4功能（无需切换）\n");
        return 0;
    } else {
        printf("未知功能（0x%X）\n", func_bits);
        return -1;
    }

    // 4. 切换为GPIO1_PORTB_4功能（禁止SD_DAT1复用）
    printf("\n>>> 开始切换复用功能为GPIO1_PORTB_4...\n");
    // 先清除原有功能位，再设置新功能（避免影响其他位）
    *mux_reg = (reg_val & ~MUX_SD_DAT1_FUNC_BITS) | MUX_FUNC_GPIO1_B4;
    // 验证切换结果
    unsigned int new_func_bits = ((*mux_reg) & MUX_SD_DAT1_FUNC_BITS) >> 20;
    if (new_func_bits == 1) {
        printf("✓ 复用功能切换成功！当前为GPIO1_PORTB_4功能\n");
        return 0;
    } else {
        fprintf(stderr, "✗ 复用功能切换失败！当前仍为0x%X\n", new_func_bits);
        return -1;
    }
}

int main(void) {
    // 注册Ctrl+C退出信号
    signal(SIGINT, sig_exit);
    printf("=== 腾锐D2000 GPIO1_B4 控制程序（含复用解除）===\n");
    printf("流程：检测复用 → 解除SD_DAT1复用 → GPIO配置 → 5秒循环切换电平\n\n");

    // 1. 打开物理内存（必须root）
    fd_mem = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd_mem < 0) {
        fprintf(stderr, "✗ 打开/dev/mem失败：%s\n", strerror(errno));
        fprintf(stderr, "  请用sudo运行！\n");
        return -1;
    }

    // 2. 检测并配置SD_DAT1复用（核心新增步骤）
    if (config_sd_dat1_mux() != 0) {
        fprintf(stderr, "✗ 复用配置失败，程序退出！\n");
        close(fd_mem);
        return -1;
    }

    // 3. 映射GPIO1物理地址
    gpio_virt = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, GPIO1_BASE);
    if (gpio_virt == MAP_FAILED) {
        fprintf(stderr, "✗ GPIO1内存映射失败：%s\n", strerror(errno));
        munmap(mux_virt, MAP_SIZE);
        close(fd_mem);
        return -1;
    }
    printf("✓ GPIO1物理地址映射成功（虚拟地址：%p）\n", gpio_virt);

    // 4. 初始化GPIO1_B4为输出模式
    volatile unsigned int *ddr = (volatile unsigned int *)(gpio_virt + GPIO_SWPORTB_DDR);
    volatile unsigned int *dr  = (volatile unsigned int *)(gpio_virt + GPIO_SWPORTB_DR);
    *ddr |= GPIO1_B4_BIT; // bit4置1 → 输出模式
    print_gpio_status("GPIO初始化完成", -1);

    // 5. 初始写0（拉低），保持5秒
    printf("\n>>> 写入0（拉低），保持%d秒...\n", SWITCH_INTERVAL);
    *dr &= ~GPIO1_B4_BIT; // 清bit4 → 写0
    print_gpio_status("写入0后", 0);
    sleep(SWITCH_INTERVAL);

    // 6. 初始写1（拉高），保持5秒
    printf("\n>>> 写入1（拉高），保持%d秒...\n", SWITCH_INTERVAL);
    *dr |= GPIO1_B4_BIT; // 置bit4 → 写1
    print_gpio_status("写入1后", 1);
    sleep(SWITCH_INTERVAL);

    // 7. 循环切换0/1（每5秒一次）
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
        print_gpio_status(level ? "写1（拉高）" : "写0（拉低）", level);
        sleep(SWITCH_INTERVAL); // 保持当前电平5秒
        level = !level; // 翻转电平（0→1，1→0）
    }

    return 0;
}
