/*
 * Copyright (c) 2006-2023, Evlers Developers
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-01-27   Evlers      first implementation
 */

#include "rtthread.h"

#if defined(RT_USING_FAL) && defined(RT_USING_DFS)

#include "fal.h"
#include "dfs_fs.h"

#define DBG_TAG             "filesystem"
#define DBG_LVL             DBG_INFO
#include "rtdbg.h"

/* File system partition name on FAL partition table */
#define FAL_FS_PART_NAME                "filesystem"


static int filesystem_init (void)
{
    /* Create a block device using the flash abstraction layer */
    if (fal_partition_find(FAL_FS_PART_NAME) != NULL && fal_blk_device_create(FAL_FS_PART_NAME) != NULL)
    {
        /* Mount the FAT file system to the root directory */
        if (dfs_mount(FAL_FS_PART_NAME, "/", "elm", 0, 0) != 0)
        {
            if (!dfs_mkfs("elm", FAL_FS_PART_NAME))
            {
                if (dfs_mount(FAL_FS_PART_NAME, "/", "elm", 0, 0) != 0)
                {
                    LOG_E("The fat file system failed to be mounted!");
                }
            }
            else
            {
                LOG_E("FAT file system formatting failed!");
            }
        }
    }

    return RT_EOK;
}
INIT_ENV_EXPORT(filesystem_init);

#endif /* RT_USING_DFS */
