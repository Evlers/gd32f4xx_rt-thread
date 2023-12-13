/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-20     BruceOu      first implementation
 * 2023-03-05     yuanzihao    change the LED pins
 */

#include <stdio.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#include "fal.h"
#include "dfs_fs.h"

#define DBG_TAG             "main"
#define DBG_LVL             DBG_INFO
#include "rtdbg.h"

/* defined the LED1 pin: PC6 */
#define LED1_PIN GET_PIN(C, 6)

int main(void)
{
    int count = 1;

    fal_init();

    fal_blk_device_create("flash");
    if (dfs_mount("flash", "/", "elm", 0, 0) != 0)
    {
        LOG_I("Failed to mount the flash on the chip!");
    }

    /* set LED1 pin mode to output */
    rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);

    while (count++)
    {
        rt_pin_write(LED1_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED1_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }

    return RT_EOK;
}
