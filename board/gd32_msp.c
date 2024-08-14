/*
 * Copyright (c) 2006-2024 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date             Author          Notes
 * 2024-03-21       Evlers          first version
 */

#include <rtthread.h>
#include "gd32f4xx.h"
#include "drv_dma.h"
#include "drv_sdio.h"
#include "f4xx/dma_config.h"


#ifdef BSP_USING_UART
/**
 * @brief UART MSP Initialization
 *        This function configures the hardware resources used in this example:
 *           - Peripheral's GPIO Configuration
 *           - NVIC configuration for interrupt priority
 *        This function belongs to weak function, users can rewrite this function according to different needs
 *
 * @param periph peripherals in gd32_uart_config
 * @return None
 */
void gd32_msp_usart_init (const uint32_t *periph)
{
#ifdef BSP_USING_UART0
    if (*periph == USART0)
    {
        /* configure gpio clock */
        rcu_periph_clock_enable(RCU_GPIOA);

        /* configure gpio */
        gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_9 | GPIO_PIN_10);
        gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9 | GPIO_PIN_10);
        gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9 | GPIO_PIN_10);

        /* configure interrupt priority */
        NVIC_SetPriority(USART0_IRQn, 2);
#ifdef BSP_UART0_RX_USING_DMA
        struct dma_config uart0_dma_rx = UART0_RX_DMA_CONFIG;
        NVIC_SetPriority(uart0_dma_rx.irq, 2);
#endif
#ifdef BSP_UART0_TX_USING_DMA
        struct dma_config uart0_dma_tx = UART0_TX_DMA_CONFIG;
        NVIC_SetPriority(uart0_dma_tx.irq, 2);
#endif
    }
#endif /* BSP_USING_UART0 */

#ifdef BSP_USING_UART1
    if (*periph == USART1)
    {
        /* configure gpio clock */
        rcu_periph_clock_enable(RCU_GPIOD);

        /* configure gpio */
        gpio_af_set(GPIOD, GPIO_AF_7, GPIO_PIN_5 | GPIO_PIN_6);
        gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_5 | GPIO_PIN_6);
        gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5 | GPIO_PIN_6);

        /* configure interrupt priority */
        NVIC_SetPriority(USART1_IRQn, 2);
#ifdef BSP_UART1_RX_USING_DMA
        struct dma_config uart1_dma_rx = UART1_RX_DMA_CONFIG;
        NVIC_SetPriority(uart1_dma_rx.irq, 2);
#endif
#ifdef BSP_UART1_TX_USING_DMA
        struct dma_config uart1_dma_tx = UART1_TX_DMA_CONFIG;
        NVIC_SetPriority(uart1_dma_tx.irq, 2);
#endif
    }
#endif /* BSP_USING_UART1 */

#ifdef BSP_USING_UART2
    if (*periph == USART2)
    {
        /* configure gpio clock */
        rcu_periph_clock_enable(RCU_GPIOB);

        /* configure gpio */
        gpio_af_set(GPIOB, GPIO_AF_7, GPIO_PIN_10 | GPIO_PIN_11);
        gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10 | GPIO_PIN_11);
        gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10 | GPIO_PIN_11);

        /* configure interrupt priority */
        NVIC_SetPriority(USART2_IRQn, 2);
#ifdef BSP_UART2_RX_USING_DMA
        struct dma_config uart2_dma_rx = UART2_RX_DMA_CONFIG;
        NVIC_SetPriority(uart2_dma_rx.irq, 2);
#endif
#ifdef BSP_UART2_TX_USING_DMA
        struct dma_config uart2_dma_tx = UART2_TX_DMA_CONFIG;
        NVIC_SetPriority(uart2_dma_tx.irq, 2);
#endif
    }
#endif /* BSP_USING_UART2 */

#ifdef BSP_USING_UART3
    if (*periph == UART3)
    {
        /* configure gpio clock */
        rcu_periph_clock_enable(RCU_GPIOA);

        /* configure gpio */
        gpio_af_set(GPIOA, GPIO_AF_8, GPIO_PIN_0 | GPIO_PIN_1);
        gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_0 | GPIO_PIN_1);
        gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0 | GPIO_PIN_1);

        /* configure interrupt priority */
        NVIC_SetPriority(UART3_IRQn, 2);
