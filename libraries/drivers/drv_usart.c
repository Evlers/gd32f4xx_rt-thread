/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-20     BruceOu      first implementation
 * 2024-03-19     Evlers       add dma supports
 * 2024-03-20     Evlers       add driver configure
 */

#include "drv_usart.h"
#include "drv_config.h"

#ifdef RT_USING_SERIAL_V1

#if !defined(BSP_USING_UART0) && !defined(BSP_USING_UART1) && \
    !defined(BSP_USING_UART2) && !defined(BSP_USING_UART3) && \
    !defined(BSP_USING_UART4) && !defined(BSP_USING_UART5) && \
    !defined(BSP_USING_UART6) && !defined(BSP_USING_UART7)
#error "Please define at least one UARTx"

#endif

#include <rtdevice.h>

enum {
#ifdef BSP_USING_UART0
    UART0_INDEX,
#endif
#ifdef BSP_USING_UART1
    UART1_INDEX,
#endif
#ifdef BSP_USING_UART2
    UART2_INDEX,
#endif
#ifdef BSP_USING_UART3
    UART3_INDEX,
#endif
#ifdef BSP_USING_UART4
    UART4_INDEX,
#endif
#ifdef BSP_USING_UART5
    UART5_INDEX,
#endif
#ifdef BSP_USING_UART6
    UART6_INDEX,
#endif
#ifdef BSP_USING_UART7
    UART7_INDEX,
#endif
};

static const struct gd32_uart_config uart_config[] = {
#ifdef BSP_USING_UART0
    UART0_CONFIG,
#endif

#ifdef BSP_USING_UART1
    UART1_CONFIG,
#endif

#ifdef BSP_USING_UART2
    UART2_CONFIG,
#endif

#ifdef BSP_USING_UART3
    UART3_CONFIG,
#endif

#ifdef BSP_USING_UART4
    UART4_CONFIG,
#endif

#ifdef BSP_USING_UART5
    UART5_CONFIG,
#endif

#ifdef BSP_USING_UART6
    UART6_CONFIG,
#endif

#ifdef BSP_USING_UART7
    UART7_CONFIG,
#endif
};

static struct gd32_uart uart_obj[sizeof(uart_config) / sizeof(uart_config[0])] = { 0 };


#ifdef RT_SERIAL_USING_DMA
static void dma_recv_isr (struct rt_serial_device *serial)
{
    struct gd32_uart *uart;
    rt_size_t recv_len, counter;

    RT_ASSERT(serial != RT_NULL);
    uart = rt_container_of(serial, struct gd32_uart, serial);

    recv_len = 0;
    counter = dma_transfer_number_get(uart->dma.rx->periph, uart->dma.rx->channel);

    if (counter <= uart->dma.last_index)
    {
        recv_len = uart->dma.last_index - counter;
    }
    else
    {
        recv_len = serial->config.bufsz + uart->dma.last_index - counter;
    }

    if (recv_len)
    {
        uart->dma.last_index = counter;
        rt_hw_serial_isr(serial, RT_SERIAL_EVENT_RX_DMADONE | (recv_len << 8));
    }
}
#endif

static void usart_isr (struct rt_serial_device *serial)
{
    struct gd32_uart *uart;

    RT_ASSERT(serial != RT_NULL);
    uart = rt_container_of(serial, struct gd32_uart, serial);

    if (usart_interrupt_flag_get(uart->config->periph, USART_INT_FLAG_RBNE) != RESET)
    {
        rt_hw_serial_isr(serial, RT_SERIAL_EVENT_RX_IND);
        /* Clear RXNE interrupt flag */
        usart_interrupt_flag_clear(uart->config->periph, USART_INT_FLAG_RBNE);
    }

#ifdef RT_SERIAL_USING_DMA
    else if (usart_interrupt_flag_get(uart->config->periph, USART_INT_FLAG_IDLE) != RESET)
    {
        volatile uint8_t data = (uint8_t)usart_data_receive(uart->config->periph);

        dma_recv_isr(serial);

        usart_interrupt_flag_clear(uart->config->periph, USART_INT_FLAG_IDLE);
    }
#endif
    else
    {
        if (usart_interrupt_flag_get(uart->config->periph, USART_INT_FLAG_ERR_ORERR) != RESET)
        {
            usart_interrupt_flag_clear(uart->config->periph, USART_INT_FLAG_ERR_ORERR);
        }

        if (usart_interrupt_flag_get(uart->config->periph, USART_INT_FLAG_ERR_NERR) != RESET)
        {
            usart_interrupt_flag_clear(uart->config->periph, USART_INT_FLAG_ERR_NERR);
        }

        if (usart_interrupt_flag_get(uart->config->periph, USART_INT_FLAG_ERR_FERR) != RESET)
        {
            usart_interrupt_flag_clear(uart->config->periph, USART_INT_FLAG_ERR_FERR);
        }

        if (usart_interrupt_flag_get(uart->config->periph, USART_INT_FLAG_RBNE_ORERR) != RESET)
        {
            usart_interrupt_flag_clear(uart->config->periph, USART_INT_FLAG_RBNE_ORERR);
        }

        if (usart_interrupt_flag_get(uart->config->periph, USART_INT_FLAG_PERR) != RESET)
        {
            usart_interrupt_flag_clear(uart->config->periph, USART_INT_FLAG_PERR);
        }

        if (usart_interrupt_flag_get(uart->config->periph, USART_INT_FLAG_CTS) != RESET)
        {
            usart_interrupt_flag_clear(uart->config->periph, USART_INT_FLAG_CTS);
        }

        if (usart_interrupt_flag_get(uart->config->periph, USART_INT_FLAG_LBD) != RESET)
        {
            usart_interrupt_flag_clear(uart->config->periph, USART_INT_FLAG_LBD);
        }

        if (usart_interrupt_flag_get(uart->config->periph, USART_INT_FLAG_EB) != RESET)
        {
            usart_interrupt_flag_clear(uart->config->periph, USART_INT_FLAG_EB);
        }

        if (usart_interrupt_flag_get(uart->config->periph, USART_INT_FLAG_RT) != RESET)
        {
            usart_interrupt_flag_clear(uart->config->periph, USART_INT_FLAG_RT);
        }
    }
}

