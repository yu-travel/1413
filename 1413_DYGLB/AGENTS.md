# AGENTS.md - Agent Guidelines for 1413 Power Management Project

## Project Overview

This is an embedded STM32F4xx C project for power management systems. Multiple variants exist: `DYGLB`, `DYGLB_U22`, `DYGLB_U48`, `DYGLB_U57`. The project uses Keil MDK as the primary IDE.

## Build Commands

### Keil MDK (Primary)
- Open `.uvprojx` files in `Project/MDK/` directory
- Build: Keil uVision Build (F7)
- Rebuild: Keil uVision Rebuild (F7 with clean)
- Debug: Keil uVision Debug (F5)

### Testing
- **No automated unit tests exist** - this is bare-metal embedded code
- Manual testing on hardware is required
- Debug output via SEGGER RTT and UART

## Code Style Guidelines

### File Organization
```
DYGLB/
├── Core/           - CMSIS core files
├── FWLIB/          - STM32 HAL library
├── Hardware/BSP/   - Board driver implementations
├── Hardware/BSP_Port/ - Hardware port extensions
├── Middleware/     - Third-party middleware
├── Project/        - Keil project files and utilities
├── System/         - System drivers (usart, sys, delay)
├── User/           - Application code
└── Utilities/      - Type definitions and common utilities
```

### Header Files
- Use `#ifndef __FILENAME_H_` / `#define __FILENAME_H_` / `#endif` guards
- Include guard format: double underscore prefix/suffix
- Put include guard at very beginning of file, before any comments

### Type Definitions (types_def.h)
- Use custom types defined in `Utilities/types_def.h`:
  - Signed: `s16`, `s32`, `s64`
  - Unsigned: `u8`, `u16`, `u32`, `u64`
- Avoid raw `int`, `char` unless necessary for standard library
- Use `uint32_t` only when interfacing with STM32 HAL

### Naming Conventions
- **Functions**: `snake_case` (e.g., `adc_init`, `spi1_configuration`)
- **Variables**: `snake_case` with optional Hungarian prefix
  - Global arrays: `k_u32[13]`, `before_default_config_power`
  - Local variables: `ret`, `errNum`, `temp`
- **Constants/Macros**: `UPPER_SNAKE_CASE` (e.g., `DEBUG_OUT`, `TRUE`, `FALSE`)
- **Enumerations**: `UPPER_SNAKE_CASE` with `CMD22RUNING`, `CMD22RUNEND`

### Comments
- Use Chinese comments for file/function documentation
- Doxygen-style block comments for function descriptions:
  ```c
  /*
      @brief      : Initialize GPIO
      @param[in]  : none
      @param[out] : none
      @retval     : none
  */
  ```
- Inline comments for complex logic (Chinese or English)

### Formatting
- 4-space indentation (not tabs)
- Opening brace on same line as control statement
- Space after keywords: `if (`, `while (`, `for (`
- No space between function name and parenthesis: `main()` not `main ()`

### Error Handling
- Check function return values where applicable
- Use `if (buffer == NULL || length == 0)` pattern for pointer validation
- Return early on invalid parameters
- No exceptions - use return codes and error counters

### Debugging
- Use `TRACE_OUT(flag, ...)` for conditional debug output
- Use `myprintf()` or `PRINTF()` for direct output
- Debug flags in `types_def.h`: `DEBUG_OUT`, `LYSDEBUG`, `LYSDEBUG1-5`
- SEGGER RTT for real-time debugging without UART

### Import Organization (main.c example)
```c
#include "main.h"        // System/STM32 headers first
#include "delay.h"        // Internal headers (alphabetical)
#include "usart.h"
#include "softtimer.h"
#include "iwdg.h"
#include "timer.h"
#include "AD7606_Driver.h"
#include "AD5542_Driver.h"
#include "control_Dev.h"
#include "transf_jkkzb.h"
#include "types_def.h"
#include <math.h>         // Standard library last
```

### Memory and Performance
- Use `volatile` for hardware registers and ISR-shared variables
- Avoid dynamic memory allocation in production
- Use static allocation with fixed-size buffers
- Consider stack size in deeply recursive functions

### Hardware-Specific Notes
- Flash addresses: `0x080E0000` for calibration data storage
- Watchdog: `IWDG_Init(4, 800)` gives ~1.6s timeout
- System clock: 168MHz default
- UART: 115200 baud for debug output

### Middleware Usage
- **Logger (elog)**: Logging framework - see `elog_cfg.h` for configuration
- **Ring-Buffer**: `ringbuffer_u8.c/h`, `ringbuffer_u32.c/h`
- **MultiTimer**: `MultiTimer.c/h`, `softtimer.c/h` for software timers
- **Memory**: `alloc.c/h` for custom allocation

### Coding Rules
1. Never commit binary files, `.uvoptx`, `.uvguix.*` files
2. Add new source files to Keil project manually
3. Keep hardware initialization separate from application logic
4. Use `extern` sparingly - prefer header declarations
5. Define hardware pin mappings in respective BSP files

### Common Macros (types_def.h)
```c
#define ARRAY_SIZE(Array)   (sizeof(Array) / sizeof((Array)[0]))
#define MIN(i, j)           (((i) < (j)) ? (i) : (j))
#define MAX(i, j)           (((i) > (j)) ? (i) : (j))
#define BIT(x)              (1<<x)
```

### File Extensions
- `.c` - C source files
- `.h` - Header files
- `.uvprojx` - Keil project files
- `.uvoptx` - Keil project options (ignore in version control)

## Version Control

- Ignore: `*.uvoptx`, `*.uvguix.*`, `*.dep`, `*.lst`, `*.htm`, `*.axf`, `*.hex`, `*.bin`
- Standard C/header files should be version controlled
- Document changes in code comments