#ifdef BSP_UART3_RX_USING_DMA
        struct dma_config uart3_dma_rx = UART3_RX_DMA_CONFIG;
        NVIC_SetPriority(uart3_dma_rx.irq, 2);
#endif
#ifdef BSP_UART3_TX_USING_DMA
        struct dma_config uart3_dma_tx = UART3_TX_DMA_CONFIG;
        NVIC_SetPriority(uart0_dma_tx.irq, 2);
#endif
    }
#endif /* BSP_USING_UART3 */

#ifdef BSP_USING_UART6
    if (*periph == UART6)
    {
        /* configure gpio clock */
        rcu_periph_clock_enable(RCU_GPIOE);

        /* configure gpio */
        gpio_af_set(GPIOE, GPIO_AF_8, GPIO_PIN_7 | GPIO_PIN_8);
        gpio_mode_set(GPIOE, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_7 | GPIO_PIN_8);
        gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7 | GPIO_PIN_8);

        /* configure interrupt priority */
        NVIC_SetPriority(UART6_IRQn, 2);
#ifdef BSP_UART6_RX_USING_DMA
        struct dma_config uart6_dma_rx = UART6_RX_DMA_CONFIG;
        NVIC_SetPriority(uart6_dma_rx.irq, 2);
#endif
#ifdef BSP_UART6_TX_USING_DMA
        struct dma_config uart6_dma_tx = UART6_TX_DMA_CONFIG;
        NVIC_SetPriority(uart6_dma_tx.irq, 2);
#endif
    }
#endif /* BSP_USING_UART6 */

#ifdef BSP_USING_UART7
    if (*periph == UART7)
    {
        /* configure gpio clock */
        rcu_periph_clock_enable(RCU_GPIOE);

        /* configure gpio */
        gpio_af_set(GPIOE, GPIO_AF_8, GPIO_PIN_0 | GPIO_PIN_1);
        gpio_mode_set(GPIOE, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_0 | GPIO_PIN_1);
        gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0 | GPIO_PIN_1);

        /* configure interrupt priority */
        NVIC_SetPriority(UART7_IRQn, 2);
#ifdef BSP_UART7_RX_USING_DMA
        struct dma_config uart7_dma_rx = UART7_RX_DMA_CONFIG;
        NVIC_SetPriority(uart7_dma_rx.irq, 2);
#endif
#ifdef BSP_UART7_TX_USING_DMA
        struct dma_config uart7_dma_tx = UART7_TX_DMA_CONFIG;
        NVIC_SetPriority(uart7_dma_tx.irq, 2);
#endif
    }
#endif /* BSP_USING_UART7 */

}
#endif /* BSP_USING_UART */


#ifdef BSP_USING_SPI
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
void gd32_msp_spi_init (const uint32_t *periph)
{
#ifdef BSP_USING_SPI0
    if (*periph == SPI0)
    {
        /* configure gpio clock */
        rcu_periph_clock_enable(RCU_GPIOA);

        /* configure gpio */
        gpio_af_set(GPIOA, GPIO_AF_5, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);

        gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5 | GPIO_PIN_7);
        gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_5 | GPIO_PIN_7);

        gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6);
        gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_6);

        /* configure interrupt priority */
#ifdef BSP_SPI0_RX_USING_DMA
        struct dma_config spi0_dma_rx = SPI0_RX_DMA_CONFIG;
        NVIC_SetPriority(spi0_dma_rx.irq, 2);
#endif
#ifdef BSP_SPI0_TX_USING_DMA
        struct dma_config spi0_dma_tx = SPI0_TX_DMA_CONFIG;
        NVIC_SetPriority(spi0_dma_tx.irq, 2);
#endif
    }
#endif /* BSP_USING_SPI0 */

#ifdef BSP_USING_SPI1
    if (*periph == SPI1)
    {
        /* configure gpio clock */
        rcu_periph_clock_enable(RCU_GPIOB);

        /* configure gpio */
        gpio_af_set(GPIOB, GPIO_AF_5, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);

        gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13 | GPIO_PIN_15);
        gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_13 | GPIO_PIN_15);

        gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_14);
        gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_14);

        /* configure interrupt priority */
#ifdef BSP_SPI1_RX_USING_DMA
        struct dma_config spi1_dma_rx = SPI1_RX_DMA_CONFIG;
        NVIC_SetPriority(spi1_dma_rx.irq, 2);