#if defined(BSP_UART0_RX_USING_DMA) || \
    defined(BSP_UART1_RX_USING_DMA) || \
    defined(BSP_UART2_RX_USING_DMA) || \
    defined(BSP_UART3_RX_USING_DMA) || \
    defined(BSP_UART4_RX_USING_DMA) || \
    defined(BSP_UART5_RX_USING_DMA) || \
    defined(BSP_UART6_RX_USING_DMA) || \
    defined(BSP_UART7_RX_USING_DMA)
static void dma_rx_isr (struct rt_serial_device *serial)
{
    struct gd32_uart *uart;

    RT_ASSERT(serial != RT_NULL);
    uart = rt_container_of(serial, struct gd32_uart, serial);

    if ((dma_interrupt_flag_get(uart->dma.rx->periph, uart->dma.rx->channel, DMA_INT_FLAG_HTF) != RESET) ||
        (dma_interrupt_flag_get(uart->dma.rx->periph, uart->dma.rx->channel, DMA_INT_FLAG_FTF) != RESET))
    {
        dma_recv_isr(serial);

        /* clear dma flag */
        dma_interrupt_flag_clear(uart->dma.rx->periph, uart->dma.rx->channel, DMA_INT_FLAG_HTF);
        dma_interrupt_flag_clear(uart->dma.rx->periph, uart->dma.rx->channel, DMA_INT_FLAG_FTF);
    }
}
#endif

#if defined(BSP_UART0_TX_USING_DMA) || \
    defined(BSP_UART1_TX_USING_DMA) || \
    defined(BSP_UART2_TX_USING_DMA) || \
    defined(BSP_UART3_TX_USING_DMA) || \
    defined(BSP_UART4_TX_USING_DMA) || \
    defined(BSP_UART5_TX_USING_DMA) || \
    defined(BSP_UART6_TX_USING_DMA) || \
    defined(BSP_UART7_TX_USING_DMA)
static void dma_tx_isr (struct rt_serial_device *serial)
{
    struct gd32_uart *uart;

    RT_ASSERT(serial != RT_NULL);
    uart = rt_container_of(serial, struct gd32_uart, serial);

    if (dma_interrupt_flag_get(uart->dma.tx->periph, uart->dma.tx->channel, DMA_INT_FLAG_FTF) != RESET)
    {
        rt_size_t trans_total_index;

        /* clear dma flag */
        dma_interrupt_flag_clear(uart->dma.tx->periph, uart->dma.tx->channel, DMA_INT_FLAG_FTF);

        /* disable dma tx channel */
        dma_channel_disable(uart->dma.tx->periph, uart->dma.tx->channel);

        trans_total_index = dma_transfer_number_get(uart->dma.tx->periph, uart->dma.tx->channel);

        if (trans_total_index == 0)
        {
            rt_hw_serial_isr(serial, RT_SERIAL_EVENT_TX_DMADONE);
        }
    }
}
#endif

#if defined(BSP_USING_UART0)
void USART0_IRQHandler (void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    usart_isr(&uart_obj[UART0_INDEX].serial);

    /* leave interrupt */
    rt_interrupt_leave();
}

#endif /* BSP_USING_UART0 */

#if defined(BSP_USING_UART1)
void USART1_IRQHandler (void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    usart_isr(&uart_obj[UART1_INDEX].serial);

    /* leave interrupt */
    rt_interrupt_leave();
}

#endif /* BSP_USING_UART1 */

