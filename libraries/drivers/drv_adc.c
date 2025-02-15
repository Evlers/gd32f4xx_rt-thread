/*
 * Copyright (c) 2006-2025 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2025-02-14     Evlers            first version
 */

#include "drv_adc.h"

#define DBG_TAG             "drv.adc"
#define DBG_LVL             DBG_INFO

#include <rtdbg.h>

#ifdef RT_USING_ADC

#if defined(BSP_USING_ADC0)
struct rt_adc_device adc0;
#endif

#if defined(BSP_USING_ADC1)
struct rt_adc_device adc1;
#endif

#if defined(BSP_USING_ADC2)
struct rt_adc_device adc2;
#endif

#define MAX_EXTERN_ADC_CHANNEL    16

static const rt_base_t adc_pins[MAX_EXTERN_ADC_CHANNEL] =
{
    GET_PIN(A, 0), GET_PIN(A, 1), GET_PIN(A, 2), GET_PIN(A, 3),
    GET_PIN(A, 4), GET_PIN(A, 5), GET_PIN(A, 6), GET_PIN(A, 7),
    GET_PIN(B, 0), GET_PIN(B, 1), GET_PIN(C, 0), GET_PIN(C, 1),
    GET_PIN(C, 2), GET_PIN(C, 3), GET_PIN(C, 4), GET_PIN(C, 5),
};

static struct gd32_adc adc_obj[] = {
#ifdef BSP_USING_ADC0
    {
        ADC0,
        RCU_ADC0,
        adc_pins,
        &adc0,
        "adc0",
    },
#endif

#ifdef BSP_USING_ADC1
    {
        ADC1,
        RCU_ADC1,
        adc_pins,
        &adc1,
        "adc1",
    },
#endif
#ifdef BSP_USING_ADC2
    {
        ADC2,
        RCU_ADC2,
        adc_pins,
        &adc2,
        "adc2",
    },
#endif
};

static rt_err_t gd32_adc_enabled(struct rt_adc_device *device, rt_int8_t channel, rt_bool_t enabled)
{
    uint32_t adc_periph;
    struct gd32_adc *adc = (struct gd32_adc *)device->parent.user_data;

    RT_ASSERT(channel >= 0 && channel < MAX_EXTERN_ADC_CHANNEL);

    if (channel >= MAX_EXTERN_ADC_CHANNEL)
    {
        LOG_E("invalid channel");
        return -RT_EINVAL;
    }

    adc_periph = (uint32_t)(adc->adc_periph);

    if (enabled == ENABLE)
    {
        /* configure adc pin */
#if defined SOC_SERIES_GD32F4xx
        gpio_mode_set(PIN_GDPORT(adc->adc_pins[channel]), GPIO_MODE_ANALOG, GPIO_PUPD_NONE, PIN_GDPIN(adc->adc_pins[channel]));
#else
        gpio_init(PIN_GDPORT(adc->adc_pins[channel]), GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, PIN_GDPIN(adc->adc_pins[channel]));
#endif

        if (adc->channel_en == 0)
        {
            /* enable ADC clock */
            rcu_periph_clock_enable(adc->adc_clk);

            /* adc alignment config */
            adc_data_alignment_config(adc_periph, ADC_DATAALIGN_RIGHT);

            /* adc config */
#if defined SOC_SERIES_GD32F4xx
            adc_channel_length_config(adc_periph, ADC_ROUTINE_CHANNEL, 1);
            adc_external_trigger_source_config(adc_periph, ADC_ROUTINE_CHANNEL, ADC_EXTTRIG_ROUTINE_EXTI_11);
            adc_external_trigger_config(adc_periph, ADC_ROUTINE_CHANNEL, ENABLE);
#else
            adc_channel_length_config(adc_periph, ADC_REGULAR_CHANNEL, 1);
            adc_external_trigger_source_config(adc_periph, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE);
            adc_external_trigger_config(adc_periph, ADC_REGULAR_CHANNEL, ENABLE);
#endif

            adc_enable(adc_periph);
            rt_thread_mdelay(1);

            /* ADC calibration and reset calibration */
            adc_calibration_enable(adc_periph);
        }

        adc->channel_en |= (1 << channel);
    }
    else
    {
        adc->channel_en &= ~(1 << channel);
        if (adc->channel_en == 0)
        {
            rcu_periph_clock_disable(adc->adc_clk);
            adc_disable(adc_periph);
        }
    }
    return RT_EOK;
}

static rt_err_t gd32_adc_convert(struct rt_adc_device *device, rt_int8_t channel, rt_uint32_t *value)
{
    uint32_t adc_periph;
    rt_tick_t tick;
    struct gd32_adc *adc = (struct gd32_adc *)(device->parent.user_data);

    if (!value)
    {
        LOG_E("invalid param");
        return -RT_EINVAL;
    }

    adc_periph = (uint32_t)(adc->adc_periph);
    adc_flag_clear(adc_periph, ADC_FLAG_EOC | ADC_FLAG_STRC);
#if defined SOC_SERIES_GD32F4xx
    adc_routine_channel_config(adc_periph, 0, channel, ADC_SAMPLETIME_480);
    adc_software_trigger_enable(adc_periph, ADC_ROUTINE_CHANNEL);
#else
    adc_regular_channel_config(adc_periph, 0, channel, ADC_SAMPLETIME_13POINT5);
    adc_software_trigger_enable(adc_periph, ADC_REGULAR_CHANNEL);
#endif

    tick = rt_tick_get();
    while (!adc_flag_get(adc_periph, ADC_FLAG_EOC))
    {
        if ((rt_tick_get() - tick) >= 10)
        {
            adc_flag_clear(adc_periph, ADC_FLAG_EOC | ADC_FLAG_STRC);
            LOG_E("convert timeout");
            return -RT_ETIMEOUT;
        }
    }

#if defined SOC_SERIES_GD32F4xx
    *value = adc_routine_data_read(adc_periph);
#else
    *value = adc_regular_data_read(adc_periph);
#endif

    adc_flag_clear(adc_periph, ADC_FLAG_EOC | ADC_FLAG_STRC);
    return RT_EOK;
}

static struct rt_adc_ops gd32_adc_ops =
{
    .enabled = gd32_adc_enabled,
    .convert = gd32_adc_convert,
};

static int rt_hw_adc_init(void)
{
    int ret;

    for (int i = 0; i < sizeof(adc_obj) / sizeof(adc_obj[0]); i++)
    {
        ret = rt_hw_adc_register(adc_obj[i].adc,
                                    adc_obj[i].device_name,
                                    &gd32_adc_ops, &adc_obj[i]);
        if (ret != RT_EOK)
        {
            LOG_E("failed register %s, err=%d", adc_obj[i].device_name, ret);
        }
    }

    return ret;
}
INIT_BOARD_EXPORT(rt_hw_adc_init);
#endif
