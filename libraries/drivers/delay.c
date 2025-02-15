/*
 * Copyright (c) 2006-2024 Evlers Developers
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-01-24   Evlers      first implementation
 */

#include "board.h"
#include "rtthread.h"

/* Note: After downloading using JLink,
 * you need to enter debug mode,
 * otherwise the DWT counter is stopped.
 */

int delay_init (void)
{
    DWT->CTRL |= DWT_CTRL_NOPRFCNT_Msk;             /* Enable DWT */
    DWT->CYCCNT = (uint32_t)0u;                     /* DWT CYCCNT clean */
    DWT->CTRL |= (uint32_t)DWT_CTRL_CYCCNTENA_Msk;  /* Enable DWT CYCCNT */
    return 1;
}
INIT_BOARD_EXPORT(delay_init);

uint32_t get_cpu_tick (void)
{
    return ((uint32_t)DWT->CYCCNT);
}

void delay_ns (uint32_t ns)
{
    uint32_t ticks;
    uint32_t told = get_cpu_tick(), tnow = get_cpu_tick();

    /* calculate the number of beats needed */
    ticks = ns / (1000000000 / SystemCoreClock);

    while (ticks >= (tnow - told))
    {
        tnow = get_cpu_tick();
    }
}

rt_inline void _delay_us (uint32_t us)
{
    uint32_t ticks;
    uint32_t told = get_cpu_tick(), tnow = get_cpu_tick();

    /* calculate the number of beats needed */
    ticks = us * (SystemCoreClock / 1000000);

    while (ticks >= (tnow - told))
    {
        tnow = get_cpu_tick();
    }
}

void delay_us (uint32_t us)
{
    _delay_us(us);
}

void delay_ms (uint32_t ms)
{
    delay_us(ms * 1000);
}

void rt_hw_us_delay (rt_uint32_t us)
{
    _delay_us(us);
}
