/*
 * Copyright (c) 2006-2023, Evlers Developers
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-01-24   Evlers      first implementation
 */

#include "board.h"
#include "rtthread.h"

#define  DWT_CR      *(volatile uint32_t *)0xE0001000
#define  DWT_CYCCNT  *(volatile uint32_t *)0xE0001004
#define  DEM_CR      *(volatile uint32_t *)0xE000EDFC

#define  DEM_CR_TRCENA                   (1 << 24)
#define  DWT_CR_CYCCNTENA                (1 <<  0)

int delay_init (void)
{
    DEM_CR |= (uint32_t)DEM_CR_TRCENA;      // Enable DWT
    DWT_CYCCNT = (uint32_t)0u;              // DWT CYCCNT clean
    DWT_CR |= (uint32_t)DWT_CR_CYCCNTENA;   // Enable DWT CYCCNT
    return 1;
}
INIT_BOARD_EXPORT(delay_init);

uint32_t get_cpu_tick (void)
{
    return ((uint32_t)DWT_CYCCNT);
}

void delay_ns (uint32_t ns)
{
    uint32_t ticks;
    uint32_t told = get_cpu_tick(), tnow = get_cpu_tick();

    /* Calculate the number of beats needed */
    ticks = ns / (1000000000 / SystemCoreClock);

    while (ticks >= (tnow - told))
    {
        tnow = get_cpu_tick();
    }
}

void delay_us (uint32_t us)
{
    uint32_t ticks;
    uint32_t told = get_cpu_tick(), tnow = get_cpu_tick();

    /* Calculate the number of beats needed */
    ticks = us * (SystemCoreClock / 1000000);

    while (ticks >= (tnow - told))
    {
        tnow = get_cpu_tick();
    }
}

void delay_ms (uint32_t ms)
{
    delay_us(ms * 1000);
}
