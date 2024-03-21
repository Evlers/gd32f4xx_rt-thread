/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-03-20   Evlers      first implementation
 */

#ifndef _SPI_CONFIG_H_
#define _SPI_CONFIG_H_

#include <rtthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(BSP_USING_SPI0)
#ifndef SPI0_CONFIG
#define SPI0_CONFIG                                         \
    {                                                       \
        "spi0",                                             \
        SPI0,                                               \
        RCU_SPI0, RCU_GPIOA,                                \
        GPIOA, GPIO_AF_5,                                   \
        GPIO_PIN_5,                                         \
        GPIO_PIN_6,                                         \
        GPIO_PIN_7,                                         \
    }
#endif /* SPI0_CONFIG */
#endif /* BSP_USING_SPI0 */

#if defined(BSP_USING_SPI1)
#ifndef SPI1_CONFIG
#define SPI1_CONFIG                                         \
    {                                                       \
        "spi1",                                             \
        SPI1,                                               \
        RCU_SPI1, RCU_GPIOB,                                \
        GPIOB, GPIO_AF_5,                                   \
        GPIO_PIN_13,                                        \
        GPIO_PIN_14,                                        \
        GPIO_PIN_15,                                        \
    }
#endif /* SPI1_CONFIG */
#endif /* BSP_USING_SPI1 */

#if defined(BSP_USING_SPI2)
#ifndef SPI2_CONFIG
#define SPI2_CONFIG                                         \
    {                                                       \
        "spi2",                                             \
        SPI2,                                               \
        RCU_SPI2, RCU_GPIOB,                                \
        GPIOB, GPIO_AF_6,                                   \
        GPIO_PIN_3,                                         \
        GPIO_PIN_4,                                         \
        GPIO_PIN_5,                                         \
    }
#endif /* SPI2_CONFIG */
#endif /* BSP_USING_SPI2 */

#if defined(BSP_USING_SPI3)
#ifndef SPI3_CONFIG
#define SPI3_CONFIG                                         \
    {                                                       \
        "spi3",                                             \
        SPI3,                                               \
        RCU_SPI3, RCU_GPIOE,                                \
        GPIOE, GPIO_AF_5,                                   \
        GPIO_PIN_2,                                         \
        GPIO_PIN_5,                                         \
        GPIO_PIN_6,                                         \
    }
#endif /* SPI3_CONFIG */
#endif /* BSP_USING_SPI3 */

#if defined(BSP_USING_SPI4)
#ifndef SPI4_CONFIG
#define SPI4_CONFIG                                         \
    {                                                       \
        "spi4",                                             \
        SPI4,                                               \
        RCU_SPI4, RCU_GPIOF,                                \
        GPIOF, GPIO_AF_5,                                   \
        GPIO_PIN_7,                                         \
        GPIO_PIN_8,                                         \
        GPIO_PIN_9,                                         \
    }
#endif /* SPI4_CONFIG */
#endif /* BSP_USING_SPI4 */

#ifdef __cplusplus
}
#endif

#endif /* _SPI_CONFIG_H_ */
