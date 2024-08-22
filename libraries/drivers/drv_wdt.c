/*
 * Copyright (c) 2006-2024 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-08-22   Evlers      first implementation
 */

#include <board.h>

#define DBG_TAG             "drv.wdt"
#define DBG_LVL             DBG_INFO
#include <rtdbg.h>

#ifdef RT_USING_WDT

#define FWDGT_CLK_SRC_FREQ  32000

typedef struct
{
    struct rt_watchdog_device wdt;
    rt_uint32_t min_threshold;
    rt_uint32_t max_threshold;
    rt_uint32_t current_threshold;
} gd32_wdt_device_t;

static gd32_wdt_device_t wdt_dev;

static rt_err_t gd32_wdt_init(rt_watchdog_t *wdt)
{
    wdt_dev.min_threshold = 1;
    wdt_dev.max_threshold = (0xfff << 8) / FWDGT_CLK_SRC_FREQ;
    LOG_D("threshold range [%u, %d]", wdt_dev.min_threshold, wdt_dev.max_threshold);

    fwdgt_write_enable();
    fwdgt_config(0xfff, FWDGT_PSC_DIV256);
    fwdgt_enable();

    return 0;
}

static rt_err_t gd32_wdt_control(rt_watchdog_t *wdt, int cmd, void *arg)
{
    switch (cmd)
    {
    case RT_DEVICE_CTRL_WDT_KEEPALIVE:
        fwdgt_counter_reload();
        break;
    case RT_DEVICE_CTRL_WDT_SET_TIMEOUT:
        if ((*(rt_uint32_t *)arg > wdt_dev.max_threshold) || (*(rt_uint32_t *)arg < wdt_dev.min_threshold))
        {
            LOG_E("invalid param@%u.", *(rt_uint32_t *)arg);
            return -RT_EINVAL;
        }
        else
        {
            wdt_dev.current_threshold = *(rt_uint32_t *)arg;
        }
        fwdgt_write_enable();
        fwdgt_config(*(rt_uint32_t *)arg * FWDGT_CLK_SRC_FREQ >> 8, FWDGT_PSC_DIV256);
        fwdgt_write_disable();
        break;
    case RT_DEVICE_CTRL_WDT_GET_TIMEOUT:
        *(rt_uint32_t *)arg = wdt_dev.current_threshold;
        break;
    case RT_DEVICE_CTRL_WDT_START:
        fwdgt_enable();
        break;
    default:
        LOG_W("this command is not supported.");
        return -RT_ERROR;
    }

    return RT_EOK;
}

static struct rt_watchdog_ops wdt_ops =
{
    gd32_wdt_init,
    gd32_wdt_control,
};

static int rt_hw_wdt_init(void)
{
    wdt_dev.wdt.ops = &wdt_ops;

    /* register watchdog device */
    if (rt_hw_watchdog_register(&wdt_dev.wdt, "wdt", RT_DEVICE_FLAG_DEACTIVATE, RT_NULL) != RT_EOK)
    {
        LOG_E("wdt device register failed.");
        return -RT_ERROR;
    }
    LOG_D("wdt device register success.");

    return RT_EOK;
}
INIT_PREV_EXPORT(rt_hw_wdt_init);
#endif
