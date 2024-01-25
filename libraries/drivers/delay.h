/*
 * Copyright (c) 2006-2023, Evlers Developers
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-01-24   Evlers      first implementation
 */

#ifndef _DELAY_H_
#define _DELAY_H_

int delay_init (void);

uint32_t get_cpu_tick (void);
void delay_ns (uint32_t ns);
void delay_us (uint32_t us);
void delay_ms (uint32_t ms);

#endif // !_DELAY_H_
