/*
 * Copyright (c) 2006-2024 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date             Author          Notes
 * 2023-12-30       Evlers          first version
 * 2024-03-20       Evlers          add driver configure
 */

#ifndef __STM32_SDIO_H__
#define __STM32_SDIO_H__

#include <rthw.h>
#include <rtthread.h>
#include <board.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SDIO_ALIGN
#define SDIO_ALIGN                  (32)
#endif

#define HW_SDIO_ERRORS              (SDIO_INT_FLAG_CCRCERR | SDIO_INT_FLAG_CMDTMOUT | \
                                    SDIO_INT_FLAG_DTCRCERR | SDIO_INT_FLAG_DTTMOUT | \
                                    SDIO_INT_FLAG_RXORE  | SDIO_INT_FLAG_TXURE)


#define HW_SDIO_DATATIMEOUT         (0xFFFFFFFFU)

typedef rt_uint32_t (*sdio_clk_get)(uint32_t hw_sdio);

struct gd32_sdio_config
{
    uint32_t periph;
    uint32_t sdio_clock_freq;
    uint32_t sdio_max_freq;

    rcu_periph_enum sdio_rcu;
    IRQn_Type irqn;
    rcu_periph_enum clk_port_rcu;
    uint32_t ckl_port;
    uint16_t ckl_pin;

    rcu_periph_enum cmd_port_rcu;
    uint32_t cmd_port;
    uint16_t cmd_pin;

    rcu_periph_enum d0_port_rcu;
    uint32_t d0_port;
    uint16_t d0_pin;

    rcu_periph_enum d1_port_rcu;
    uint32_t d1_port;
    uint16_t d1_pin;

    rcu_periph_enum d2_port_rcu;
    uint32_t d2_port;
    uint16_t d2_pin;

    rcu_periph_enum d3_port_rcu;
    uint32_t d3_port;
    uint16_t d3_pin;

#if defined SOC_SERIES_GD32F4xx
    uint32_t alt_func_num;
#endif
};

struct gd32_sdio_des
{
    uint32_t hw_sdio;
    sdio_clk_get clk_get;
};

#ifdef __cplusplus
}
#endif

#endif
