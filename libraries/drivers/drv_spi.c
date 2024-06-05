/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-12-20     BruceOu      first implementation
 * 2023-12-12     Evlers       update the io write of cs
 * 2024-01-10     Evlers       add dma supports
 * 2024-01-22     Evlers       add support for half duplex (3-wire mode)
 * 2024-03-20     Evlers       add driver configure
 * 2024-03-21     Evlers       add msp layer supports
 * 2024-06-04     Evlers       use the new cs pin specification
 * 2024-06-05     Evlers       fix an issue where unknown data was received when dma rx was used only
 */

#include "drv_spi.h"
#include "drv_config.h"

#ifdef RT_USING_SPI

#if defined(BSP_USING_SPI0) || defined(BSP_USING_SPI1) || defined(BSP_USING_SPI2) || defined(BSP_USING_SPI3) || defined(BSP_USING_SPI4)
#define DBG_LVL              DBG_INFO
#define LOG_TAG              "drv.spi"

#include <rtdbg.h>

enum {
#ifdef BSP_USING_SPI0
    SPI0_INDEX,
#endif
#ifdef BSP_USING_SPI1
    SPI1_INDEX,
#endif
#ifdef BSP_USING_SPI2
    SPI2_INDEX,
#endif
#ifdef BSP_USING_SPI3
    SPI3_INDEX,
#endif
#ifdef BSP_USING_SPI4
    SPI4_INDEX,
#endif
};

static const struct gd32_spi_config spi_config[] = {
#ifdef BSP_USING_SPI0
    SPI0_CONFIG,
#endif

#ifdef BSP_USING_SPI1
    SPI1_CONFIG,
#endif

#ifdef BSP_USING_SPI2
    SPI2_CONFIG,
#endif

#ifdef BSP_USING_SPI3
    SPI3_CONFIG,
#endif

#ifdef BSP_USING_SPI4
    SPI4_CONFIG,
#endif
};

static struct gd32_spi spi_bus_obj[sizeof(spi_config) / sizeof(spi_config[0])] = { 0 };

#if defined(BSP_SPI0_RX_USING_DMA) || \
    defined(BSP_SPI1_RX_USING_DMA) || \
    defined(BSP_SPI2_RX_USING_DMA) || \
    defined(BSP_SPI3_RX_USING_DMA) || \
    defined(BSP_SPI4_RX_USING_DMA)

static void dma_rx_isr (struct gd32_spi *spi_bus)
{
    if (dma_interrupt_flag_get(spi_bus->dma.rx->periph, spi_bus->dma.rx->channel, DMA_INT_FLAG_FTF))
    {
        dma_interrupt_flag_clear(spi_bus->dma.rx->periph, spi_bus->dma.rx->channel, DMA_INT_FLAG_FTF);
        if (spi_bus->dma.rx_sem_ftf != NULL)
        {
            /* If in half-duplex mode, stop the Rx clock */
            if (spi_bus->trans_mode == SPI_TRANSMODE_BDTRANSMIT)
            {
                /* Set Tx mode to stop the Rx clock */
                spi_bidirectional_transfer_config(spi_bus->config->periph, SPI_BIDIRECTIONAL_TRANSMIT);
            }
            rt_interrupt_enter();
            rt_sem_release(spi_bus->dma.rx_sem_ftf);
            rt_interrupt_leave();
            return ;
        }
    }
}

#endif

#if defined(BSP_SPI0_TX_USING_DMA) || \
    defined(BSP_SPI1_TX_USING_DMA) || \
    defined(BSP_SPI2_TX_USING_DMA) || \
    defined(BSP_SPI3_TX_USING_DMA) || \
    defined(BSP_SPI4_TX_USING_DMA)

static void dma_tx_isr (struct gd32_spi *spi_bus)
{
    if (dma_interrupt_flag_get(spi_bus->dma.tx->periph, spi_bus->dma.tx->channel, DMA_INT_FLAG_FTF))
    {
        dma_interrupt_flag_clear(spi_bus->dma.tx->periph, spi_bus->dma.tx->channel, DMA_INT_FLAG_FTF);
        if (spi_bus->dma.tx_sem_ftf != NULL)
        {
            rt_interrupt_enter();
            rt_sem_release(spi_bus->dma.tx_sem_ftf);
            rt_interrupt_leave();
            return ;
        }
    }
}

#endif

#ifdef BSP_SPI0_RX_USING_DMA
void SPI0_DMA_RX_IRQHandler (void)
{
    dma_rx_isr(&spi_bus_obj[SPI0_INDEX]);
}
#endif