#if defined(BSP_USING_UART2)
void USART2_IRQHandler (void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    usart_isr(&uart_obj[UART2_INDEX].serial);

    /* leave interrupt */
    rt_interrupt_leave();
}

#endif /* BSP_USING_UART2 */

#if defined(BSP_USING_UART3)
void UART3_IRQHandler (void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    usart_isr(&uart_obj[UART3_INDEX].serial);

    /* leave interrupt */
    rt_interrupt_leave();
}

#endif /* BSP_USING_UART3 */

#if defined(BSP_USING_UART4)
void UART4_IRQHandler (void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    usart_isr(&uart_obj[UART4_INDEX].serial);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* BSP_USING_UART4 */

#if defined(BSP_USING_UART5)
void USART5_IRQHandler (void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    usart_isr(&uart_obj[UART5_INDEX].serial);

    /* leave interrupt */
    rt_interrupt_leave();
}

#endif /* BSP_USING_UART5 */

#if defined(BSP_USING_UART6)
void UART6_IRQHandler (void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    usart_isr(&uart_obj[UART6_INDEX].serial);

    /* leave interrupt */
    rt_interrupt_leave();
}

#endif /* BSP_USING_UART6 */

#if defined(BSP_USING_UART7)
void UART7_IRQHandler (void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    usart_isr(&uart_obj[UART7_INDEX].serial);

    /* leave interrupt */
    rt_interrupt_leave();
}

#endif /* BSP_USING_UART7 */

#ifdef BSP_UART0_RX_USING_DMA
void UART0_DMA_RX_IRQHandler (void)
{
    dma_rx_isr(&uart_obj[UART0_INDEX].serial);
}
#endif

#ifdef BSP_UART0_TX_USING_DMA
void UART0_DMA_TX_IRQHandler (void)
{
    dma_tx_isr(&uart_obj[UART0_INDEX].serial);
}
#endif

#ifdef BSP_UART1_RX_USING_DMA
void UART1_DMA_RX_IRQHandler (void)
{
    dma_rx_isr(&uart_obj[UART1_INDEX].serial);
}
#endif

#ifdef BSP_UART1_TX_USING_DMA
void UART1_DMA_TX_IRQHandler (void)
{
    dma_tx_isr(&uart_obj[UART1_INDEX].serial);
}
#endif

#ifdef BSP_UART2_RX_USING_DMA
void UART2_DMA_RX_IRQHandler (void)
{
    dma_rx_isr(&uart_obj[UART2_INDEX].serial);
}
#endif

#ifdef BSP_UART2_TX_USING_DMA
void UART2_DMA_TX_IRQHandler (void)
{
    dma_tx_isr(&uart_obj[UART2_INDEX].serial);
}
#endif

#ifdef BSP_UART3_RX_USING_DMA
void UART3_DMA_RX_IRQHandler (void)
{
    dma_rx_isr(&uart_obj[UART3_INDEX].serial);
}
#endif

#ifdef BSP_UART3_TX_USING_DMA
void UART3_DMA_TX_IRQHandler (void)
{
    dma_tx_isr(&uart_obj[UART3_INDEX].serial);
}
#endif

#ifdef BSP_UART4_RX_USING_DMA
void UART4_DMA_RX_IRQHandler (void)
{
    dma_rx_isr(&uart_obj[UART4_INDEX].serial);
}
#endif

#ifdef BSP_UART4_TX_USING_DMA
void UART4_DMA_TX_IRQHandler (void)
{
    dma_tx_isr(&uart_obj[UART4_INDEX].serial);
}
#endif

#ifdef BSP_UART5_RX_USING_DMA
void UART5_DMA_RX_IRQHandler (void)
{
    dma_rx_isr(&uart_obj[UART5_INDEX].serial);
}
#endif

#ifdef BSP_UART5_TX_USING_DMA
void UART5_DMA_TX_IRQHandler (void)
{
    dma_tx_isr(&uart_obj[UART5_INDEX].serial);
}
#endif

#ifdef BSP_UART6_RX_USING_DMA
void UART6_DMA_RX_IRQHandler (void)
{
    dma_rx_isr(&uart_obj[UART6_INDEX].serial);
}
#endif

#ifdef BSP_UART6_TX_USING_DMA
void UART6_DMA_TX_IRQHandler (void)
{
    dma_tx_isr(&uart_obj[UART6_INDEX].serial);
}
#endif

#ifdef BSP_UART7_RX_USING_DMA
void UART7_DMA_RX_IRQHandler (void)
{
    dma_rx_isr(&uart_obj[UART7_INDEX].serial);
}
#endif

#ifdef BSP_UART7_TX_USING_DMA
void UART7_DMA_TX_IRQHandler (void)
{
    dma_tx_isr(&uart_obj[UART7_INDEX].serial);
}
#endif

/**
* @brief UART MSP Initialization
*        This function configures the hardware resources used in this example:
*           - Peripheral's clock enable
*           - Peripheral's GPIO Configuration
*           - NVIC configuration for UART interrupt request enable
* @param huart: UART handle pointer
* @retval None
*/
void gd32_uart_gpio_init (struct gd32_uart *uart)
{
    /* enable USART clock */
    rcu_periph_clock_enable(uart->config->tx_gpio_clk);
    rcu_periph_clock_enable(uart->config->rx_gpio_clk);
    rcu_periph_clock_enable(uart->config->per_clk);

#if defined SOC_SERIES_GD32F4xx
    /* connect port to USARTx_Tx */
    gpio_af_set(uart->config->tx_port, uart->config->tx_af, uart->config->tx_pin);

    /* connect port to USARTx_Rx */
    gpio_af_set(uart->config->rx_port, uart->config->rx_af, uart->config->rx_pin);

    /* configure USART Tx as alternate function push-pull */
    gpio_mode_set(uart->config->tx_port, GPIO_MODE_AF, GPIO_PUPD_PULLUP, uart->config->tx_pin);
    gpio_output_options_set(uart->config->tx_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, uart->config->tx_pin);

    /* configure USART Rx as alternate function push-pull */
    gpio_mode_set(uart->config->rx_port, GPIO_MODE_AF, GPIO_PUPD_PULLUP, uart->config->rx_pin);
    gpio_output_options_set(uart->config->rx_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, uart->config->rx_pin);
#else
    /* connect port to USARTx_Tx */
    gpio_init(uart->tx_port, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, uart->tx_pin);

    /* connect port to USARTx_Rx */
    gpio_init(uart->rx_port, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, uart->rx_pin);
#endif

    NVIC_SetPriority(uart->config->irqn, 0);
    NVIC_EnableIRQ(uart->config->irqn);
}

/**
  * @brief  uart configure
  * @param  serial, cfg
  * @retval None
  */
static rt_err_t gd32_uart_configure (struct rt_serial_device *serial, struct serial_configure *cfg)
{
    struct gd32_uart *uart;

    RT_ASSERT(serial != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);
    uart = rt_container_of(serial, struct gd32_uart, serial);

#ifdef RT_SERIAL_USING_DMA
    uart->dma.last_index = serial->config.bufsz;
#endif

    gd32_uart_gpio_init(uart);

    usart_baudrate_set(uart->config->periph, cfg->baud_rate);

    switch (cfg->data_bits)
    {
    case DATA_BITS_9:
        usart_word_length_set(uart->config->periph, USART_WL_9BIT);
        break;

    default:
        usart_word_length_set(uart->config->periph, USART_WL_8BIT);
        break;
    }

    switch (cfg->stop_bits)
    {
    case STOP_BITS_2:
        usart_stop_bit_set(uart->config->periph, USART_STB_2BIT);
        break;
    default:
        usart_stop_bit_set(uart->config->periph, USART_STB_1BIT);
        break;
    }

    switch (cfg->parity)
    {
    case PARITY_ODD:
        usart_parity_config(uart->config->periph, USART_PM_ODD);
        break;
    case PARITY_EVEN:
        usart_parity_config(uart->config->periph, USART_PM_EVEN);
        break;
    default:
        usart_parity_config(uart->config->periph, USART_PM_NONE);
        break;
    }

    usart_receive_config(uart->config->periph, USART_RECEIVE_ENABLE);
    usart_transmit_config(uart->config->periph, USART_TRANSMIT_ENABLE);
    usart_enable(uart->config->periph);

    return RT_EOK;
}

#ifdef RT_SERIAL_USING_DMA
static void _uart_dma_receive (struct gd32_uart *uart, rt_uint8_t *buffer, rt_uint32_t size)
{
#ifndef SOC_SERIES_GD32F4xx
    dma_parameter_struct dma_init_struct = { 0 };
#else
    dma_single_data_parameter_struct dma_init_struct = { 0 };
#endif

    /* clear all the interrupt flags */
#ifndef SOC_SERIES_GD32F4xx
    dma_flag_clear(uart->dma.rx->periph, uart->dma.rx->channel, DMA_FLAG_ERR);
#else
    dma_flag_clear(uart->dma.rx->periph, uart->dma.rx->channel, DMA_FLAG_FEE);
    dma_flag_clear(uart->dma.rx->periph, uart->dma.rx->channel, DMA_FLAG_SDE);
    dma_flag_clear(uart->dma.rx->periph, uart->dma.rx->channel, DMA_FLAG_TAE);
#endif
    dma_flag_clear(uart->dma.rx->periph, uart->dma.rx->channel, DMA_FLAG_HTF);
    dma_flag_clear(uart->dma.rx->periph, uart->dma.rx->channel, DMA_FLAG_FTF);
    dma_channel_disable(uart->dma.rx->periph, uart->dma.rx->channel);
    dma_deinit(uart->dma.rx->periph, uart->dma.rx->channel);

    /* configure receive DMA */
    rcu_periph_clock_enable(uart->dma.rx->rcu);
    dma_deinit(uart->dma.rx->periph, uart->dma.rx->channel);

    dma_init_struct.number              = size;
#ifndef SOC_SERIES_GD32F4xx
    dma_init_struct.memory_addr         = (uint32_t)buffer;
    dma_init_struct.periph_addr         = (uint32_t)&USART_DATA(uart->config->periph);
    dma_init_struct.memory_width        = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.periph_width        = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.direction           = DMA_PERIPHERAL_TO_MEMORY;
#else
    dma_init_struct.memory0_addr        = (uint32_t)buffer;
    dma_init_struct.periph_addr         = (uint32_t)&USART_DATA(uart->config->periph);
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.circular_mode       = DMA_CIRCULAR_MODE_ENABLE;
    dma_init_struct.direction           = DMA_PERIPH_TO_MEMORY;
#endif
    dma_init_struct.periph_inc          = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.memory_inc          = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.priority            = DMA_PRIORITY_HIGH;
#ifndef SOC_SERIES_GD32F4xx
    dma_init(uart->dma.rx->periph, uart->dma.rx->channel, &dma_init_struct);
    dma_circulation_enable(uart->dma.rx->periph, uart->dma.rx->channel);
    dma_memory_to_memory_disable(uart->dma.rx->periph, uart->dma.rx->channel);
#else
    dma_single_data_mode_init(uart->dma.rx->periph, uart->dma.rx->channel, &dma_init_struct);
    dma_channel_subperipheral_select(uart->dma.rx->periph, uart->dma.rx->channel, uart->dma.rx->subperiph);
#endif

    /* enable transmit complete interrupt */
    nvic_irq_enable(uart->dma.rx->irq, 2, 0);
    dma_interrupt_enable(uart->dma.rx->periph, uart->dma.rx->channel, DMA_CHXCTL_HTFIE);
    dma_interrupt_enable(uart->dma.rx->periph, uart->dma.rx->channel, DMA_CHXCTL_FTFIE);

    /* enable dma channel */
    dma_channel_enable(uart->dma.rx->periph, uart->dma.rx->channel);

    /* enable usart idle interrupt */
    usart_interrupt_enable(uart->config->periph, USART_INT_IDLE);

    /* enable dma receive */
#ifndef SOC_SERIES_GD32F4xx
    usart_dma_receive_config(uart->config->periph, USART_DENR_ENABLE);
#else
    usart_dma_receive_config(uart->config->periph, USART_RECEIVE_DMA_ENABLE);
#endif
}

static void _uart_dma_transmit (struct gd32_uart *uart, rt_uint8_t *buffer, rt_uint32_t size)
{
    /* Set the data length and data pointer */
#ifndef SOC_SERIES_GD32F4xx
    DMA_CHMADDR(uart->dma.tx->periph, uart->dma.tx->channel) = (uint32_t)buffer;
#else
    DMA_CHM0ADDR(uart->dma.tx->periph, uart->dma.tx->channel) = (uint32_t)buffer;
#endif
    DMA_CHCNT(uart->dma.tx->periph, uart->dma.tx->channel) = size;

    /* enable dma transmit */
#ifndef SOC_SERIES_GD32F4xx
    usart_dma_transmit_config(uart->config->periph, USART_DENT_ENABLE);
#else
    usart_dma_transmit_config(uart->config->periph, USART_TRANSMIT_DMA_ENABLE);
#endif

    /* enable dma channel */
    dma_channel_enable(uart->dma.tx->periph, uart->dma.tx->channel);
}

static void gd32_dma_config (struct rt_serial_device *serial, rt_ubase_t flag)
{
    struct gd32_uart *uart;
    struct rt_serial_rx_fifo *rx_fifo;
#ifndef SOC_SERIES_GD32F4xx
    dma_parameter_struct dma_init_struct = { 0 };
#else
    dma_single_data_parameter_struct dma_init_struct = { 0 };
#endif

    RT_ASSERT(serial != RT_NULL);
    uart = rt_container_of(serial, struct gd32_uart, serial);

    RT_ASSERT(flag == RT_DEVICE_FLAG_DMA_TX || flag == RT_DEVICE_FLAG_DMA_RX);

    /* enable tx dma */
    if (flag == RT_DEVICE_FLAG_DMA_TX)
    {
        /* clear all the interrupt flags */
#ifndef SOC_SERIES_GD32F4xx
        dma_flag_clear(uart->dma.tx->periph, uart->dma.tx->channel, DMA_FLAG_ERR);
#else
        dma_flag_clear(uart->dma.tx->periph, uart->dma.tx->channel, DMA_FLAG_FEE);
        dma_flag_clear(uart->dma.tx->periph, uart->dma.tx->channel, DMA_FLAG_SDE);
        dma_flag_clear(uart->dma.tx->periph, uart->dma.tx->channel, DMA_FLAG_TAE);
#endif
        dma_flag_clear(uart->dma.tx->periph, uart->dma.tx->channel, DMA_FLAG_HTF);
        dma_flag_clear(uart->dma.tx->periph, uart->dma.tx->channel, DMA_FLAG_FTF);
        dma_channel_disable(uart->dma.tx->periph, uart->dma.tx->channel);
        dma_deinit(uart->dma.tx->periph, uart->dma.tx->channel);

        /* configure receive DMA */
        rcu_periph_clock_enable(uart->dma.tx->rcu);
        dma_deinit(uart->dma.tx->periph, uart->dma.tx->channel);

#ifndef SOC_SERIES_GD32F4xx
        dma_init_struct.periph_addr         = (uint32_t)&USART_DATA(uart->config->periph);
        dma_init_struct.memory_width        = DMA_MEMORY_WIDTH_8BIT;
        dma_init_struct.periph_width        = DMA_PERIPHERAL_WIDTH_8BIT;
        dma_init_struct.direction           = DMA_MEMORY_TO_PERIPHERAL;
#else
        dma_init_struct.periph_addr         = (uint32_t)&USART_DATA(uart->config->periph);
        dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
        dma_init_struct.circular_mode       = DMA_CIRCULAR_MODE_DISABLE;
        dma_init_struct.direction           = DMA_MEMORY_TO_PERIPH;
#endif
        dma_init_struct.periph_inc          = DMA_PERIPH_INCREASE_DISABLE;
        dma_init_struct.memory_inc          = DMA_MEMORY_INCREASE_ENABLE;
        dma_init_struct.priority            = DMA_PRIORITY_HIGH;

#ifndef SOC_SERIES_GD32F4xx
        dma_init(uart->dma.tx->periph, uart->dma.tx->channel, &dma_init_struct);
        dma_circulation_disable(uart->dma.tx->periph, uart->dma.tx->channel);
        dma_memory_to_memory_disable(uart->dma.tx->periph, uart->dma.tx->channel);
#else
        dma_single_data_mode_init(uart->dma.tx->periph, uart->dma.tx->channel, &dma_init_struct);
        dma_channel_subperipheral_select(uart->dma.tx->periph, uart->dma.tx->channel, uart->dma.tx->subperiph);
#endif

        /* enable tx dma interrupt */
        nvic_irq_enable(uart->dma.tx->irq, 2, 0);

        /* enable transmit complete interrupt */
        dma_interrupt_enable(uart->dma.tx->periph, uart->dma.tx->channel, DMA_CHXCTL_FTFIE);
    }

    /* enable rx dma */
    if (flag == RT_DEVICE_FLAG_DMA_RX)
    {
        rx_fifo = (struct rt_serial_rx_fifo *)serial->serial_rx;
        /* start dma transfer */
        _uart_dma_receive(uart, rx_fifo->buffer, serial->config.bufsz);
    }
}

#endif

/**
  * @brief  uart control
  * @param  serial, arg
  * @retval None
  */
static rt_err_t gd32_uart_control (struct rt_serial_device *serial, int cmd, void *arg)
{
    struct gd32_uart *uart;
#ifdef RT_SERIAL_USING_DMA
    rt_ubase_t ctrl_arg = (rt_ubase_t)arg;
#endif

    RT_ASSERT(serial != RT_NULL);
    uart = rt_container_of(serial, struct gd32_uart, serial);

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        /* disable rx irq */
        NVIC_DisableIRQ(uart->config->irqn);

        /* disable interrupt */
        if (ctrl_arg == RT_DEVICE_FLAG_INT_RX)
        {
            usart_interrupt_disable(uart->config->periph, USART_INT_RBNE);
        }
        else if (ctrl_arg == RT_DEVICE_FLAG_INT_TX)
        {
            usart_interrupt_disable(uart->config->periph, USART_INT_TBE);
        }

#ifdef RT_SERIAL_USING_DMA
        /* disable DMA */
        else if (ctrl_arg == RT_DEVICE_FLAG_DMA_RX)
        {
            usart_interrupt_disable(uart->config->periph, USART_INT_RBNE);
            NVIC_DisableIRQ(uart->dma.rx->irq);
            dma_deinit(uart->dma.rx->periph, uart->dma.rx->channel);
        }
        else if(ctrl_arg == RT_DEVICE_FLAG_DMA_TX)
        {
            usart_interrupt_disable(uart->config->periph, USART_INT_TBE);
            NVIC_DisableIRQ(uart->dma.tx->irq);
            dma_deinit(uart->dma.tx->periph, uart->dma.tx->channel);
        }
#endif
        break;

    case RT_DEVICE_CTRL_SET_INT:
        /* enable rx irq */
        NVIC_EnableIRQ(uart->config->irqn);

        /* enable interrupt */
        if (ctrl_arg == RT_DEVICE_FLAG_INT_RX)
        {
            usart_interrupt_enable(uart->config->periph, USART_INT_RBNE);
        }
        else if (ctrl_arg == RT_DEVICE_FLAG_INT_TX)
        {
            usart_interrupt_enable(uart->config->periph, USART_INT_TBE);
        }
        break;

    case RT_DEVICE_CTRL_CONFIG:
        if(ctrl_arg & (RT_DEVICE_FLAG_DMA_RX | RT_DEVICE_FLAG_DMA_TX))
        {
#ifdef RT_SERIAL_USING_DMA
            gd32_dma_config(serial, ctrl_arg);
#endif
        }
        else
        {
            gd32_uart_control(serial, RT_DEVICE_CTRL_SET_INT, (void *)ctrl_arg);
        }
        break;

    case RT_DEVICE_CTRL_CLOSE:
        usart_deinit(uart->config->periph);
        break;
    }

    return RT_EOK;
}

