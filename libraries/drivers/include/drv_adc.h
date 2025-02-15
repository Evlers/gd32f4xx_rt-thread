/*
 * Copyright (c) 2006-2025 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2025-02-14     Evlers            first version
 */

#ifndef __DRV_ADC_H__
#define __DRV_ADC_H__

#include <rthw.h>
#include <rtthread.h>
#include <board.h>
#include "drv_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/* gd32 adc dirver class */
struct gd32_adc
{
    uint32_t adc_periph;
    rcu_periph_enum adc_clk;
    const rt_base_t *adc_pins;
    struct rt_adc_device *adc;
    const char *device_name;
    uint16_t channel_en;
};

#ifdef __cplusplus
}
#endif

#endif /* __DRV_ADC_H__ */