#ifdef BSP_SPI0_TX_USING_DMA
void SPI0_DMA_TX_IRQHandler (void)
{
    dma_tx_isr(&spi_bus_obj[SPI0_INDEX]);
}
#endif

#ifdef BSP_SPI1_RX_USING_DMA
void SPI1_DMA_RX_IRQHandler (void)
{
    dma_rx_isr(&spi_bus_obj[SPI1_INDEX]);
}
#endif

#ifdef BSP_SPI1_TX_USING_DMA
void SPI1_DMA_TX_IRQHandler (void)
{
    dma_tx_isr(&spi_bus_obj[SPI1_INDEX]);
}
#endif

#ifdef BSP_SPI2_RX_USING_DMA
void SPI2_DMA_RX_IRQHandler (void)
{
    dma_rx_isr(&spi_bus_obj[SPI2_INDEX]);
}
#endif

#ifdef BSP_SPI2_TX_USING_DMA
void SPI2_DMA_TX_IRQHandler (void)
{
    dma_tx_isr(&spi_bus_obj[SPI2_INDEX]);
}
#endif

#ifdef BSP_SPI3_RX_USING_DMA
void SPI3_DMA_RX_IRQHandler (void)
{
    dma_rx_isr(&spi_bus_obj[SPI3_INDEX]);
}
#endif

#ifdef BSP_SPI3_TX_USING_DMA
void SPI3_DMA_TX_IRQHandler (void)
{
    dma_tx_isr(&spi_bus_obj[SPI3_INDEX]);
}
#endif

#ifdef BSP_SPI4_RX_USING_DMA
void SPI4_DMA_RX_IRQHandler (void)
{
    dma_rx_isr(&spi_bus_obj[SPI4_INDEX]);
}
#endif

#ifdef BSP_SPI4_TX_USING_DMA
void SPI4_DMA_TX_IRQHandler (void)
{
    dma_tx_isr(&spi_bus_obj[SPI4_INDEX]);
}
#endif

