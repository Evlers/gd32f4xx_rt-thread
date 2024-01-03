/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date             Author          Notes
 * 2023-12-18       Evlers          first version
 */

#include <stddef.h>
#include <rtthread.h>
#include <rthw.h>
#include <drivers/mmcsd_core.h>
#include <drivers/sdio.h>

#ifdef BSP_USING_SDIO

#include "drv_sdio.h"

static struct rt_mmcsd_host *host;

void SDIO_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    /* Process All SDIO Interrupt Sources */
    rthw_sdio_irq_process(host);

    /* leave interrupt */
    rt_interrupt_leave();
}

/*!
    \brief      configure the DMA1 channel 3 for transferring data
    \param[in]  srcbuf: a pointer point to a buffer which will be transferred
    \param[in]  bufsize: the size of buffer(not used in flow controller is peripheral)
    \param[out] none
    \retval     none
*/
static void dma_transfer_config(uint32_t *src, uint32_t *dst, uint32_t bufsize)
{
    dma_multi_data_parameter_struct dma_struct;
    /* clear all the interrupt flags */
    dma_flag_clear(SDIO_DMA, SDIO_DMA_CHANNEL, DMA_FLAG_FEE);
    dma_flag_clear(SDIO_DMA, SDIO_DMA_CHANNEL, DMA_FLAG_SDE);
    dma_flag_clear(SDIO_DMA, SDIO_DMA_CHANNEL, DMA_FLAG_TAE);
    dma_flag_clear(SDIO_DMA, SDIO_DMA_CHANNEL, DMA_FLAG_HTF);
    dma_flag_clear(SDIO_DMA, SDIO_DMA_CHANNEL, DMA_FLAG_FTF);
    dma_channel_disable(SDIO_DMA, SDIO_DMA_CHANNEL);
    dma_deinit(SDIO_DMA, SDIO_DMA_CHANNEL);

    /* configure the DMA1 channel 3 */
    dma_struct.periph_addr        = (uint32_t)dst;
    dma_struct.memory0_addr       = (uint32_t)src;
    dma_struct.direction          = DMA_MEMORY_TO_PERIPH;
    dma_struct.number             = bufsize;
    dma_struct.periph_inc         = DMA_PERIPH_INCREASE_DISABLE;
    dma_struct.memory_inc         = DMA_MEMORY_INCREASE_ENABLE;
    dma_struct.periph_width       = DMA_PERIPH_WIDTH_32BIT;
    dma_struct.memory_width       = DMA_MEMORY_WIDTH_32BIT;
    dma_struct.priority           = DMA_PRIORITY_ULTRA_HIGH;
    dma_struct.periph_burst_width = DMA_PERIPH_BURST_4_BEAT;
    dma_struct.memory_burst_width = DMA_MEMORY_BURST_4_BEAT;
    dma_struct.circular_mode      = DMA_CIRCULAR_MODE_DISABLE;
    dma_struct.critical_value     = DMA_FIFO_4_WORD;
    dma_multi_data_mode_init(SDIO_DMA, SDIO_DMA_CHANNEL, &dma_struct);

    dma_flow_controller_config(SDIO_DMA, SDIO_DMA_CHANNEL, DMA_FLOW_CONTROLLER_PERI);
    dma_channel_subperipheral_select(SDIO_DMA, SDIO_DMA_CHANNEL, DMA_SUBPERI4);

    dma_channel_enable(SDIO_DMA, SDIO_DMA_CHANNEL);
}

/*!
    \brief      configure the DMA1 channel 3 for receiving data
    \param[in]  dstbuf: a pointer point to a buffer which will receive data
    \param[in]  bufsize: the size of buffer(not used in flow controller is peripheral)
    \param[out] none
    \retval     none
*/
static void dma_receive_config(uint32_t *src, uint32_t *dst, uint32_t bufsize)
{
    dma_multi_data_parameter_struct dma_struct;
    /* clear all the interrupt flags */
    dma_flag_clear(SDIO_DMA, SDIO_DMA_CHANNEL, DMA_FLAG_FEE);
    dma_flag_clear(SDIO_DMA, SDIO_DMA_CHANNEL, DMA_FLAG_SDE);
    dma_flag_clear(SDIO_DMA, SDIO_DMA_CHANNEL, DMA_FLAG_TAE);
    dma_flag_clear(SDIO_DMA, SDIO_DMA_CHANNEL, DMA_FLAG_HTF);
    dma_flag_clear(SDIO_DMA, SDIO_DMA_CHANNEL, DMA_FLAG_FTF);
    dma_channel_disable(SDIO_DMA, SDIO_DMA_CHANNEL);
    dma_deinit(SDIO_DMA, SDIO_DMA_CHANNEL);

    /* configure the DMA1 channel 3 */
    dma_struct.periph_addr        = (uint32_t)src;
    dma_struct.memory0_addr       = (uint32_t)dst;
    dma_struct.direction          = DMA_PERIPH_TO_MEMORY;
    dma_struct.number             = bufsize;
    dma_struct.periph_inc         = DMA_PERIPH_INCREASE_DISABLE;
    dma_struct.memory_inc         = DMA_MEMORY_INCREASE_ENABLE;
    dma_struct.periph_width       = DMA_PERIPH_WIDTH_32BIT;
    dma_struct.memory_width       = DMA_MEMORY_WIDTH_32BIT;
    dma_struct.priority           = DMA_PRIORITY_ULTRA_HIGH;
    dma_struct.periph_burst_width = DMA_PERIPH_BURST_4_BEAT;
    dma_struct.memory_burst_width = DMA_MEMORY_BURST_4_BEAT;
    dma_struct.critical_value     = DMA_FIFO_4_WORD;
    dma_struct.circular_mode      = DMA_CIRCULAR_MODE_DISABLE;
    dma_multi_data_mode_init(SDIO_DMA, SDIO_DMA_CHANNEL, &dma_struct);

    dma_flow_controller_config(SDIO_DMA, SDIO_DMA_CHANNEL, DMA_FLOW_CONTROLLER_PERI);
    dma_channel_subperipheral_select(SDIO_DMA, SDIO_DMA_CHANNEL, DMA_SUBPERI4);
    dma_channel_enable(SDIO_DMA, SDIO_DMA_CHANNEL);
}

