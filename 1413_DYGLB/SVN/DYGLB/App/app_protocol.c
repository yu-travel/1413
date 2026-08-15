#include "app_protocol.h"
#include "bsp_spi.h"

/*
    @brief      : FPGA SPI 协议组包/解包/校验与前导帧读取 (App 层)
    @note       : 1. 帧格式: [帧头 2B][帧长度 2B=内容字节数][帧内容][校验和 2B][帧尾 2B]
                  2. 上传帧头 0x55AA (MCU->FPGA), 下发帧头 0xAA55 (FPGA->MCU),
                     帧尾固定 0xACBC
                  3. 校验和: 帧头到校验和字段之前所有 16 位字段值累加,
                     保留低 16 位; 文档验证样例: 前导帧
                     0x55AA+0x0002+0xABDE=0x1018A -> sum=0x018A (已按本实现复核通过)
                  4. 字节序假设: 帧内所有 u16 字段按小端组包 (低字节在前);
                     待联调确认#3: 若 FPGA 实际为大端, 仅需将 put16_le/rd16_le
                     改为大端实现, 其余代码不变
                  5. 待确认#2 握手时序: 按"发前导帧后连续时钟接收下发帧"
                     标准 SPI 主从模式实现, 前导发送与接收窗口分段隔离,
                     联调调整点集中
*/

/* 帧总长 = 帧头2 + 长度2 + 内容96 + 校验和2 + 帧尾2 = 104B */
#define FRAME_LEN_TOTAL     (2u + 2u + FRAME_LEN_CONTENT + 2u + 2u)

/* 前导帧 = 头2 + 长度2 + 命令字2 + 校验和2 + 尾2 = 10B */
#define PROTO_PREAMBLE_LEN  10u

/* 一次读流程接收窗口 = 前导 10B (回声) + 下发帧全长 104B = 114B */
#define PROTO_RX_WIN_LEN    (PROTO_PREAMBLE_LEN + FRAME_LEN_TOTAL)

/*
    @brief      : 小端写入 u16 (低字节在前)
    @param[in]  : buf  目标地址 (至少 2B)
                  val  数值
    @param[out] : none
    @retval     : none
*/
static void put16_le(u8 *buf, u16 val)
{
    buf[0] = (u8)(val & 0xFFu);
    buf[1] = (u8)(val >> 8);
}

/*
    @brief      : 小端读取 u16 (低字节在前)
    @param[in]  : buf 源地址 (至少 2B)
    @param[out] : none
    @retval     : 读出数值
*/
static u16 rd16_le(const u8 *buf)
{
    return (u16)((u16)buf[0] | ((u16)buf[1] << 8));
}

/*
    @brief      : 前导帧字节流 (小端, 共 10B)
    @note       : 帧头 0x55AA -> AA 55; 长度 0x0002 -> 02 00;
                  命令字 0xABDE -> DE AB; 校验和 0x018A -> 8A 01;
                  帧尾 0xACBC -> BC AC;
                  校验和复核: 0x55AA+0x0002+0xABDE=0x1018A -> 低16位 0x018A,
                  与文档样例一致
*/
static const u8 preamble_frame[PROTO_PREAMBLE_LEN] = {
    0xAA, 0x55,             /* 帧头 0x55AA (小端) */
    0x02, 0x00,             /* 帧长度 0x0002 (内容 2B) */
    0xDE, 0xAB,             /* 读命令字 0xABDE */
    0x8A, 0x01,             /* 校验和 0x018A */
    0xBC, 0xAC              /* 帧尾 0xACBC */
};