static void gd32_spi_dma_config (struct gd32_spi *gd32_spi, rt_uint8_t data_width)
{
    dma_single_data_parameter_struct dma_init_struct = { 0 };

    dma_init_struct.periph_addr         = (uint32_t)&SPI_DATA(gd32_spi->config->periph);
    dma_init_struct.periph_memory_width = data_width <= 8 ? DMA_PERIPH_WIDTH_8BIT : DMA_PERIPH_WIDTH_16BIT;
    dma_init_struct.periph_inc          = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.memory_inc          = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.circular_mode       = DMA_CIRCULAR_MODE_DISABLE;

    if (gd32_spi->spi_dma_flag & SPI_USING_RX_DMA_FLAG)
    {
        /* clear all the interrupt flags */
        dma_flag_clear(gd32_spi->dma.rx->periph, gd32_spi->dma.rx->channel, DMA_FLAG_FEE);
        dma_flag_clear(gd32_spi->dma.rx->periph, gd32_spi->dma.rx->channel, DMA_FLAG_SDE);
        dma_flag_clear(gd32_spi->dma.rx->periph, gd32_spi->dma.rx->channel, DMA_FLAG_TAE);
        dma_flag_clear(gd32_spi->dma.rx->periph, gd32_spi->dma.rx->channel, DMA_FLAG_HTF);
        dma_flag_clear(gd32_spi->dma.rx->periph, gd32_spi->dma.rx->channel, DMA_FLAG_FTF);
        dma_channel_disable(gd32_spi->dma.rx->periph, gd32_spi->dma.rx->channel);
        dma_deinit(gd32_spi->dma.rx->periph, gd32_spi->dma.rx->channel);

        /* configure SPI receive DMA */
        rcu_periph_clock_enable(gd32_spi->dma.rx->rcu);

        dma_deinit(gd32_spi->dma.rx->periph, gd32_spi->dma.rx->channel);
        dma_init_struct.direction           = DMA_PERIPH_TO_MEMORY;
        dma_init_struct.priority            = DMA_PRIORITY_HIGH;
        dma_single_data_mode_init(gd32_spi->dma.rx->periph, gd32_spi->dma.rx->channel, &dma_init_struct);
        dma_channel_subperipheral_select(gd32_spi->dma.rx->periph, gd32_spi->dma.rx->channel, gd32_spi->dma.rx->subperiph);

        if (gd32_spi->dma.rx->irq != NULL)
        {
            NVIC_EnableIRQ(gd32_spi->dma.rx->irq);
            dma_interrupt_enable(gd32_spi->dma.rx->periph, gd32_spi->dma.rx->channel, DMA_CHXCTL_FTFIE);
        }

        dma_channel_disable(gd32_spi->dma.rx->periph, gd32_spi->dma.rx->channel);

        if (gd32_spi->dma.rx_sem_ftf == NULL)
        {
            gd32_spi->dma.rx_sem_ftf = rt_sem_create("spi dma rx", 0, RT_IPC_FLAG_PRIO);
            RT_ASSERT(gd32_spi->dma.rx_sem_ftf);
        }
    }

    if (gd32_spi->spi_dma_flag & SPI_USING_TX_DMA_FLAG)
    {
        /* clear all the interrupt flags */
        dma_flag_clear(gd32_spi->dma.tx->periph, gd32_spi->dma.tx->channel, DMA_FLAG_FEE);
        dma_flag_clear(gd32_spi->dma.tx->periph, gd32_spi->dma.tx->channel, DMA_FLAG_SDE);
        dma_flag_clear(gd32_spi->dma.tx->periph, gd32_spi->dma.tx->channel, DMA_FLAG_TAE);
        dma_flag_clear(gd32_spi->dma.tx->periph, gd32_spi->dma.tx->channel, DMA_FLAG_HTF);
        dma_flag_clear(gd32_spi->dma.tx->periph, gd32_spi->dma.tx->channel, DMA_FLAG_FTF);
        dma_channel_disable(gd32_spi->dma.tx->periph, gd32_spi->dma.tx->channel);
        dma_deinit(gd32_spi->dma.tx->periph, gd32_spi->dma.tx->channel);

        /* configure SPI transmit DMA */
        rcu_periph_clock_enable(gd32_spi->dma.tx->rcu);

        dma_deinit(gd32_spi->dma.tx->periph, gd32_spi->dma.tx->channel);
        dma_init_struct.direction           = DMA_MEMORY_TO_PERIPH;
        dma_init_struct.priority            = DMA_PRIORITY_LOW;
        dma_single_data_mode_init(gd32_spi->dma.tx->periph, gd32_spi->dma.tx->channel, &dma_init_struct);
        dma_channel_subperipheral_select(gd32_spi->dma.tx->periph, gd32_spi->dma.tx->channel, gd32_spi->dma.tx->subperiph);

        if (gd32_spi->dma.tx->irq != NULL)
        {
            NVIC_EnableIRQ(gd32_spi->dma.tx->irq);
            dma_interrupt_enable(gd32_spi->dma.tx->periph, gd32_spi->dma.tx->channel, DMA_CHXCTL_FTFIE);
        }

        dma_channel_disable(gd32_spi->dma.tx->periph, gd32_spi->dma.tx->channel);

        if (gd32_spi->dma.tx_sem_ftf == NULL)
        {
            gd32_spi->dma.tx_sem_ftf = rt_sem_create("spi dma tx", 0, RT_IPC_FLAG_PRIO);
            RT_ASSERT(gd32_spi->dma.tx_sem_ftf);
        }
    }
}

