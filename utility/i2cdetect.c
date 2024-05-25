/*
 * Copyright (c) 2006-2023, Evlers Developers
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-02-02   Evlers      first implementation
 */

#include "rtthread.h"

#if defined(RT_USING_DEVICE) && defined(RT_USING_I2C)

#include "rtdevice.h"

static void i2cdetect (int argc, char **argv)
{
    struct rt_i2c_bus_device *bus;

    if (argc < 2)
    {
        rt_kprintf("Using: i2cdetect <bus name>\n");
        return ;
    }

    if ((bus = (struct rt_i2c_bus_device *)rt_device_find(argv[1])) == NULL)
    {
        rt_kprintf("%s bus not found!\n", argv[1]);
        return ;
    }

    rt_kprintf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f");

    for (uint8_t i = 0; i < 128; i ++)
    {
        struct rt_i2c_msg msg =
        {
            .addr = i,
            .flags = RT_I2C_WR,
            .buf = (rt_uint8_t *)NULL,
            .len = 0,
        };

        if (!(i % 16))
        {
            rt_kprintf("\n%02x: ", i);
        }

        if (rt_i2c_transfer(bus, &msg, 1) >= 0)
        {
            rt_kprintf("%02x ", i);
        }
        else
        {
            rt_kprintf("-- ");
        }
    }

    rt_kprintf("\n");
}
MSH_CMD_EXPORT(i2cdetect, Detect for I2C bus devices);

#endif /* defined(RT_USING_DEVICE) && defined(RT_USING_I2C) */
