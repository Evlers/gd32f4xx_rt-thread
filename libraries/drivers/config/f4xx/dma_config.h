/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-03-20   Evlers      first implementation
 */

#ifndef _DMA_CONFIG_H_
#define _DMA_CONFIG_H_

#include <rtthread.h>

#ifdef __cplusplus
extern "C" {
#endif


/* DMA0 Channel0 */
#if defined(BSP_SPI2_RX_USING_DMA) && !defined(SPI2_RX_DMA_CONFIG)
#define SPI2_RX_DMA_CONFIG              DRV_DMA_CONFIG(0, 0, 0)
#define SPI2_DMA_RX_IRQHandler          DMA0_Channel0_IRQHandler
#elif defined(BSP_I2C0_RX_USING_DMA) && !defined(I2C0_RX_DMA_CONFIG)
#define I2C0_RX_DMA_CONFIG              DRV_DMA_CONFIG(0, 0, 1)
#define I2C0_DMA_RX_IRQHandler          DMA0_Channel0_IRQHandler
#elif defined(BSP_UART4_RX_USING_DMA) && !defined(UART4_RX_DMA_CONFIG)
#define UART4_RX_DMA_CONFIG             DRV_DMA_CONFIG(0, 0, 4)
#define UART4_DMA_RX_IRQHandler         DMA0_Channel0_IRQHandler
#elif defined(BSP_UART7_TX_USING_DMA) && !defined(UART7_TX_DMA_CONFIG)
#define UART7_TX_DMA_CONFIG             DRV_DMA_CONFIG(0, 0, 5)
#define UART7_DMA_RX_IRQHandler         DMA0_Channel0_IRQHandler
#endif

/* DMA0 Channel1 */
#if defined(BSP_UART2_RX_USING_DMA) && !defined(UART2_RX_DMA_CONFIG)
#define UART2_RX_DMA_CONFIG             DRV_DMA_CONFIG(0, 1, 4)
#define UART2_DMA_RX_IRQHandler         DMA0_Channel1_IRQHandler
#elif defined(BSP_UART6_TX_USING_DMA) && !defined(UART6_TX_DMA_CONFIG)
#define UART6_TX_DMA_CONFIG             DRV_DMA_CONFIG(0, 1, 5)
#define UART6_DMA_TX_IRQHandler         DMA0_Channel1_IRQHandler
#endif

/* DMA0 Channel2 */
#if defined(BSP_SPI2_RX_USING_DMA) && !defined(SPI2_RX_DMA_CONFIG)
#define SPI2_RX_DMA_CONFIG              DRV_DMA_CONFIG(0, 2, 0)
#define SPI2_DMA_RX_IRQHandler          DMA0_Channel2_IRQHandler
#elif defined(BSP_I2C2_RX_USING_DMA) && !defined(I2C2_RX_DMA_CONFIG)
#define I2C2_RX_DMA_CONFIG              DRV_DMA_CONFIG(0, 2, 3)
#define I2C2_DMA_RX_IRQHandler          DMA0_Channel2_IRQHandler
#elif defined(BSP_UART3_RX_USING_DMA) && !defined(UART3_RX_DMA_CONFIG)
#define UART3_RX_DMA_CONFIG             DRV_DMA_CONFIG(0, 2, 4)
#define UART3_DMA_RX_IRQHandler         DMA0_Channel2_IRQHandler
#elif defined(BSP_I2C1_RX_USING_DMA) && !defined(I2C1_RX_DMA_CONFIG)
#define I2C1_RX_DMA_CONFIG              DRV_DMA_CONFIG(0, 2, 7)
#define I2C1_DMA_RX_IRQHandler          DMA0_Channel2_IRQHandler
#endif

/* DMA0 Channel3 */
#if defined(BSP_SPI1_RX_USING_DMA) && !defined(SPI1_RX_DMA_CONFIG)
#define SPI1_RX_DMA_CONFIG              DRV_DMA_CONFIG(0, 3, 0)
#define SPI1_DMA_RX_IRQHandler          DMA0_Channel3_IRQHandler
#elif defined(BSP_UART2_TX_USING_DMA) && !defined(UART2_TX_DMA_CONFIG)
#define UART2_TX_DMA_CONFIG             DRV_DMA_CONFIG(0, 3, 4)
#define UART2_DMA_TX_IRQHandler         DMA0_Channel3_IRQHandler
#elif defined(BSP_UART6_RX_USING_DMA) && !defined(UART6_RX_DMA_CONFIG)
#define UART6_RX_DMA_CONFIG             DRV_DMA_CONFIG(0, 3, 5)
#define UART6_DMA_RX_IRQHandler         DMA0_Channel3_IRQHandler
#elif defined(BSP_I2C1_RX_USING_DMA) && !defined(I2C1_RX_DMA_CONFIG)
#define I2C1_RX_DMA_CONFIG              DRV_DMA_CONFIG(0, 3, 7)
#define I2C1_DMA_RX_IRQHandler          DMA0_Channel3_IRQHandler
#endif

/* DMA0 Channel4 */
#if defined(BSP_SPI1_TX_USING_DMA) && !defined(SPI1_TX_DMA_CONFIG)
#define SPI1_TX_DMA_CONFIG              DRV_DMA_CONFIG(0, 4, 0)
#define SPI1_DMA_TX_IRQHandler          DMA0_Channel4_IRQHandler
#elif defined(BSP_I2C2_TX_USING_DMA) && !defined(I2C2_TX_DMA_CONFIG)
#define I2C2_TX_DMA_CONFIG              DRV_DMA_CONFIG(0, 4, 3)
#define I2C2_DMA_TX_IRQHandler          DMA0_Channel4_IRQHandler
#elif defined(BSP_UART3_TX_USING_DMA) && !defined(UART3_TX_DMA_CONFIG)
#define UART3_TX_DMA_CONFIG             DRV_DMA_CONFIG(0, 4, 4)
#define UART3_DMA_TX_IRQHandler         DMA0_Channel4_IRQHandler
#elif defined(BSP_UART2_TX_USING_DMA) && !defined(UART2_TX_DMA_CONFIG)
#define UART2_TX_DMA_CONFIG             DRV_DMA_CONFIG(0, 4, 7)
#define UART2_DMA_TX_IRQHandler         DMA0_Channel4_IRQHandler
#endif

/* DMA0 Channel5 */
#if defined(BSP_SPI2_TX_USING_DMA) && !defined(SPI2_TX_DMA_CONFIG)
#define SPI2_TX_DMA_CONFIG              DRV_DMA_CONFIG(0, 5, 0)
#define SPI2_DMA_TX_IRQHandler          DMA0_Channel5_IRQHandler
#elif defined(BSP_I2C2_RX_USING_DMA) && !defined(I2C2_RX_DMA_CONFIG)
#define I2C2_RX_DMA_CONFIG              DRV_DMA_CONFIG(0, 5, 1)
#define I2C2_DMA_RX_IRQHandler          DMA0_Channel5_IRQHandler
#elif defined(BSP_UART1_RX_USING_DMA) && !defined(UART1_RX_DMA_CONFIG)
#define UART1_RX_DMA_CONFIG             DRV_DMA_CONFIG(0, 5, 4)
#define UART1_DMA_RX_IRQHandler         DMA0_Channel5_IRQHandler
#endif

/* DMA0 Channel6 */
#if defined(BSP_I2C0_TX_USING_DMA) && !defined(I2C0_TX_DMA_CONFIG)
#define I2C0_TX_DMA_CONFIG              DRV_DMA_CONFIG(0, 6, 1)
#define I2C0_DMA_TX_IRQHandler          DMA0_Channel6_IRQHandler
#elif defined(BSP_UART1_TX_USING_DMA) && !defined(UART1_TX_DMA_CONFIG)
#define UART1_TX_DMA_CONFIG             DRV_DMA_CONFIG(0, 6, 4)
#define UART1_DMA_TX_IRQHandler         DMA0_Channel6_IRQHandler
#elif defined(BSP_UART7_RX_USING_DMA) && !defined(UART7_RX_DMA_CONFIG)
#define UART7_RX_DMA_CONFIG             DRV_DMA_CONFIG(0, 6, 5)
#define UART7_DMA_RX_IRQHandler         DMA0_Channel6_IRQHandler
#endif

/* DMA0 Channel7 */
#if defined(BSP_SPI2_TX_USING_DMA) && !defined(SPI2_TX_DMA_CONFIG)
#define SPI2_TX_DMA_CONFIG              DRV_DMA_CONFIG(0, 7, 0)
#define SPI2_DMA_TX_IRQHandler          DMA0_Channel7_IRQHandler
#elif defined(BSP_I2C0_TX_USING_DMA) && !defined(I2C0_TX_DMA_CONFIG)
#define I2C0_TX_DMA_CONFIG              DRV_DMA_CONFIG(0, 7, 1)
#define I2C0_DMA_TX_IRQHandler          DMA0_Channel7_IRQHandler
#elif defined(BSP_UART4_TX_USING_DMA) && !defined(UART4_TX_DMA_CONFIG)
#define UART4_TX_DMA_CONFIG             DRV_DMA_CONFIG(0, 7, 4)
#define UART4_DMA_TX_IRQHandler         DMA0_Channel7_IRQHandler
#elif defined(BSP_I2C1_TX_USING_DMA) && !defined(I2C1_TX_DMA_CONFIG)
#define I2C1_TX_DMA_CONFIG              DRV_DMA_CONFIG(0, 7, 7)
#define I2C1_DMA_TX_IRQHandler          DMA0_Channel7_IRQHandler
#endif


/* DMA1 Channel0 */
#if defined(BSP_SPI0_RX_USING_DMA) && !defined(SPI0_RX_DMA_CONFIG)
#define SPI0_RX_DMA_CONFIG              DRV_DMA_CONFIG(1, 0, 3)
#define SPI0_DMA_RX_IRQHandler          DMA1_Channel0_IRQHandler
#elif defined(BSP_SPI3_RX_USING_DMA) && !defined(SPI3_RX_DMA_CONFIG)
#define SPI3_RX_DMA_CONFIG              DRV_DMA_CONFIG(1, 0, 4)
#define SPI3_DMA_RX_IRQHandler          DMA1_Channel0_IRQHandler
#endif

/* DMA1 Channel1 */
#if defined(BSP_SPI3_TX_USING_DMA) && !defined(SPI3_TX_DMA_CONFIG)
#define SPI3_TX_DMA_CONFIG              DRV_DMA_CONFIG(1, 1, 4)
#define SPI3_DMA_TX_IRQHandler          DMA1_Channel1_IRQHandler
#elif defined(BSP_UART5_RX_USING_DMA) && !defined(UART5_RX_DMA_CONFIG)
#define UART5_RX_DMA_CONFIG             DRV_DMA_CONFIG(1, 1, 5)
#define UART5_DMA_RX_IRQHandler         DMA1_Channel1_IRQHandler
#endif

/* DMA1 Channel2 */
#if defined(BSP_SPI0_RX_USING_DMA) && !defined(SPI0_RX_DMA_CONFIG)
#define SPI0_RX_DMA_CONFIG              DRV_DMA_CONFIG(1, 2, 3)
#define SPI0_DMA_RX_IRQHandler          DMA1_Channel2_IRQHandler
#elif defined(BSP_UART0_RX_USING_DMA) && !defined(UART0_RX_DMA_CONFIG)
#define UART0_RX_DMA_CONFIG             DRV_DMA_CONFIG(1, 2, 4)
#define UART0_DMA_RX_IRQHandler         DMA1_Channel2_IRQHandler
#elif defined(BSP_UART5_RX_USING_DMA) && !defined(UART5_RX_DMA_CONFIG)
#define UART5_RX_DMA_CONFIG             DRV_DMA_CONFIG(1, 2, 5)
#define UART5_DMA_RX_IRQHandler         DMA1_Channel2_IRQHandler
#endif

/* DMA1 Channel3 */
#if defined(BSP_SPI4_RX_USING_DMA) && !defined(SPI4_RX_DMA_CONFIG)
#define SPI4_RX_DMA_CONFIG              DRV_DMA_CONFIG(1, 3, 2)
#define SPI4_DMA_RX_IRQHandler          DMA1_Channel3_IRQHandler
#elif defined(BSP_SPI0_TX_USING_DMA) && !defined(SPI0_TX_DMA_CONFIG)
#define SPI0_TX_DMA_CONFIG              DRV_DMA_CONFIG(1, 3, 3)
#define SPI0_DMA_TX_IRQHandler          DMA1_Channel3_IRQHandler
#elif defined(BSP_USING_SDIO) && !defined(SDIO_DMA_CONFIG)
#define SDIO_DMA_CONFIG                 DRV_DMA_CONFIG(1, 3, 4)
#define SDIO_DMA_IRQHandler             DMA1_Channel3_IRQHandler
#elif defined(BSP_SPI3_RX_USING_DMA) && !defined(SPI3_RX_DMA_CONFIG)
#define SPI3_RX_DMA_CONFIG              DRV_DMA_CONFIG(1, 3, 5)
#define SPI3_DMA_RX_IRQHandler          DMA1_Channel3_IRQHandler
#endif

/* DMA1 Channel4 */
#if defined(BSP_SPI4_TX_USING_DMA) && !defined(SPI4_TX_DMA_CONFIG)
#define SPI4_TX_DMA_CONFIG              DRV_DMA_CONFIG(1, 4, 2)
#define SPI4_DMA_TX_IRQHandler          DMA1_Channel4_IRQHandler
#elif defined(BSP_SPI3_TX_USING_DMA) && !defined(SPI3_TX_DMA_CONFIG)
#define SPI3_TX_DMA_CONFIG              DRV_DMA_CONFIG(1, 4, 5)
#define SPI3_DMA_TX_IRQHandler          DMA1_Channel4_IRQHandler
#endif

/* DMA1 Channel5 */
#if defined(BSP_SPI5_TX_USING_DMA) && !defined(SPI5_TX_DMA_CONFIG)
#define SPI5_TX_DMA_CONFIG              DRV_DMA_CONFIG(1, 5, 1)
#define SPI5_DMA_TX_IRQHandler          DMA1_Channel5_IRQHandler
#elif defined(BSP_SPI0_TX_USING_DMA) && !defined(SPI0_TX_DMA_CONFIG)
#define SPI0_TX_DMA_CONFIG              DRV_DMA_CONFIG(1, 5, 3)
#define SPI0_DMA_TX_IRQHandler          DMA1_Channel5_IRQHandler
#elif defined(BSP_UART0_RX_USING_DMA) && !defined(UART0_RX_DMA_CONFIG)
#define UART0_RX_DMA_CONFIG             DRV_DMA_CONFIG(1, 5, 4)
#define UART0_DMA_RX_IRQHandler         DMA1_Channel5_IRQHandler
#elif defined(BSP_SPI4_RX_USING_DMA) && !defined(SPI4_RX_DMA_CONFIG)
#define SPI4_RX_DMA_CONFIG              DRV_DMA_CONFIG(1, 5, 7)
#define SPI4_DMA_RX_IRQHandler          DMA1_Channel5_IRQHandle
#endif

/* DMA1 Channel6 */
#if defined(BSP_SPI5_RX_USING_DMA) && !defined(SPI5_RX_DMA_CONFIG)
#define SPI5_RX_DMA_CONFIG              DRV_DMA_CONFIG(1, 6, 1)
#define SPI5_DMA_RX_IRQHandler          DMA1_Channel6_IRQHandler
#elif defined(BSP_USING_SDIO) && !defined(SDIO_DMA_CONFIG)
#define SDIO_DMA_CONFIG                 DRV_DMA_CONFIG(1, 6, 4)
#define SDIO_DMA_IRQHandler             DMA1_Channel6_IRQHandler
#elif defined(BSP_UART5_TX_USING_DMA) && !defined(UART5_TX_DMA_CONFIG)
#define UART5_TX_DMA_CONFIG             DRV_DMA_CONFIG(1, 6, 5)
#define UART5_DMA_TX_IRQHandler         DMA1_Channel6_IRQHandler
#elif defined(BSP_SPI4_TX_USING_DMA) && !defined(SPI4_TX_DMA_CONFIG)
#define SPI4_TX_DMA_CONFIG              DRV_DMA_CONFIG(1, 6, 7)
#define SPI4_DMA_TX_IRQHandler          DMA1_Channel6_IRQHandler
#endif

/* DMA1 Channel7 */
#if defined(BSP_UART0_TX_USING_DMA) && !defined(UART0_TX_DMA_CONFIG)
#define UART0_TX_DMA_CONFIG             DRV_DMA_CONFIG(1, 7, 4)
#define UART0_DMA_TX_IRQHandler         DMA1_Channel7_IRQHandler
#elif defined(BSP_UART5_TX_USING_DMA) && !defined(UART5_TX_DMA_CONFIG)
#define UART5_TX_DMA_CONFIG             DRV_DMA_CONFIG(1, 7, 5)
#define UART5_DMA_TX_IRQHandler         DMA1_Channel7_IRQHandler
#endif

#ifdef __cplusplus
}
#endif

#endif /* _DMA_CONFIG_H_ */