static void spi_dma_exchange (struct rt_spi_device* device, struct rt_spi_message* message)
{
    struct rt_spi_bus *gd32_spi_bus = (struct rt_spi_bus *)device->bus;
    struct gd32_spi *spi_device = (struct gd32_spi *)gd32_spi_bus->parent.user_data;

    LOG_D("spi dma transfer start: %d\n", size);

    if (message->send_buf && message->recv_buf)
    {
        /* Set the data length and data pointer */
        DMA_CHM0ADDR(spi_device->dma.rx->periph, spi_device->dma.rx->channel) = (uint32_t)message->recv_buf;
        DMA_CHM0ADDR(spi_device->dma.tx->periph, spi_device->dma.tx->channel) = (uint32_t)message->send_buf;
        DMA_CHCNT(spi_device->dma.rx->periph, spi_device->dma.rx->channel) = message->length;
        DMA_CHCNT(spi_device->dma.tx->periph, spi_device->dma.tx->channel) = message->length;

        /* Enable DMA transfer */
        dma_channel_enable(spi_device->dma.rx->periph, spi_device->dma.rx->channel);
        dma_channel_enable(spi_device->dma.tx->periph, spi_device->dma.tx->channel);
        spi_dma_enable(spi_device->config->periph, SPI_DMA_RECEIVE);
        spi_dma_enable(spi_device->config->periph, SPI_DMA_TRANSMIT);

        /* Wait for transmission to complete */
        rt_sem_take(spi_device->dma.rx_sem_ftf, rt_tick_from_millisecond(SPI_DMA_TIMEOUT_TIME));
        rt_sem_take(spi_device->dma.tx_sem_ftf, rt_tick_from_millisecond(SPI_DMA_TIMEOUT_TIME));

        /* Disable dma and spi */
        dma_channel_disable(spi_device->dma.rx->periph, spi_device->dma.rx->channel);
        dma_channel_disable(spi_device->dma.tx->periph, spi_device->dma.tx->channel);
        spi_dma_disable(spi_device->config->periph, SPI_DMA_RECEIVE);
        spi_dma_disable(spi_device->config->periph, SPI_DMA_TRANSMIT);
    }
    else if (message->send_buf)
    {
        /* Set the data length and data pointer */
        DMA_CHM0ADDR(spi_device->dma.tx->periph, spi_device->dma.tx->channel) = (uint32_t)message->send_buf;
        DMA_CHCNT(spi_device->dma.tx->periph, spi_device->dma.tx->channel) = message->length;

        /* Enable DMA transfer */
        dma_channel_enable(spi_device->dma.tx->periph, spi_device->dma.tx->channel);
        spi_dma_enable(spi_device->config->periph, SPI_DMA_TRANSMIT);

        /* Wait for transmission to complete */
        rt_sem_take(spi_device->dma.tx_sem_ftf, rt_tick_from_millisecond(SPI_DMA_TIMEOUT_TIME));

        /* Wait for the last data transfer to complete */
        while (RESET != spi_i2s_flag_get(spi_device->config->periph, SPI_FLAG_TRANS));

        /* Disable dma and spi */
        dma_channel_disable(spi_device->dma.tx->periph, spi_device->dma.tx->channel);
        spi_dma_disable(spi_device->config->periph, SPI_DMA_TRANSMIT);
    }
    else
    {
        uint16_t data = ~0;

        /* Clean rx buffer */
        spi_i2s_data_receive(spi_device->config->periph);

        /* Set the data length and data pointer */
        DMA_CHM0ADDR(spi_device->dma.rx->periph, spi_device->dma.rx->channel) = (uint32_t)message->recv_buf;
        DMA_CHCNT(spi_device->dma.rx->periph, spi_device->dma.rx->channel) = message->length;

        /* Enable DMA transfer */
        dma_channel_enable(spi_device->dma.rx->periph, spi_device->dma.rx->channel);
        spi_dma_enable(spi_device->config->periph, SPI_DMA_RECEIVE);

        /* In full-duplex mode, you need to configure transmission dma */
        if (spi_device->trans_mode == SPI_TRANSMODE_FULLDUPLEX)
        {
            /* Set the data length and data pointer */
            DMA_CHM0ADDR(spi_device->dma.tx->periph, spi_device->dma.tx->channel) = (uint32_t)(&data);
            DMA_CHCNT(spi_device->dma.tx->periph, spi_device->dma.tx->channel) = message->length;
            dma_memory_address_generation_config(spi_device->dma.tx->periph, spi_device->dma.tx->channel, DMA_MEMORY_INCREASE_DISABLE);

            /* Enable DMA transfer */
            dma_channel_enable(spi_device->dma.tx->periph, spi_device->dma.tx->channel);
            spi_dma_enable(spi_device->config->periph, SPI_DMA_TRANSMIT);

            /* Wait for receive to complete */
            rt_sem_take(spi_device->dma.tx_sem_ftf, rt_tick_from_millisecond(SPI_DMA_TIMEOUT_TIME));
        }
        else if (spi_device->trans_mode == SPI_TRANSMODE_BDTRANSMIT)
        {
            /* Set Rx mode to start the Rx clock */
            spi_bidirectional_transfer_config(spi_device->config->periph, SPI_BIDIRECTIONAL_RECEIVE);
        }

        /* Wait for transmission to complete */
        rt_sem_take(spi_device->dma.rx_sem_ftf, rt_tick_from_millisecond(SPI_DMA_TIMEOUT_TIME));

        /* Disable dma and spi */
        dma_channel_disable(spi_device->dma.rx->periph, spi_device->dma.rx->channel);
        spi_dma_disable(spi_device->config->periph, SPI_DMA_RECEIVE);

        if (spi_device->trans_mode == SPI_TRANSMODE_FULLDUPLEX)
        {
            dma_channel_disable(spi_device->dma.tx->periph, spi_device->dma.tx->channel);
            spi_dma_disable(spi_device->config->periph, SPI_DMA_TRANSMIT);
            dma_memory_address_generation_config(spi_device->dma.tx->periph, spi_device->dma.tx->channel, DMA_MEMORY_INCREASE_ENABLE);
        }
    }

    LOG_D("spi dma transfer finsh\n");
}