#endif
#ifdef BSP_SPI1_TX_USING_DMA
        struct dma_config spi1_dma_tx = SPI1_TX_DMA_CONFIG;
        NVIC_SetPriority(spi1_dma_tx.irq, 2);
#endif
    }
#endif /* BSP_USING_SPI1 */

#ifdef BSP_USING_SPI2
    if (*periph == SPI2)
    {
        /* configure gpio clock */
        rcu_periph_clock_enable(RCU_GPIOC);

        /* configure gpio */
        gpio_af_set(GPIOC, GPIO_AF_6, GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);

        gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_10 | GPIO_PIN_12);
        gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_10 | GPIO_PIN_12);

        gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11);
        gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_11);

        /* configure interrupt priority */
#ifdef BSP_SPI2_RX_USING_DMA
        struct dma_config spi2_dma_rx = SPI2_RX_DMA_CONFIG;
        NVIC_SetPriority(spi2_dma_rx.irq, 2);
#endif
#ifdef BSP_SPI2_TX_USING_DMA
        struct dma_config spi2_dma_tx = SPI2_TX_DMA_CONFIG;
        NVIC_SetPriority(spi2_dma_tx.irq, 2);
#endif
    }
#endif /* BSP_USING_SPI2 */

#ifdef BSP_USING_SPI3
    if (*periph == SPI3)
    {
        /* configure gpio clock */
        rcu_periph_clock_enable(RCU_GPIOE);

        /* configure gpio */
        gpio_af_set(GPIOE, GPIO_AF_5, GPIO_PIN_2 | GPIO_PIN_5 | GPIO_PIN_6);

        gpio_mode_set(GPIOE, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_2 | GPIO_PIN_6);
        gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_2 | GPIO_PIN_6);

        gpio_mode_set(GPIOE, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5);
        gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_5);

        /* configure interrupt priority */
#ifdef BSP_SPI3_RX_USING_DMA
        struct dma_config spi3_dma_rx = SPI3_RX_DMA_CONFIG;
        NVIC_SetPriority(spi3_dma_rx.irq, 2);
#endif
#ifdef BSP_SPI3_TX_USING_DMA
        struct dma_config spi3_dma_tx = SPI3_TX_DMA_CONFIG;
        NVIC_SetPriority(spi3_dma_tx.irq, 2);
#endif
    }
#endif /* BSP_USING_SPI3 */

#ifdef BSP_USING_SPI4
    if (*periph == SPI4)
    {
        /* configure gpio clock */
        rcu_periph_clock_enable(RCU_GPIOF);

        /* configure gpio */
        gpio_af_set(GPIOF, GPIO_AF_5, GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9);

        gpio_mode_set(GPIOF, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7 | GPIO_PIN_9);
        gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_7 | GPIO_PIN_9);

        gpio_mode_set(GPIOF, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);
        gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_8);

        /* configure interrupt priority */
#ifdef BSP_SPI4_RX_USING_DMA
        struct dma_config spi4_dma_rx = SPI4_RX_DMA_CONFIG;
        NVIC_SetPriority(spi4_dma_rx.irq, 2);
#endif
#ifdef BSP_SPI4_TX_USING_DMA
        struct dma_config spi4_dma_tx = SPI4_TX_DMA_CONFIG;
        NVIC_SetPriority(spi4_dma_tx.irq, 2);
#endif
    }
#endif /* BSP_USING_SPI4 */

}
#endif /* BSP_USING_SPI */


#ifdef BSP_USING_SDIO
/**
 * @brief SDIO MSP Initialization
 *        This function configures the hardware resources used in this example:
 *           - Peripheral's GPIO Configuration
 *           - NVIC configuration for interrupt priority
 *        This function belongs to weak function, users can rewrite this function according to different needs
 *
 * @param periph peripherals in gd32_sdio_config
 * @return None
 */
void gd32_msp_sdio_init (const uint32_t *periph)
{
    /* configure gpio clock */
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOD);

    /* configure gpio */
    gpio_af_set(GPIOD, GPIO_AF_12, GPIO_PIN_2);
    gpio_af_set(GPIOC, GPIO_AF_12, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);

    gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_2);
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_12);
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11);

    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_2);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);

    /* configure interrupt priority */
    NVIC_SetPriority(SDIO_IRQn, 2);
}
#endif /* BSP_USING_SDIO */
