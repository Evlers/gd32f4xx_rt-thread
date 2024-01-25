/*
 * Copyright (c) 2006-2023, Evlers Developers
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-01-11   Evlers      first implementation
 */

#include "rtthread.h"

#define DBG_TAG             "fal.port"
#define DBG_LVL             DBG_INFO
#include "rtdbg.h"

#ifdef RT_USING_FAL

#include "fal.h"
#include "dfs_fs.h"
#include "easyflash.h"


static int rt_fal_init (void)
{
    /* Initializes the flash abstraction layer */
    fal_init();

#ifdef RT_USING_DFS
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
#endif /* RT_USING_DFS */

#ifdef PKG_USING_EASYFLASH
    /* Initializes the easyflash */
    easyflash_init();
#endif /* PKG_USING_EASYFLASH */

    return RT_EOK;
}
INIT_ENV_EXPORT(rt_fal_init);

#endif /* RT_USING_FAL */