/*
    @brief      : 协议校验和计算
    @note       : 按 16 位字段值累加 (小端解读), 保留低 16 位;
                  n 应为偶数 (帧长恒偶), 奇数时末字节按低字节防御性计入
    @param[in]  : buf 待校验数据
                  n   字节数
    @param[out] : none
    @retval     : 校验和 (低 16 位)
*/
u16 protocol_calc_sum(const u8 *buf, u16 n)
{
    u16 sum = 0u;
    u16 i;

    if (buf == NULL || n < 2u) {
        return 0u;
    }

    for (i = 0u; i + 1u < n; i += 2u) {
        sum += rd16_le(&buf[i]);
    }

    /* 奇数长度防御: 末字节计入低字节位 (正常帧长恒偶不会走到) */
    if ((n & 1u) != 0u) {
        sum += (u16)buf[n - 1u];
    }

    return sum;
}

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
u16 protocol_build_upload(u8 *buf, const dev_measure_t *m, const power_state_t *ps)
{
    u16 idx = 0u;
    u16 i;

    if (buf == NULL || m == NULL || ps == NULL) {
        return 0u;
    }

    /* 帧头 0x55AA */
    put16_le(&buf[idx], PROTO_HEAD_UP);
    idx += 2u;

    /* 帧长度 = 内容字节数 0x0060 (96B); 待确认#1: 文档写 0x5F(95B),
       实际内容 90+6=96B, 以 0x60 实现 */
    put16_le(&buf[idx], FRAME_LEN_CONTENT);
    idx += 2u;

    /* 内容: 15 组 ID+电压+电流 (按 m 数组顺序, 由调用方保证 ID 1~15 升序) */
    for (i = 0u; i < PROTO_DEV_NUM; i++) {
        put16_le(&buf[idx], m[i].id);
        idx += 2u;
        put16_le(&buf[idx], m[i].vol_mv);
        idx += 2u;
        put16_le(&buf[idx], m[i].cur_ma);
        idx += 2u;
    }

    /* 状态字: 默认状态 / 开关状态 / 告警状态 */
    put16_le(&buf[idx], ps->default_state);
    idx += 2u;
    put16_le(&buf[idx], ps->switch_state);
    idx += 2u;
    put16_le(&buf[idx], ps->alarm_state);
    idx += 2u;

    /* 校验和: 帧头到校验和字段之前 (idx 字节) */
    put16_le(&buf[idx], protocol_calc_sum(buf, idx));
    idx += 2u;

    /* 帧尾 0xACBC */
    put16_le(&buf[idx], PROTO_TAIL);
    idx += 2u;

    return idx;
}

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
u8 protocol_parse_down(const u8 *buf, u16 len, dev_threshold_t *thr, power_state_t *ps)
{
    u16 idx;
    u16 i;

    if (buf == NULL || thr == NULL || ps == NULL) {
        return 0u;
    }

    if (len < FRAME_LEN_TOTAL) {
        return 0u;
    }

    /* 帧头 0xAA55 */
    if (rd16_le(&buf[0]) != PROTO_HEAD_DOWN) {
        return 0u;
    }

    /* 帧长度 = 内容字节数 0x0060 */
    if (rd16_le(&buf[2]) != FRAME_LEN_CONTENT) {
        return 0u;
    }

    /* 校验和: 帧头到校验和字段之前 (按帧定长 104B, 与入参 len 解耦,
       防止 len 大于帧长时把帧尾垃圾字节计入) */
    if (protocol_calc_sum(buf, FRAME_LEN_TOTAL - 4u) != rd16_le(&buf[FRAME_LEN_TOTAL - 4u])) {
        return 0u;
    }

    /* 帧尾 0xACBC */
    if (rd16_le(&buf[FRAME_LEN_TOTAL - 2u]) != PROTO_TAIL) {
        return 0u;
    }

    /* 内容: 15 组 ID+基准电压+基准电流 */
    idx = 4u;
    for (i = 0u; i < PROTO_DEV_NUM; i++) {
        thr[i].id = rd16_le(&buf[idx]);
        idx += 2u;
        thr[i].ref_vol_mv = rd16_le(&buf[idx]);
        idx += 2u;
        thr[i].ref_cur_ma = rd16_le(&buf[idx]);
        idx += 2u;
    }

    /* 状态字 */
    ps->default_state = rd16_le(&buf[idx]);
    idx += 2u;
    ps->switch_state = rd16_le(&buf[idx]);
    idx += 2u;
    ps->alarm_state = rd16_le(&buf[idx]);

    return 1u;
}