/**
  * @brief  uart put char
  * @param  serial, ch
  * @retval None
  */
static int gd32_uart_putc (struct rt_serial_device *serial, char ch)
{
    struct gd32_uart *uart;

    RT_ASSERT(serial != RT_NULL);
    uart = rt_container_of(serial, struct gd32_uart, serial);

    usart_data_transmit(uart->config->periph, ch);
    while((usart_flag_get(uart->config->periph, USART_FLAG_TBE) == RESET));

    return RT_EOK;
}

/**
  * @brief  uart get char
  * @param  serial
  * @retval None
  */
static int gd32_uart_getc (struct rt_serial_device *serial)
{
    int ch;
    struct gd32_uart *uart;

    RT_ASSERT(serial != RT_NULL);
    uart = rt_container_of(serial, struct gd32_uart, serial);

    ch = -1;
    if (usart_flag_get(uart->config->periph, USART_FLAG_RBNE) != RESET)
        ch = usart_data_receive(uart->config->periph);
    return ch;
}

#ifdef RT_SERIAL_USING_DMA

static rt_ssize_t gd32_dma_transmit (struct rt_serial_device *serial, rt_uint8_t *buf, rt_size_t size, int direction)
{
    struct gd32_uart *uart;

    RT_ASSERT(buf != RT_NULL);
    RT_ASSERT(serial != RT_NULL);
    uart = rt_container_of(serial, struct gd32_uart, serial);

    if (size == 0)
    {
        return 0;
    }

    if (RT_SERIAL_DMA_TX == direction)
    {
        _uart_dma_transmit(uart, buf, size);
    }

    return size;
}
#endif

