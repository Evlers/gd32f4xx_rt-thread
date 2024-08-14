/*
 * Copyright (c) 2006-2024 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-03-20   Evlers      first implementation
 */

#ifndef _SDIO_CONFIG_H_
#define _SDIO_CONFIG_H_

#include <rtthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SDIO_BUFF_SIZE
#define SDIO_BUFF_SIZE              (4096)
#endif

#if defined(BSP_USING_SDIO)
#ifndef SDIO_CONFIG
#define SDIO_CONFIG                                         \
    {                                                       \
        .periph = SDIO,                                     \
        .sdio_clock_freq = (48U * 1000 * 1000),             \
        .sdio_max_freq = (24 * 1000 * 1000),                \
        .sdio_rcu = RCU_SDIO,                               \
        .irqn = SDIO_IRQn,                                  \
        RCU_GPIOC, GPIOC, GPIO_PIN_12, /* CLK */            \
        RCU_GPIOD, GPIOD, GPIO_PIN_2,  /* CMD */            \
        RCU_GPIOC, GPIOC, GPIO_PIN_8,  /* D0 */             \
        RCU_GPIOC, GPIOC, GPIO_PIN_9,  /* D1 */             \
        RCU_GPIOC, GPIOC, GPIO_PIN_10, /* D2 */             \
        RCU_GPIOC, GPIOC, GPIO_PIN_11, /* D3 */             \
        GPIO_AF_12,                                         \
    }
#endif /* SDIO_CONFIG */
#endif /* BSP_USING_SDIO */

#ifdef __cplusplus
}
#endif

#endif /* _SDIO_CONFIG_H_ */
