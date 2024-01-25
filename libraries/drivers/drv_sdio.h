/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date             Author          Notes
 * 2023-12-30       Evlers          first version
 */

#ifndef __STM32_SDIO_H__
#define __STM32_SDIO_H__

#include <board.h>

#if defined SOC_SERIES_GD32F10x
#include "gd32f10x_sdio.h"
#include "gd32f10x_dma.h"
#elif defined SOC_SERIES_GD32F20x
#include "gd32f20x_sdio.h"
#include "gd32f20x_dma.h"
#elif defined SOC_SERIES_GD32F30x
#include "gd32f30x_sdio.h"
#include "gd32f30x_dma.h"
#elif defined SOC_SERIES_GD32F4xx
#include "gd32f4xx_sdio.h"
#include "gd32f4xx_dma.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define SDIO_PERI_CLOCK             RCU_SDIO
#define SDIO_GPIO_CLK               RCU_GPIOC
#define SDIO_GPIO_CMD               RCU_GPIOD
#define SDIO_GPIO_D0                RCU_GPIOC
#define SDIO_GPIO_D1                RCU_GPIOC
#define SDIO_GPIO_D2                RCU_GPIOC
#define SDIO_GPIO_D3                RCU_GPIOC

#define SDIO_CLK_PORT               GPIOC
#define SDIO_CLK_PIN                GPIO_PIN_12
#define SDIO_CMD_PORT               GPIOD
#define SDIO_CMD_PIN                GPIO_PIN_2
#define SDIO_D0_PORT                GPIOC
#define SDIO_D0_PIN                 GPIO_PIN_8
#define SDIO_D1_PORT                GPIOC
#define SDIO_D1_PIN                 GPIO_PIN_9
#define SDIO_D2_PORT                GPIOC
#define SDIO_D2_PIN                 GPIO_PIN_10
#define SDIO_D3_PORT                GPIOC
#define SDIO_D3_PIN                 GPIO_PIN_11

#define SDIO_DMA                    DMA1
#define SDIO_DMA_CLOCK              RCU_DMA1
#define SDIO_DMA_CHANNEL            DMA_CH3
#define SDIO_DMA_IRQ                DMA1_Channel3_IRQn
#define SDIO_DMA_IRQ_HANDLER        DMA1_Channel3_IRQHandler
#define SDIO_DMA_SUBPERI            DMA_SUBPERI4

#ifndef SDIO_BASE_ADDRESS
#define SDIO_BASE_ADDRESS           (SDIO_BASE)
#endif

#ifndef SDIO_CLOCK_FREQ
#define SDIO_CLOCK_FREQ             (48U * 1000 * 1000)
#endif

#ifndef SDIO_BUFF_SIZE
#define SDIO_BUFF_SIZE              (4096)
#endif

#ifndef SDIO_ALIGN
#define SDIO_ALIGN                  (32)
#endif

#ifndef SDIO_MAX_FREQ
#define SDIO_MAX_FREQ               (24 * 1000 * 1000)
#endif


#define HW_SDIO_ERRORS              (SDIO_INT_FLAG_CCRCERR | SDIO_INT_FLAG_CMDTMOUT | \
                                    SDIO_INT_FLAG_DTCRCERR | SDIO_INT_FLAG_DTTMOUT | \
                                    SDIO_INT_FLAG_RXORE  | SDIO_INT_FLAG_TXURE)


#define HW_SDIO_DATATIMEOUT         (0xFFFFFFFFU)


typedef void (*dma_txconfig)(uint32_t *src, uint32_t *dst, uint32_t size);
typedef void (*dma_rxconfig)(uint32_t *src, uint32_t *dst, uint32_t size);
typedef rt_uint32_t (*sdio_clk_get)(uint32_t hw_sdio);

struct gd32_sdio_des
{
    uint32_t hw_sdio;
    sdio_clk_get clk_get;
};

#ifdef __cplusplus
}
#endif

#endif