static const struct rt_uart_ops gd32_uart_ops =
{
    .configure = gd32_uart_configure,
    .control = gd32_uart_control,
    .putc = gd32_uart_putc,
    .getc = gd32_uart_getc,
#ifdef RT_SERIAL_USING_DMA
    .dma_transmit = gd32_dma_transmit,
#else
    .dma_transmit = RT_NULL,
#endif
};

static void gd32_get_dma_info (void)
{
#ifdef BSP_USING_UART0
    uart_obj[UART0_INDEX].uart_dma_flag = 0;
#ifdef BSP_UART0_RX_USING_DMA
    uart_obj[UART0_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_RX;
    static const struct dma_config uart0_dma_rx = UART0_RX_DMA_CONFIG;
    uart_obj[UART0_INDEX].dma.rx = &uart0_dma_rx;
#endif
#ifdef BSP_UART0_TX_USING_DMA
    uart_obj[UART0_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_TX;
    static const struct dma_config uart0_dma_tx = UART0_TX_DMA_CONFIG;
    uart_obj[UART0_INDEX].dma.tx = &uart0_dma_tx;
#endif
#endif

#ifdef BSP_USING_UART1
    uart_obj[UART1_INDEX].uart_dma_flag = 0;
#ifdef BSP_UART1_RX_USING_DMA
    uart_obj[UART1_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_RX;
    static const struct dma_config uart1_dma_rx = UART1_RX_DMA_CONFIG;
    uart_obj[UART1_INDEX].dma.rx = &uart1_dma_rx;
#endif
#ifdef BSP_UART1_TX_USING_DMA
    uart_obj[UART1_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_TX;
    static const struct dma_config uart1_dma_tx = UART1_TX_DMA_CONFIG;
    uart_obj[UART1_INDEX].dma.tx = &uart1_dma_tx;
#endif
#endif

#ifdef BSP_USING_UART2
    uart_obj[UART2_INDEX].uart_dma_flag = 0;
#ifdef BSP_UART2_RX_USING_DMA
    uart_obj[UART2_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_RX;
    static const struct dma_config uart2_dma_rx = UART2_RX_DMA_CONFIG;
    uart_obj[UART2_INDEX].dma.rx = &uart2_dma_rx;
#endif
#ifdef BSP_UART2_TX_USING_DMA
    uart_obj[UART2_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_TX;
    static const struct dma_config uart2_dma_tx = UART2_TX_DMA_CONFIG;
    uart_obj[UART2_INDEX].dma.tx = &uart2_dma_tx;
#endif
#endif

#ifdef BSP_USING_UART3
    uart_obj[UART3_INDEX].uart_dma_flag = 0;
#ifdef BSP_UART3_RX_USING_DMA
    uart_obj[UART3_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_RX;
    static const struct dma_config uart3_dma_rx = UART3_RX_DMA_CONFIG;
    uart_obj[UART3_INDEX].dma.rx = &uart3_dma_rx;
#endif
#ifdef BSP_UART3_TX_USING_DMA
    uart_obj[UART3_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_TX;
    static const struct dma_config uart3_dma_tx = UART3_TX_DMA_CONFIG;
    uart_obj[UART3_INDEX].dma.tx = &uart3_dma_tx;
#endif
#endif

#ifdef BSP_USING_UART4
    uart_obj[UART4_INDEX].uart_dma_flag = 0;
#ifdef BSP_UART4_RX_USING_DMA
    uart_obj[UART4_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_RX;
    static const struct dma_config uart4_dma_rx = UART4_RX_DMA_CONFIG;
    uart_obj[UART4_INDEX].dma.rx = &uart4_dma_rx;
#endif
#ifdef BSP_UART4_TX_USING_DMA
    uart_obj[UART4_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_TX;
    static const struct dma_config uart4_dma_tx = UART4_TX_DMA_CONFIG;
    uart_obj[UART4_INDEX].dma.tx = &uart4_dma_tx;
#endif
#endif

#ifdef BSP_USING_UART5
    uart_obj[UART5_INDEX].uart_dma_flag = 0;
#ifdef BSP_UART5_RX_USING_DMA
    uart_obj[UART5_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_RX;
    static const struct dma_config uart5_dma_rx = UART5_RX_DMA_CONFIG;
    uart_obj[UART5_INDEX].dma.rx = &uart5_dma_rx;
#endif
#ifdef BSP_UART5_TX_USING_DMA
    uart_obj[UART5_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_TX;
    static const struct dma_config uart5_dma_tx = UART5_TX_DMA_CONFIG;
    uart_obj[UART5_INDEX].dma.tx = &uart5_dma_tx;
#endif
#endif

#ifdef BSP_USING_UART6
    uart_obj[UART6_INDEX].uart_dma_flag = 0;
#ifdef BSP_UART6_RX_USING_DMA
    uart_obj[UART6_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_RX;
    static const struct dma_config uart6_dma_rx = UART6_RX_DMA_CONFIG;
    uart_obj[UART6_INDEX].dma.rx = &uart6_dma_rx;
#endif
#ifdef BSP_UART6_TX_USING_DMA
    uart_obj[UART6_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_TX;
    static const struct dma_config uart6_dma_tx = UART6_TX_DMA_CONFIG;
    uart_obj[UART6_INDEX].dma.tx = &uart6_dma_tx;
#endif
#endif

#ifdef BSP_USING_UART7
    uart_obj[UART7_INDEX].uart_dma_flag = 0;
#ifdef BSP_UART7_RX_USING_DMA
    uart_obj[UART7_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_RX;
    static const struct dma_config uart7_dma_rx = UART7_RX_DMA_CONFIG;
    uart_obj[UART7_INDEX].dma.rx = &uart7_dma_rx;
#endif
#ifdef BSP_UART7_TX_USING_DMA
    uart_obj[UART7_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_TX;
    static const struct dma_config uart7_dma_tx = UART7_TX_DMA_CONFIG;
    uart_obj[UART7_INDEX].dma.tx = &uart7_dma_tx;
#endif
#endif
}

/**
  * @brief  uart init
  * @param  None
  * @retval None
  */
int rt_hw_usart_init (void)
{
    int i;
    int result;
	struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

    gd32_get_dma_info();

    for (i = 0; i < sizeof(uart_obj) / sizeof(uart_obj[0]); i++)
    {
        uart_obj[i].config = &uart_config[i];
        uart_obj[i].serial.ops = &gd32_uart_ops;
        uart_obj[i].serial.config = config;

        /* register UART1 device */
        result = rt_hw_serial_register(&uart_obj[i].serial,
                              uart_obj[i].config->device_name,
                              RT_DEVICE_FLAG_RDWR |
                              RT_DEVICE_FLAG_INT_RX |
                              uart_obj[i].uart_dma_flag,
                              (void *)&uart_obj[i]);
        RT_ASSERT(result == RT_EOK);
    }

    return result;
}

#endif
