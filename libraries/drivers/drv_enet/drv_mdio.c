/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-06-13   Evlers      first implementation
 */

#include "rtthread.h"

#ifdef RT_USING_PHY

#define LOG_TAG "drv.mdio"
#include "drv_log.h"

#include "rtdevice.h"
#include "drv_mdio.h"

#include "board.h"

static rt_bool_t rt_hw_mdio_init (void *bus, rt_uint32_t src_clock_hz)
{
    uint32_t ahbclk;
    uint32_t reg;

    /* clear the previous MDC clock */
    reg = ENET_MAC_PHY_CTL;
    reg &= ~ENET_MAC_PHY_CTL_CLR;

    /* get the HCLK frequency */
    ahbclk = rcu_clock_freq_get(CK_AHB);

    /* configure MDC clock according to HCLK frequency range */
    if (ENET_RANGE(ahbclk, 20000000U, 35000000U)) {
        reg |= ENET_MDC_HCLK_DIV16;
    } else if (ENET_RANGE(ahbclk, 35000000U, 60000000U)) {
        reg |= ENET_MDC_HCLK_DIV26;
    } else if (ENET_RANGE(ahbclk, 60000000U, 100000000U)) {
        reg |= ENET_MDC_HCLK_DIV42;
    } else if (ENET_RANGE(ahbclk, 100000000U, 150000000U)) {
        reg |= ENET_MDC_HCLK_DIV62;
    } else if ((ENET_RANGE(ahbclk, 150000000U, 240000000U)) || (240000000U == ahbclk)) {
        reg |= ENET_MDC_HCLK_DIV102;
    } else {
        return -RT_ERROR;
    }
    ENET_MAC_PHY_CTL = reg;

    return RT_TRUE;
}

static rt_size_t rt_hw_mdio_read (void *bus, rt_uint32_t addr, rt_uint32_t reg, void *data, rt_uint32_t size)
{
    RT_ASSERT(data != NULL);

    if (enet_phy_write_read(ENET_PHY_READ, addr, reg, data) != SUCCESS)
    {
        return 0;
    }

    return 2;
}

static rt_size_t rt_hw_mdio_write (void *bus, rt_uint32_t addr, rt_uint32_t reg, void *data, rt_uint32_t size)
{
    RT_ASSERT(data != NULL);

    if (enet_phy_write_read(ENET_PHY_READ, addr, reg, data) != SUCCESS)
    {
        return 0;
    }

    return size;
}

static struct rt_mdio_bus_ops gd32_mdio_ops =
{
    .init = rt_hw_mdio_init,
    .read = rt_hw_mdio_read,
    .write = rt_hw_mdio_write,
    .uninit = RT_NULL,
};

static rt_mdio_t mdio_bus;

rt_mdio_t *rt_hw_mdio_register (void *hw_obj, char *name)
{
    mdio_bus.hw_obj = hw_obj;
    mdio_bus.name = name;
    mdio_bus.ops = &gd32_mdio_ops;
    return &mdio_bus;
}

#endif