/*
    @brief      : 发送 10B 前导帧 (发起读请求)
    @note       : 片选自管理 (低选中 -> 发送 -> 高释放)
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void protocol_send_preamble(void)
{
    bsp_spi_cs(BSP_SPI_CS_LOW);
    bsp_spi_transfer((u8 *)preamble_frame, NULL, PROTO_PREAMBLE_LEN);
    bsp_spi_cs(BSP_SPI_CS_HIGH);
}

/* 下发帧接收窗口: 前导回声 + 下发帧全长, 由 protocol_read_task 使用 */
static u8 rx_buf[PROTO_RX_BUF_LEN];

/* 最近一次解包结果 (由 protocol_read_result 读取) */
static dev_threshold_t down_thr[PROTO_DEV_NUM];
static power_state_t   down_ps;

/* 接收下发帧时主机发送的哑字节 (FPGA 忽略 MOSI) */
static const u8 dummy_tx[FRAME_LEN_TOTAL] = {0};

/*
    @brief      : 一次完整收发周期: 发前导帧 -> 连续时钟接收下发帧 -> 解包
    @note       : 1. 片选全程保持选中, 分两段传输:
                     第1段 发前导 10B (同时收回声);
                     第2段 发哑字节 104B 连续时钟收下发帧 (帧全长 104B,
                     若只发 96B 会少收帧尾 8B, 故取帧全长)
                  2. 接收完成后在窗口内扫描下发帧头 0xAA55 (小端 55 AA);
                     前导回声或 FPGA 引入的前导噪声字节不影响扫描定位
                  3. 握手时序 (待确认#2) 的调整点集中在本函数两段传输
                     与扫描窗口, 不影响组包/解包逻辑
    @param[in]  : none
    @param[out] : none
    @retval     : 1 = 收到并解包成功; 0 = 未找到帧头或校验失败
*/
u8 protocol_read_task(void)
{
    u16 i;
    u16 head_idx;

    /* 第1段: 发前导帧并接收回声 */
    bsp_spi_cs(BSP_SPI_CS_LOW);
    bsp_spi_transfer((u8 *)preamble_frame, rx_buf, PROTO_PREAMBLE_LEN);

    /* 第2段: 连续时钟接收下发帧 (帧全长 104B) */
    bsp_spi_transfer((u8 *)dummy_tx, &rx_buf[PROTO_PREAMBLE_LEN], FRAME_LEN_TOTAL);
    bsp_spi_cs(BSP_SPI_CS_HIGH);

    /* 在接收窗口内扫描下发帧头 0xAA55 (小端字节序: 55 AA) */
    head_idx = 0xFFFFu;
    for (i = 0u; i + 1u < PROTO_RX_WIN_LEN; i++) {
        if (rx_buf[i] == (u8)(PROTO_HEAD_DOWN & 0xFFu) &&
            rx_buf[i + 1u] == (u8)(PROTO_HEAD_DOWN >> 8)) {
            head_idx = i;
            break;
        }
    }

    if (head_idx == 0xFFFFu) {
        return 0u;
    }

    /* 解包并存入模块静态结果区 */
    return protocol_parse_down(&rx_buf[head_idx], PROTO_RX_WIN_LEN - head_idx,
                               down_thr, &down_ps);
}

/*
    @brief      : 读取 protocol_read_task() 最近一次解包结果
    @note       : Task9 补充接口 (原 5 函数规格外), 供监控层轮询读取
    @param[in]  : none
    @param[out] : thr 15 组设备基准阈值 (可为 NULL 不取)
                  ps  电源状态字 (可为 NULL 不取)
    @retval     : none
*/
void protocol_read_result(dev_threshold_t *thr, power_state_t *ps)
{
    u16 i;

    if (thr != NULL) {
        for (i = 0u; i < PROTO_DEV_NUM; i++) {
            thr[i] = down_thr[i];
        }
    }

    if (ps != NULL) {
        *ps = down_ps;
    }
}