/*!
    \brief      configure the GPIO of SDIO interface
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void gpio_config(void)
{
    /* configure the SDIO_DAT0(PC8), SDIO_DAT1(PC9), SDIO_DAT2(PC10), SDIO_DAT3(PC11), SDIO_CLK(PC12) and SDIO_CMD(PD2) */
    gpio_af_set(SDIO_CLK_PORT, GPIO_AF_12, SDIO_CLK_PIN);
    gpio_af_set(SDIO_CMD_PORT, GPIO_AF_12, SDIO_CMD_PIN);
    gpio_af_set(SDIO_D0_PORT,  GPIO_AF_12, SDIO_D0_PIN);
    gpio_af_set(SDIO_D1_PORT,  GPIO_AF_12, SDIO_D1_PIN);
    gpio_af_set(SDIO_D2_PORT,  GPIO_AF_12, SDIO_D2_PIN);
    gpio_af_set(SDIO_D3_PORT,  GPIO_AF_12, SDIO_D3_PIN);

    gpio_mode_set(SDIO_CLK_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SDIO_CLK_PIN);
    gpio_output_options_set(SDIO_CLK_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, SDIO_CLK_PIN);

    gpio_mode_set(SDIO_CMD_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, SDIO_CMD_PIN);
    gpio_output_options_set(SDIO_CMD_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, SDIO_CMD_PIN);

    gpio_mode_set(SDIO_D0_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, SDIO_D0_PIN);
    gpio_output_options_set(SDIO_D0_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, SDIO_D0_PIN);

    gpio_mode_set(SDIO_D1_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, SDIO_D1_PIN);
    gpio_output_options_set(SDIO_D1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, SDIO_D1_PIN);

    gpio_mode_set(SDIO_D2_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, SDIO_D2_PIN);
    gpio_output_options_set(SDIO_D2_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, SDIO_D2_PIN);

    gpio_mode_set(SDIO_D3_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, SDIO_D3_PIN);
    gpio_output_options_set(SDIO_D3_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, SDIO_D3_PIN);
}

/*!
    \brief      configure the RCU of SDIO and DMA
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void rcu_config(void)
{
    rcu_periph_clock_enable(SDIO_GPIO_CLK);
    rcu_periph_clock_enable(SDIO_GPIO_CMD);
    rcu_periph_clock_enable(SDIO_GPIO_D0);
    rcu_periph_clock_enable(SDIO_GPIO_D1);
    rcu_periph_clock_enable(SDIO_GPIO_D2);
    rcu_periph_clock_enable(SDIO_GPIO_D3);

    rcu_periph_clock_enable(SDIO_PERI_CLOCK);
    rcu_periph_clock_enable(SDIO_DMA_CLOCK);
}

int gd32_sdio_init(void)
{
    struct gd32_sdio_des sdio_des;

    /* configure the RCU and GPIO, deinitialize the SDIO */
    rcu_config();
    gpio_config();
    sdio_deinit();

    /* configure the sdio interrupt */
    nvic_irq_enable(SDIO_IRQn, 0, 0);

    sdio_des.hw_sdio = SDIO_BASE;
    sdio_des.rxconfig = dma_receive_config;
    sdio_des.txconfig = dma_transfer_config;

    host = sdio_host_create(&sdio_des);
    if (host == RT_NULL)
    {
        rt_kprintf("%s host create fail\n");
        return -1;
    }

    return 0;
}
INIT_DEVICE_EXPORT(gd32_sdio_init);

#endif
