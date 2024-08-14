/*
 * Copyright (c) 2006-2024 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-03-20   Evlers      first implementation
 */

#ifndef _DRV_CONFIG_H_
#define _DRV_CONFIG_H_

#include <rtthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(SOC_SERIES_GD32F4xx)
#include "f4xx/dma_config.h"
#include "f4xx/spi_config.h"
#include "f4xx/uart_config.h"
#include "f4xx/sdio_config.h"
#elif defined(SOC_SERIES_GD32F30x)
#include "f30x/dma_config.h"
#endif

#ifdef __cplusplus
}
#endif

#endif /* _DRV_CONFIG_H_ */
