#ifndef __APP_PROTOCOL_H_
#define __APP_PROTOCOL_H_

#include "app_config.h"

/*
    @brief      : FPGA SPI 协议组包/解包/校验与前导帧读取 (App 层)
    @note       : 1. 依赖 Bsp 层 bsp_spi (本层合法依赖), 不依赖 User 层
                  2. 帧格式: [帧头 2B][帧长度 2B=内容字节数][帧内容][校验和 2B][帧尾 2B]
                  3. 校验和 = 帧头到校验和字段之前所有 16 位字段值累加, 保留低 16 位
                     (文档样例: 0x55AA+0x0002+0xABDE=0x1018A -> sum=0x018A)
                  4. 字节序: 帧内 u16 字段按小端组包, 切换字节序仅需改
                     app_protocol.c 内 put16_le/rd16_le 两个静态函数
*/

/*
    @brief      : 上传帧组包 (MCU -> FPGA)
    @note       : 帧 = 帧头 0x55AA + 长度 0x0060 + 15x(ID+V+I) + 默认状态
                  + 开关状态 + 告警状态 + 校验和 + 帧尾 0xACBC, 共 104B;
                  契约: m 必须 15 组且按设备ID 1~15 顺序填充 (调用方保证)
    @param[in]  : buf 输出缓冲 (容量 >= PROTO_TX_BUF_LEN)
                  m    15 组设备测量值
                  ps   电源状态字
    @param[out] : none
    @retval     : 帧总字节数 104; 参数无效返回 0
*/
u16 protocol_build_upload(u8 *buf, const dev_measure_t *m, const power_state_t *ps);

/*
    @brief      : 下发帧解包 (FPGA -> MCU)
    @note       : 依次校验帧头 0xAA55 / 帧长度 / 校验和 / 帧尾, 全部通过后
                  提取 15 组基准电压/电流与 3 个状态字
    @param[in]  : buf 接收到的完整帧缓冲
                  len 缓冲有效长度 (>= 帧总长 104B)
    @param[out] : thr 15 组设备基准阈值
                  ps  电源状态字
    @retval     : 1 = 校验通过且已提取; 0 = 校验失败或参数无效
*/
u8  protocol_parse_down(const u8 *buf, u16 len, dev_threshold_t *thr, power_state_t *ps);

/*
    @brief      : 协议校验和计算
    @note       : 按 16 位字段值累加 (小端解读), 保留低 16 位;
                  n 应为偶数 (帧长恒偶), 奇数时末字节按低字节防御性计入
    @param[in]  : buf 待校验数据
                  n   字节数
    @param[out] : none
    @retval     : 校验和 (低 16 位)
*/
u16 protocol_calc_sum(const u8 *buf, u16 n);

/*
    @brief      : 发送 10B 前导帧 (发起读请求)
    @note       : 前导帧 = 0x55AA + 0x0002 + 0xABDE + 0x018A + 0xACBC;
                  校验和 0x018A 与文档样例一致 (已验证)
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void protocol_send_preamble(void);

/*
    @brief      : 一次完整收发周期: 发前导帧 -> 连续时钟接收下发帧 -> 解包
    @note       : 待确认#2 握手时序按"发前导帧后连续时钟接收"的标准 SPI
                  主从模式实现, 前导发送与接收窗口分段实现, 便于联调调整;
                  解包结果存模块静态区, 通过 protocol_read_result() 读取
    @param[in]  : none
    @param[out] : none
    @retval     : 1 = 收到并解包成功; 0 = 未找到帧头或校验失败
*/
u8  protocol_read_task(void);

/*
    @brief      : 读取 protocol_read_task() 最近一次解包结果
    @note       : Task9 补充接口 (原 5 函数规格外), 供监控层轮询读取
    @param[in]  : none
    @param[out] : thr 15 组设备基准阈值 (可为 NULL 不取)
                  ps  电源状态字 (可为 NULL 不取)
    @retval     : none
*/
void protocol_read_result(dev_threshold_t *thr, power_state_t *ps);

#endif /* __APP_PROTOCOL_H_ */
