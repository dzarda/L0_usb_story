#pragma once

#ifndef _FROM_ASM_

#include <stdint.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

// ChibiOS/HAL requires these macros
#define STM32_LSECLK
#define STM32_LSEDRV
#define STM32_HSECLK

// ChibiOS/HAL uses this symbol
void boardInit();

#ifdef __cplusplus
} // extern "C"
#endif

#endif