static void spi_exchange (struct rt_spi_device* device, struct rt_spi_message* message)
{
    struct rt_spi_bus * gd32_spi_bus = (struct rt_spi_bus *)device->bus;
    struct gd32_spi *spi_device = (struct gd32_spi *)gd32_spi_bus->parent.user_data;
    struct rt_spi_configuration * config = &device->config;
    uint32_t spi_periph = spi_device->config->periph;

    LOG_D("spi poll transfer start: %d\n", size);

    /* Set the transmission direction of half duplex */
    if (spi_device->trans_mode == SPI_TRANSMODE_BDTRANSMIT && message->recv_buf != RT_NULL)
    {
        /* Clear receive buffer */
        while (RESET != spi_i2s_flag_get(spi_periph, SPI_FLAG_RBNE))
        {
            uint16_t data = spi_i2s_data_receive(spi_periph);
        }

        /* Set Rx mode to start the Rx clock */
        spi_bidirectional_transfer_config(spi_periph, SPI_BIDIRECTIONAL_RECEIVE);
    }

    if (config->data_width <= 8)
    {
        const rt_uint8_t *send_ptr = message->send_buf;
        rt_uint8_t *recv_ptr = message->recv_buf;
        rt_uint32_t size = message->length;

        while (size--)
        {
            rt_uint8_t data = ~0;

            if (send_ptr != RT_NULL)
            {
                data = *send_ptr++;
            }

            /* Todo: replace register read/write by gd32f4 lib */
            if ((spi_device->trans_mode == SPI_TRANSMODE_FULLDUPLEX) ||
                (spi_device->trans_mode == SPI_TRANSMODE_BDTRANSMIT && message->send_buf != RT_NULL))
            {
                /* Send the byte */
                spi_i2s_data_transmit(spi_periph, data);
                /* Wait until the transmit buffer is empty */
                while (RESET == spi_i2s_flag_get(spi_periph, SPI_FLAG_TBE));
            }

            if ((spi_device->trans_mode == SPI_TRANSMODE_FULLDUPLEX) ||
                (spi_device->trans_mode == SPI_TRANSMODE_BDTRANSMIT && message->recv_buf != RT_NULL))
            {
                /* Wait until a data is received */
                while (RESET == spi_i2s_flag_get(spi_periph, SPI_FLAG_RBNE));
                /* Get the received data */
                data = spi_i2s_data_receive(spi_periph);
            }

            if (recv_ptr != RT_NULL)
            {
                *recv_ptr++ = data;
            }
        }
    }
    else if (config->data_width <= 16)
    {
        const rt_uint16_t * send_ptr = message->send_buf;
        rt_uint16_t * recv_ptr = message->recv_buf;
        rt_uint32_t size = message->length;

        while (size--)
        {
            rt_uint16_t data = ~0;

            if (send_ptr != RT_NULL)
            {
                data = *send_ptr++;
            }

            if ((spi_device->trans_mode == SPI_TRANSMODE_FULLDUPLEX) ||
                (spi_device->trans_mode == SPI_TRANSMODE_BDTRANSMIT && message->send_buf != RT_NULL))
            {
                /* Send the short */
                spi_i2s_data_transmit(spi_periph, data);
                /* Wait until the transmit buffer is empty */
                while (RESET == spi_i2s_flag_get(spi_periph, SPI_FLAG_TBE));
            }

            if ((spi_device->trans_mode == SPI_TRANSMODE_FULLDUPLEX) ||
                (spi_device->trans_mode == SPI_TRANSMODE_BDTRANSMIT && message->recv_buf != RT_NULL))
            {
                /* Wait until a data is received */
                while (RESET == spi_i2s_flag_get(spi_periph, SPI_FLAG_RBNE));
                /* Get the received data */
                data = spi_i2s_data_receive(spi_periph);
            }

            if (recv_ptr != RT_NULL)
            {
                *recv_ptr++ = data;
            }
        }
    }

    if (spi_device->trans_mode == SPI_TRANSMODE_BDTRANSMIT && message->recv_buf != RT_NULL)
    {
        /* Set Tx mode to stop the Rx clock */
        spi_bidirectional_transfer_config(spi_periph, SPI_BIDIRECTIONAL_TRANSMIT);
    }

    /* Wait for the last data transfer to complete */
    while (RESET != spi_i2s_flag_get(spi_periph, SPI_FLAG_TRANS));

    LOG_D("spi poll transfer finsh\n");
}

