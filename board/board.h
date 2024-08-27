/*
 * Copyright (c) 2006-2024 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-08-27     Evlers       first implementation
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#include "gd32f4xx.h"
#include "gd32f4xx_exti.h"

#include <rtthread.h>

#ifdef RT_USING_PIN
#include "drv_gpio.h"
#endif

#define GD32_FLASH_START_ADRESS         ((uint32_t)0x08000000)
#define GD32_FLASH_SIZE                 (2048 * 1024)
#define GD32_FLASH_PAGE_SIZE            (4 * 1024)
#define GD32_FLASH_END_ADDRESS          ((uint32_t)(GD32_FLASH_START_ADRESS + GD32_FLASH_SIZE))
#define EXT_SDRAM_BEGIN                 (0xC0000000U) /* the begining address of external SDRAM */
#define EXT_SDRAM_END                   (EXT_SDRAM_BEGIN + (32U * 1024 * 1024)) /* the end address of external SDRAM */

/* SRAM Size:704KB, Addr:0x20000000  TCM RAM Size: 64KB, Addr:0x10000000 */
#define GD32_SRAM0_SIZE                 112
#define GD32_SRAM1_SIZE                 16
#define GD32_SRAM2_SIZE                 64
#define GD32_ADDSRAM_SIZE               512
#define GD32_TCMRAM_SIZE                64  // TCMSRAM(紧耦合存储器SRAM) 只可被Cortex®-M4内核的数据总线访问
#define GD32_SRAM_SIZE                  (GD32_SRAM0_SIZE + GD32_SRAM1_SIZE + GD32_SRAM2_SIZE + GD32_ADDSRAM_SIZE)
#define GD32_SRAM_END                   (0x20000000 + GD32_SRAM_SIZE * 1024)

#ifdef __ARMCC_VERSION
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define HEAP_BEGIN                      (&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section="HEAP"
#define HEAP_BEGIN                      (__segment_end("HEAP"))
#else
extern int __bss_end;
#define HEAP_BEGIN                      (&__bss_end)
#endif

#define HEAP_END                        GD32_SRAM_END

#endif
