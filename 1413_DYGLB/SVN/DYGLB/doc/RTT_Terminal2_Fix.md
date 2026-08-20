# RTT 终端 2 输出修复记录

## 修复时间
2026-08-20

## 问题描述
用户将日志输出函数改为 `SEGGER_RTT_printf(2, ...)`，将日志输出位置从默认的 Terminal/channel 0 移至 RTT 终端 2（up-buffer index 2）。然而，实际测试后发现 **无任何输出**，RTT 终端 2 完全静默。

## 根因分析
- SEGGER RTT 的 `_DoInit()` 函数仅在系统启动时**默认初始化 up-buffer 0**（以及 down-buffer 0）。
- Buffers 1 和 2 被 memsetting 为全零，即 `pBuffer = NULL`、`Flags = 0`、`SizeOfBuffer = 0`。
- 在 `SEGGER_RTT_WriteNoLock()` 中，当 `pRing->Flags == 0` 时直接命中 `switch` 的 `default` 分支，返回 `Status = 0u`，** silently drop（静默丢弃）所有字节**，并无任何错误返回或断言。

用户最初的猜测“可能没调用 SEGGER_RTT_ConfigUpBuffer”完全正确。

## 修复方案
在 `diag_init()` 函数首部调用 `SEGGER_RTT_ConfigUpBuffer()`，显式配置 up-buffer 2 为一个有效的带名字的环形缓冲区，供后续 `SEGGER_RTT_printf(2, ...)` 写入使用。

### 关键代码 (App/app_diag.c)
```c
void diag_init(void)
{
    /* 初始化 SEGGER RTT 通道 2 (buffer 2), 仅在 CHIP_TEST_LOG=1 时使用 */
    static char s_diag_rtt_buf[2048];
    SEGGER_RTT_ConfigUpBuffer(2, "DIAG", s_diag_rtt_buf,
                              sizeof(s_diag_rtt_buf), SEGGER_RTT_MODE_NO_BLOCK_TRIM);

#if DIAG_EFUSE_ACTIVE_TEST
    power_diag_test_seq();
#else
    TRACE_OUT_2(DEBUG_OUT, "[DIAG] efuse active test disabled (DIAG_EFUSE_ACTIVE_TEST=0)\r\n");
#endif
}
```

### 参数说明
- **BufferIndex**: 2（对应 SEGGER_RTT_printf 第一个参数的索引）
- **sName**: "DIAG"（在 J-Link RTT Viewer 的通道下拉框中显示的名称）
- **pBuffer**: 2048 字节静态缓冲区（每秒诊断输出约 3KB，勉足够）
- **BufferSize**: `sizeof(s_diag_rtt_buf)` = 2048
- **Flags**: `SEGGER_RTT_MODE_NO_BLOCK_TRIM` —— 缓冲满时**裁剪尾部**而非整行丢弃（NO_BLOCK_SKIP 会在不够整行时直接丢弃 entire line）

### 配套变更
1. **App/app_main.c** — 用 `#if CHIP_TEST_LOG` / `#else` / `#endif` 包裹 `diag_init()` 调用，避免当 `CHIP_TEST_LOG=0` 时因函数未定义而链接错误。
2. **App/app_diag.c** — 将 `diag_xca_dump()` 中的 `TRACE_OUT` 统一改为 `TRACE_OUT_2`，保持诊断输出全部进入 buffer 2。
3. 先前合入的其他文件（`types_def.h`、`app_monitor.c`、`app_power.c`、`app_protocol.c`、`LC1258.txt`）与 E6 配方固化及诊断模块开发相关，本次未重新修改。

## 使用说明
- 烧录固件后，打开 **J-Link RTT Viewer**。
- 在上方 **通道（Channel）** 下拉框中，默认选中 **"Terminal"**（即 channel 0，对应原来的 TRACE_OUT 输出）。
- 手动将通道切换至 **"DIAG"**（即 newly configured up-buffer 2），即可看到诊断日志输出。
- channel 0 继续输出心跳、协议、报错信息，两者互不干扰。

## 文件修改摘要
| 文件 | 主要变更 |
|------|----------|
| `App/app_diag.c` | 新增 `SEGGER_RTT_ConfigUpBuffer(2, "DIAG", ..., SEGGER_RTT_MODE_NO_BLOCK_TRIM)`；`diag_xca_dump` 中 `TRACE_OUT` → `TRACE_OUT_2` |
| `App/app_main.c` | `#if CHIP_TEST_LOG` 包裹 `diag_init()` 调用 |
| `App/app_monitor.c` |（先前）`monitor_diag_dump` 改用 `TRACE_OUT_2` |
| `App/app_power.c` |（先前）`power_diag_dump`/`power_diag_test_seq` 新增 + 缓存 |
| `App/app_protocol.c` |（先前）注释掉一条 `TRACE_OUT` |
| `Utilities/types_def.h` |（先前）增 `myprintf_1/2` 与 `TRACE_OUT_2` 宏 |
| `doc/LC1258.txt` |（先前）Task 7 补丁 #4 记录 E6 固化 |

## 验证
- Keil MDK Build: **0 Error, 0 Warning**（已修换行符相关 3 个 Warning）
- Git commit `14b516e` 已推送至 GitHub