/**
 * @brief SPI MSP Initialization
 *        This function configures the hardware resources used in this example:
 *           - Peripheral's GPIO Configuration
 *           - NVIC configuration for interrupt priority
 *        This function belongs to weak function, users can rewrite this function according to different needs
 *
 * @param periph peripherals in gd32_spi_config
 * @return None
 */
rt_weak void gd32_msp_spi_init (const uint32_t *periph)
{
    struct gd32_spi_config *config = rt_container_of(periph, struct gd32_spi_config, periph);

    /* enable gpio clock */
    rcu_periph_clock_enable(config->gpio_clk);

    /* GPIO pin configuration */
#if defined SOC_SERIES_GD32F4xx
    gpio_af_set(config->spi_port, config->alt_func_num, config->sck_pin | config->mosi_pin);

    gpio_mode_set(config->spi_port, GPIO_MODE_AF, GPIO_PUPD_NONE, config->sck_pin | config->mosi_pin);
    gpio_output_options_set(config->spi_port, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, config->sck_pin | config->mosi_pin);

    gpio_af_set(config->spi_port, config->alt_func_num, config->miso_pin);
    gpio_mode_set(config->spi_port, GPIO_MODE_AF, GPIO_PUPD_NONE, config->miso_pin);
    gpio_output_options_set(config->spi_port, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, config->miso_pin);
#else
    gpio_init(config->spi_port, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, config->sck_pin | config->mosi_pin);
    gpio_init(config->spi_port, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_MAX, config->miso_pin);
#endif
}

static rt_err_t spi_configure (struct rt_spi_device* device, struct rt_spi_configuration* configuration)
{
    struct rt_spi_bus * spi_bus = (struct rt_spi_bus *)device->bus;
    struct gd32_spi *spi_device = (struct gd32_spi *)spi_bus->parent.user_data;
    spi_parameter_struct spi_init_struct;
    uint32_t spi_periph = spi_device->config->periph;

    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(configuration != RT_NULL);

    gd32_msp_spi_init(&spi_device->config->periph);

    /* configure spi clock */
    rcu_periph_clock_enable(spi_device->config->spi_clk);

    /* init DMA */
    if (spi_device->spi_dma_flag)
    {
        gd32_spi_dma_config(spi_device, configuration->data_width);
    }

    /* data_width */
    if(configuration->data_width <= 8)
    {
        spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
    }
    else if(configuration->data_width <= 16)
    {
        spi_init_struct.frame_size = SPI_FRAMESIZE_16BIT;
    }
    else
    {
        return -RT_EIO;
    }

    /* baudrate */
    {
        rcu_clock_freq_enum spi_src;
        uint32_t spi_apb_clock;
        uint32_t max_hz;

        max_hz = configuration->max_hz;

        LOG_D("sys   freq: %d\n", rcu_clock_freq_get(CK_SYS));
        LOG_D("CK_APB2 freq: %d\n", rcu_clock_freq_get(CK_APB2));
        LOG_D("max   freq: %d\n", max_hz);

        if (spi_periph == SPI1 || spi_periph == SPI2)
        {
            spi_src = CK_APB1;
        }
        else
        {
            spi_src = CK_APB2;
        }
        spi_apb_clock = rcu_clock_freq_get(spi_src);

        if(max_hz >= spi_apb_clock/2)
        {
            spi_init_struct.prescale = SPI_PSC_2;
        }
        else if (max_hz >= spi_apb_clock/4)
        {
            spi_init_struct.prescale = SPI_PSC_4;
        }
        else if (max_hz >= spi_apb_clock/8)
        {
            spi_init_struct.prescale = SPI_PSC_8;
        }
        else if (max_hz >= spi_apb_clock/16)
        {
            spi_init_struct.prescale = SPI_PSC_16;
        }
        else if (max_hz >= spi_apb_clock/32)
        {
            spi_init_struct.prescale = SPI_PSC_32;
        }
        else if (max_hz >= spi_apb_clock/64)
        {
            spi_init_struct.prescale = SPI_PSC_64;
        }
        else if (max_hz >= spi_apb_clock/128)
        {
            spi_init_struct.prescale = SPI_PSC_128;
        }
        else
        {
            /*  min prescaler 256 */
            spi_init_struct.prescale = SPI_PSC_256;
        }
    } /* baudrate */

    switch(configuration->mode & RT_SPI_MODE_3)
    {
    case RT_SPI_MODE_0:
        spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
        break;
    case RT_SPI_MODE_1:
        spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_2EDGE;
        break;
    case RT_SPI_MODE_2:
        spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_1EDGE;
        break;
    case RT_SPI_MODE_3:
        spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
        break;
    }

    /* MSB or LSB */
    if(configuration->mode & RT_SPI_MSB)
    {
        spi_init_struct.endian = SPI_ENDIAN_MSB;
    }
    else
    {
        spi_init_struct.endian = SPI_ENDIAN_LSB;
    }

    /* Full duplex or half duplex */
    if (configuration->mode & RT_SPI_3WIRE)
    {
        spi_init_struct.trans_mode = SPI_TRANSMODE_BDTRANSMIT;
    }
    else
    {
        spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    }

    spi_init_struct.device_mode = SPI_MASTER;
    spi_init_struct.nss = SPI_NSS_SOFT;
    spi_device->trans_mode = spi_init_struct.trans_mode;

    spi_crc_off(spi_periph);

    /* init SPI */
    spi_init(spi_periph, &spi_init_struct);
    /* Enable SPI_MASTER */
    spi_enable(spi_periph);

    return RT_EOK;
};

