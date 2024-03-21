/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-03-20   Evlers      first implementation
 */

#ifndef _UART_CONFIG_H_
#define _UART_CONFIG_H_

#include <rtthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(BSP_USING_UART0)
#ifndef UART0_CONFIG
#define UART0_CONFIG                                        \
    {                                                       \
        "uart0",                                            \
        USART0,                                             \
        USART0_IRQn,                                        \
        RCU_USART0, RCU_GPIOA, RCU_GPIOA,                   \
        GPIOA, GPIO_AF_7, GPIO_PIN_9,                       \
        GPIOA, GPIO_AF_7, GPIO_PIN_10,                      \
    }
#endif /* UART0_CONFIG */
#endif /* BSP_USING_UART0 */

#if defined(BSP_USING_UART1)
#ifndef UART1_CONFIG
#define UART1_CONFIG                                        \
    {                                                       \
        "uart1",                                            \
        USART1,                                             \
        USART1_IRQn,                                        \
        RCU_USART1, RCU_GPIOA, RCU_GPIOA,                   \
        GPIOB, GPIO_AF_7, GPIO_PIN_10,                      \
        GPIOB, GPIO_AF_7, GPIO_PIN_11,                      \
    }
#endif /* UART1_CONFIG */
#endif /* BSP_USING_UART1 */

#if defined(BSP_USING_UART2)
#ifndef UART2_CONFIG
#define UART2_CONFIG                                        \
    {                                                       \
        "uart2",                                            \
        USART2,                                             \
        USART2_IRQn,                                        \
        RCU_USART2, RCU_GPIOB, RCU_GPIOB,                   \
        GPIOB, GPIO_AF_7, GPIO_PIN_10,                      \
        GPIOB, GPIO_AF_7, GPIO_PIN_11,                      \
    }
#endif /* UART2_CONFIG */
#endif /* BSP_USING_UART2 */

#if defined(BSP_USING_UART3)
#ifndef UART3_CONFIG
#define UART3_CONFIG                                        \
    {                                                       \
        "uart3",                                            \
        UART3,                                              \
        UART3_IRQn,                                         \
        RCU_UART3, RCU_GPIOC, RCU_GPIOC,                    \
        GPIOC, GPIO_AF_8, GPIO_PIN_10,                      \
        GPIOC, GPIO_AF_8, GPIO_PIN_11,                      \
    }
#endif /* UART3_CONFIG */
#endif /* BSP_USING_UART3 */

#if defined(BSP_USING_UART4)
#ifndef UART4_CONFIG
#define UART4_CONFIG                                        \
    {                                                       \
        "uart4",                                            \
        UART4,                                              \
        UART4_IRQn,                                         \
        RCU_UART4, RCU_GPIOC, RCU_GPIOD,                    \
        GPIOC, GPIO_AF_8, GPIO_PIN_12,                      \
        GPIOD, GPIO_AF_8, GPIO_PIN_2,                       \
    }
#endif /* UART4_CONFIG */
#endif /* BSP_USING_UART4 */

#if defined(BSP_USING_UART5)
#ifndef UART5_CONFIG
#define UART5_CONFIG                                        \
    {                                                       \
        "uart5",                                            \
        USART5,                                             \
        USART5_IRQn,                                        \
        RCU_USART5, RCU_GPIOC, RCU_GPIOC,                   \
        GPIOC, GPIO_AF_8, GPIO_PIN_6,                       \
        GPIOC, GPIO_AF_8, GPIO_PIN_7,                       \
    }
#endif /* UART5_CONFIG */
#endif /* BSP_USING_UART5 */

#if defined(BSP_USING_UART6)
#ifndef UART6_CONFIG
#define UART6_CONFIG                                        \
    {                                                       \
        "uart6",                                            \
        UART6,                                              \
        UART6_IRQn,                                         \
        RCU_UART6, RCU_GPIOE, RCU_GPIOE,                    \
        GPIOE, GPIO_AF_8, GPIO_PIN_7,                       \
        GPIOE, GPIO_AF_8, GPIO_PIN_8,                       \
    }
#endif /* UART6_CONFIG */
#endif /* BSP_USING_UART6 */

#if defined(BSP_USING_UART7)
#ifndef UART7_CONFIG
#define UART7_CONFIG                                        \
    {                                                       \
        "uart7",                                            \
        UART7,                                              \
        UART7_IRQn,                                         \
        RCU_UART7, RCU_GPIOE, RCU_GPIOE,                    \
        GPIOE, GPIO_AF_8, GPIO_PIN_0,                       \
        GPIOE, GPIO_AF_8, GPIO_PIN_1,                       \
    }
#endif /* UART7_CONFIG */
#endif /* BSP_USING_UART7 */

#ifdef __cplusplus
}
#endif

#endif /* _UART_CONFIG_H_ */
