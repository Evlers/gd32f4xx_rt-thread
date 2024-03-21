/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-03-18   Evlers      first implementation
 * 2024-03-21   Evlers      add f10x/f20x/f30x series chip supports
 */

#ifndef _DRV_DMA_H_
#define _DRV_DMA_H_

#include "rtconfig.h"

#ifdef SOC_SERIES_GD32F4xx
#define DRV_DMA_CONFIG(dmax, chx, subx)     {                                                   \
                                                .periph     = DMA##dmax,                        \
                                                .channel    = DMA_CH##chx,                      \
                                                .rcu        = RCU_DMA##dmax,                    \
                                                .subperiph  = DMA_SUBPERI##subx,                \
                                                .irq        = DMA##dmax##_Channel##chx##_IRQn,  \
                                            }
#else
#define DRV_DMA_CONFIG(dmax, chx)           {                                                   \
                                                .periph     = DMA##dmax,                        \
                                                .channel    = DMA_CH##chx,                      \
                                                .rcu        = RCU_DMA##dmax,                    \
                                                .irq        = DMA##dmax##_Channel##chx##_IRQn,  \
                                            }
#endif

struct dma_config
{
    uint32_t periph;
    rcu_periph_enum rcu;
    dma_channel_enum channel;
#ifdef SOC_SERIES_GD32F4xx
    dma_subperipheral_enum subperiph;
#endif
    IRQn_Type irq;
};

#endif /* _DRV_DMA_H_ */
