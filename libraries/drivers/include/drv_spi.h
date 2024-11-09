/*
 * Copyright (c) 2006-2024 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-12-20     BruceOu      first implementation
 * 2024-01-10     Evlers       add dma supports
 * 2024-03-20     Evlers       add driver configure
 */

#ifndef __DRV_SPI_H__
#define __DRV_SPI_H__

#include <rthw.h>
#include <rtthread.h>
#include <board.h>
#include "drv_dma.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SPI_DMA_TIMEOUT_TIME            200     /* ms */
#define SPI_USING_RX_DMA_FLAG           (1<<0)
#define SPI_USING_TX_DMA_FLAG           (1<<1)

/* gd32 spi config class */
struct gd32_spi_config
{
    char *bus_name;
    uint32_t periph;
    rcu_periph_enum spi_clk;
    rcu_periph_enum gpio_clk;
    uint32_t spi_port;
#if defined SOC_SERIES_GD32F4xx
    uint32_t alt_func_num;
#endif
    uint16_t sck_pin;
    uint16_t miso_pin;
    uint16_t mosi_pin;
};

/* gd32 spi dirver class */
struct gd32_spi
{
    const struct gd32_spi_config *config;

    rt_uint8_t spi_dma_flag;
    struct
    {
        const struct dma_config *rx;
        const struct dma_config *tx;
        rt_sem_t rx_sem_ftf;
        rt_sem_t tx_sem_ftf;
    } dma;

    /* Save the spi transfer mode configured */
    uint32_t trans_mode;

    struct rt_spi_bus spi_bus;
};

rt_err_t rt_hw_spi_device_attach(const char *bus_name, const char *device_name, rt_base_t cs_pin);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_SPI_H__ */