static rt_ssize_t spixfer (struct rt_spi_device* device, struct rt_spi_message* message)
{
    struct rt_spi_bus * gd32_spi_bus = (struct rt_spi_bus *)device->bus;
    struct gd32_spi *spi_device = (struct gd32_spi *)gd32_spi_bus->parent.user_data;

    RT_ASSERT(device != NULL);
    RT_ASSERT(message != NULL);

    /* take CS */
    if(message->cs_take)
    {
        rt_pin_write(device->cs_pin, PIN_LOW);
        LOG_D("spi take cs\n");
    }

    if (message->send_buf && message->recv_buf)
    {
        /* Data can be exchanged only in full duplex mode */
        RT_ASSERT(spi_device->trans_mode == SPI_TRANSMODE_FULLDUPLEX);

        if ((spi_device->spi_dma_flag & SPI_USING_RX_DMA_FLAG) && (spi_device->spi_dma_flag & SPI_USING_TX_DMA_FLAG))
        {
            spi_dma_exchange(device, message);
        }
        else
        {
            spi_exchange(device, message);
        }
    }
    else if (message->send_buf)
    {
        if (spi_device->spi_dma_flag & SPI_USING_TX_DMA_FLAG)
        {
            spi_dma_exchange(device, message);
        }
        else
        {
            spi_exchange(device, message);
        }
    }
    else
    {
        if (spi_device->spi_dma_flag & SPI_USING_RX_DMA_FLAG)
        {
            spi_dma_exchange(device, message);
        }
        else
        {
            spi_exchange(device, message);
        }
    }

    /* release CS */
    if(message->cs_release)
    {
        rt_pin_write(device->cs_pin, PIN_HIGH);
        LOG_D("spi release cs\n");
    }

    return message->length;
};

static struct rt_spi_ops gd32_spi_ops =
{
    .configure = spi_configure,
    .xfer = spixfer,
};

/**
  * Attach the spi device to SPI bus, this function must be used after initialization.
  */
rt_err_t rt_hw_spi_device_attach (const char *bus_name, const char *device_name, rt_base_t cs_pin)
{
    RT_ASSERT(bus_name != RT_NULL);
    RT_ASSERT(device_name != RT_NULL);

    rt_err_t result;
    struct rt_spi_device *spi_device;

    /* attach the device to spi bus*/
    spi_device = (struct rt_spi_device *)rt_malloc(sizeof(struct rt_spi_device));
    RT_ASSERT(spi_device != RT_NULL);

    if(cs_pin != PIN_NONE)
    {
        /* initialize the cs pin && select the slave*/
        rt_pin_mode(cs_pin, PIN_MODE_OUTPUT);
        rt_pin_write(cs_pin, PIN_HIGH);
    }

    result = rt_spi_bus_attach_device_cspin(spi_device, device_name, bus_name, cs_pin, NULL);

    if (result != RT_EOK)
    {
        LOG_E("%s attach to %s faild, %d\n", device_name, bus_name, result);
    }

    RT_ASSERT(result == RT_EOK);

    LOG_D("%s attach to %s done", device_name, bus_name);

    return result;
}

