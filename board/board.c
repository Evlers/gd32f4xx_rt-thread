/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-07-10   Evlers      first implementation
 * 2024-08-14   Evlers      add independent interrupt management
 * 2024-09-20   Evlers      add support for SEGGER Real Time Transfer (RTT)
 */

#include <stdint.h>
#include <rthw.h>
#include <rtthread.h>
#include <board.h>

#ifdef RT_USING_SERIAL
#include "drv_usart.h"
#endif

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler */
    /* User can add his own implementation to report the HAL error return state */
    while (1)
    {
    }
    /* USER CODE END Error_Handler */
}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{
    SysTick_Config(SystemCoreClock / RT_TICK_PER_SECOND);
}

/**
 * This is the timer interrupt service routine.
 *
 */
void SysTick_Handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    rt_tick_increase();

    /* leave interrupt */
    rt_interrupt_leave();
}

/**
 * This function will initial GD32 board.
 */
void rt_hw_board_init()
{
    /* NVIC Configuration */
#define NVIC_VTOR_MASK              0x3FFFFF80
#ifdef  VECT_TAB_RAM
    /* Set the Vector Table base location at 0x10000000 */
    SCB->VTOR  = (0x10000000 & NVIC_VTOR_MASK);
#else  /* VECT_TAB_FLASH  */
    /* Set the Vector Table base location at 0x08000000 */
    SCB->VTOR  = (0x08000000 & NVIC_VTOR_MASK);
#endif

    SystemClock_Config();

#ifdef RT_USING_HEAP
    rt_system_heap_init((void *)HEAP_BEGIN, (void *)HEAP_END);
#endif

#ifdef RT_USING_PIN
    rt_hw_pin_init();
#endif

    /* usart driver initialization is open by default */
#ifdef RT_USING_SERIAL
    rt_hw_usart_init();
#endif

#ifdef PKG_USING_SEGGER_RTT
    int rt_hw_segger_rtt_init(void);
    rt_hw_segger_rtt_init();
#endif

#if defined(RT_USING_CONSOLE) && defined(RT_USING_DEVICE)
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif

#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif
}

#ifdef RT_USING_INDEPENDENT_INTERRUPT_MANAGEMENT
#define RT_NVIC_PRO_BITS    __NVIC_PRIO_BITS

rt_base_t rt_hw_interrupt_disable(void)
{
    rt_base_t level = __get_BASEPRI();
    __set_BASEPRI(RT_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - RT_NVIC_PRO_BITS));

    __ISB();
    __DSB();

    return level;
}

void rt_hw_interrupt_enable(rt_base_t level)
{
    __set_BASEPRI(level);
}

#endif /* RT_USING_INDEPENDENT_INTERRUPT_MANAGEMENT */

