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
#include "easyflash.h"

#define DBG_TAG             "main"
#define DBG_LVL             DBG_INFO
#include "rtdbg.h"

/* defined the LED1 pin: PC6 */
#define LED1_PIN GET_PIN(C, 6)

int main(void)
{
    int count = 1;

    /* Initializes the flash abstraction layer */
    fal_init();

    /* Create a block device using the flash abstraction layer */
    fal_blk_device_create("root");

    /* Mount the FAT file system to the root directory */
    if (dfs_mount("root", "/", "elm", 0, 0) != 0)
    {
        if (!dfs_mkfs("elm", "root")) 
        {
            if (dfs_mount("root", "/", "elm", 0, 0) != 0)
            {
                LOG_E("The fat file system failed to be mounted!");
            }
        }
        else
        {
            LOG_E("FAT file system formatting failed!");
        }
    }

    /* Initializes the easyflash */
    easyflash_init();

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