static void gd32_get_dma_info (void)
{
#ifdef BSP_SPI0_RX_USING_DMA
    spi_bus_obj[SPI0_INDEX].spi_dma_flag |= SPI_USING_RX_DMA_FLAG;
    static const struct dma_config spi0_dma_rx = SPI0_RX_DMA_CONFIG;
    spi_bus_obj[SPI0_INDEX].dma.rx = &spi0_dma_rx;
#endif

#ifdef BSP_SPI0_TX_USING_DMA
    spi_bus_obj[SPI0_INDEX].spi_dma_flag |= SPI_USING_TX_DMA_FLAG;
    static const struct dma_config spi0_dma_tx = SPI0_TX_DMA_CONFIG;
    spi_bus_obj[SPI0_INDEX].dma.tx = &spi0_dma_tx;
#endif

#ifdef BSP_SPI1_RX_USING_DMA
    spi_bus_obj[SPI1_INDEX].spi_dma_flag |= SPI_USING_RX_DMA_FLAG;
    static const struct dma_config spi1_dma_rx = SPI1_RX_DMA_CONFIG;
    spi_bus_obj[SPI1_INDEX].dma.rx = &spi1_dma_rx;
#endif

#ifdef BSP_SPI1_TX_USING_DMA
    spi_bus_obj[SPI1_INDEX].spi_dma_flag |= SPI_USING_TX_DMA_FLAG;
    static const struct dma_config spi1_dma_tx = SPI1_TX_DMA_CONFIG;
    spi_bus_obj[SPI1_INDEX].dma.tx = &spi1_dma_tx;
#endif

#ifdef BSP_SPI2_RX_USING_DMA
    spi_bus_obj[SPI2_INDEX].spi_dma_flag |= SPI_USING_RX_DMA_FLAG;
    static const struct dma_config spi2_dma_rx = SPI2_RX_DMA_CONFIG;
    spi_bus_obj[SPI2_INDEX].dma.rx = &spi2_dma_rx;
#endif

#ifdef BSP_SPI2_TX_USING_DMA
    spi_bus_obj[SPI2_INDEX].spi_dma_flag |= SPI_USING_TX_DMA_FLAG;
    static const struct dma_config spi2_dma_tx = SPI2_TX_DMA_CONFIG;
    spi_bus_obj[SPI2_INDEX].dma.tx = &spi2_dma_tx;
#endif

#ifdef BSP_SPI3_RX_USING_DMA
    spi_bus_obj[SPI3_INDEX].spi_dma_flag |= SPI_USING_RX_DMA_FLAG;
    static const struct dma_config spi3_dma_rx = SPI3_RX_DMA_CONFIG;
    spi_bus_obj[SPI3_INDEX].dma.rx = &spi3_dma_rx;
#endif

#ifdef BSP_SPI3_TX_USING_DMA
    spi_bus_obj[SPI3_INDEX].spi_dma_flag |= SPI_USING_TX_DMA_FLAG;
    static const struct dma_config spi3_dma_tx = SPI3_TX_DMA_CONFIG;
    spi_bus_obj[SPI3_INDEX].dma.tx = &spi3_dma_tx;
#endif

#ifdef BSP_SPI4_RX_USING_DMA
    spi_bus_obj[SPI4_INDEX].spi_dma_flag |= SPI_USING_RX_DMA_FLAG;
    static const struct dma_config spi4_dma_rx = SPI4_RX_DMA_CONFIG;
    spi_bus_obj[SPI4_INDEX].dma.rx = &spi4_dma_rx;
#endif

#ifdef BSP_SPI4_TX_USING_DMA
    spi_bus_obj[SPI4_INDEX].spi_dma_flag |= SPI_USING_TX_DMA_FLAG;
    static const struct dma_config spi4_dma_tx = SPI4_TX_DMA_CONFIG;
    spi_bus_obj[SPI4_INDEX].dma.tx = &spi4_dma_tx;
#endif
}

int rt_hw_spi_init (void)
{
    int result = 0;
    int i;

    gd32_get_dma_info();

    for (i = 0; i < sizeof(spi_bus_obj) / sizeof(spi_bus_obj[0]); i++)
    {
        spi_bus_obj[i].config = &spi_config[i];
        spi_bus_obj[i].spi_bus.parent.user_data = (void *)&spi_bus_obj[i];

        result = rt_spi_bus_register(&spi_bus_obj[i].spi_bus, spi_bus_obj[i].config->bus_name, &gd32_spi_ops);

        RT_ASSERT(result == RT_EOK);

        LOG_D("%s bus init done", spi_bus_obj[i].bus_name);
    }

    return result;
}

INIT_BOARD_EXPORT(rt_hw_spi_init);

#endif /* BSP_USING_SPI0 || BSP_USING_SPI1 || BSP_USING_SPI2 || BSP_USING_SPI3 || BSP_USING_SPI4*/
#endif /* RT_USING_SPI */
