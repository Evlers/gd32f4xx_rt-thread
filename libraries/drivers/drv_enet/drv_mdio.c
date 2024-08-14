/*
 * Copyright (c) 2006-2024 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-06-13   Evlers      first implementation
 * 2024-07-22   Evlers      fix bug for register write
 */

#include <stdio.h>
#include "rtthread.h"

#ifdef RT_USING_PHY

#define LOG_TAG         "drv.mdio"
#define DBG_LVL         DBG_INFO
#include "rtdbg.h"

#include "rtdevice.h"
#include "drv_mdio.h"

#include "board.h"

#ifdef BSP_ETH_COMMAND_LINE_DEBUG
static rt_mutex_t mutex = RT_NULL;
#endif

static rt_bool_t rt_hw_mdio_init (void *bus, rt_uint32_t src_clock_hz)
{
    uint32_t reg;

    /* clear the previous MDC clock */
    reg = ENET_MAC_PHY_CTL;
    reg &= ~ENET_MAC_PHY_CTL_CLR;

    /* configure MDC clock according to HCLK frequency range */
    if (ENET_RANGE(src_clock_hz, 20000000U, 35000000U))
    {
        reg |= ENET_MDC_HCLK_DIV16;
    }
    else if (ENET_RANGE(src_clock_hz, 35000000U, 60000000U))
    {
        reg |= ENET_MDC_HCLK_DIV26;
    }
    else if (ENET_RANGE(src_clock_hz, 60000000U, 100000000U))
    {
        reg |= ENET_MDC_HCLK_DIV42;
    }
    else if (ENET_RANGE(src_clock_hz, 100000000U, 150000000U))
    {
        reg |= ENET_MDC_HCLK_DIV62;
    }
    else if ((ENET_RANGE(src_clock_hz, 150000000U, 240000000U)) || (240000000U == src_clock_hz))
    {
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

#ifdef BSP_ETH_COMMAND_LINE_DEBUG
    rt_mutex_take(mutex, RT_WAITING_FOREVER);
#endif

    if (enet_phy_write_read(ENET_PHY_READ, addr, reg, data) != SUCCESS)
    {
#ifdef BSP_ETH_COMMAND_LINE_DEBUG
        rt_mutex_release(mutex);
#endif
        return 0;
    }

#ifdef BSP_ETH_COMMAND_LINE_DEBUG
    rt_mutex_release(mutex);
#endif

    return 2;
}

static rt_size_t rt_hw_mdio_write (void *bus, rt_uint32_t addr, rt_uint32_t reg, void *data, rt_uint32_t size)
{
    RT_ASSERT(data != NULL);

#ifdef BSP_ETH_COMMAND_LINE_DEBUG
    rt_mutex_take(mutex, RT_WAITING_FOREVER);
#endif

    if (enet_phy_write_read(ENET_PHY_WRITE, addr, reg, data) != SUCCESS)
    {
#ifdef BSP_ETH_COMMAND_LINE_DEBUG
        rt_mutex_release(mutex);
#endif
        return 0;
    }

#ifdef BSP_ETH_COMMAND_LINE_DEBUG
    rt_mutex_release(mutex);
#endif

    return size;
}

static struct rt_mdio_bus_ops gd32_mdio_ops =
{
    .init = rt_hw_mdio_init,
    .read = rt_hw_mdio_read,
    .write = rt_hw_mdio_write,
    .uninit = RT_NULL,
};

static rt_mdio_t mdio_bus = { 0 };

rt_mdio_t *rt_hw_mdio_register (void *hw_obj, char *name)
{
#ifdef BSP_ETH_COMMAND_LINE_DEBUG
    mutex = rt_mutex_create("mdio", RT_IPC_FLAG_PRIO);
    RT_ASSERT(mutex != RT_NULL);
#endif
    mdio_bus.hw_obj = hw_obj;
    mdio_bus.name = name;
    mdio_bus.ops = &gd32_mdio_ops;
    return &mdio_bus;
}

#ifdef BSP_ETH_COMMAND_LINE_DEBUG

static void print_help (void)
{
    rt_kprintf("using: mdio [command]\n");
    rt_kprintf("the following is the command description:\n");
    rt_kprintf("\tread\tread register value of the phy chips. [address]\n");
    rt_kprintf("\twrite\twrite register value of the phy chips. [address] [value(hex)]\n");
}

static int mdio_msh (int argc, char **argv)
{
    if (argc < 2)
    {
        __help:
        print_help();
        return -RT_ERROR;
    }

    if (mdio_bus.ops == NULL)
    {
        rt_kprintf("mdio is not registered!\n");
        return -RT_ERROR;
    }

    if (!strcmp("read", argv[1]))
    {
        uint32_t reg;
        uint16_t data;
        if (argc < 3) goto __help;

        reg = atol(argv[2]);

        if (mdio_bus.ops->read(&mdio_bus, BSP_USING_ENET_PHY_ADDR, reg, &data, sizeof(data)) != sizeof(data))
        {
            rt_kprintf("read error!\n");
        }
        rt_kprintf("read address: %u, data: %04X\n", reg, data);
    }

    if (!strcmp("write", argv[1]))
    {
        uint32_t reg;
        uint16_t data;
        uint32_t tmp = 0;
        if (argc < 3) goto __help;

        reg = atol(argv[2]);
        sscanf(argv[3], "%x", &tmp);
        data = tmp;

        if (mdio_bus.ops->write(&mdio_bus, BSP_USING_ENET_PHY_ADDR, reg, &data, sizeof(data)) != sizeof(data))
        {
            rt_kprintf("write error!\n");
        }

        rt_kprintf("write address: %u, data: %04x\n", reg, data);
    }

    return RT_EOK;
}
MSH_CMD_EXPORT_ALIAS(mdio_msh, mdio, mdio control command);

#endif /* BSP_ETH_COMMAND_LINE_DEBUG */

#endif /* RT_USING_PHY */
