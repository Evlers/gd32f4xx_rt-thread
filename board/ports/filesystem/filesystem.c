/*
 * Copyright (c) 2006-2023, Evlers Developers
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-01-27   Evlers      first implementation
 * 2024-07-14   Evlers      add support for sdcard and romfs
 */

#include "rtthread.h"

#ifdef RT_USING_DFS

#include "dfs_fs.h"

#ifdef RT_USING_FAL
#include "fal.h"
#endif
#ifdef RT_USING_DFS_ROMFS
#include "dfs_romfs.h"
#else
#if defined(BSP_USING_SDCARD_FS) && defined(BSP_USING_SPI_FLASH_FS)
#error "If two file systems are enabled, ROMFS must be enabled"
#endif
#endif

#define DBG_TAG             "filesystem"
#define DBG_LVL             DBG_INFO
#include "rtdbg.h"

/* File system partition name on FAL partition table */
#ifndef SPI_FLASH_FS_PART_NAME
#define SPI_FLASH_FS_PART_NAME      "filesystem"
#endif

#ifdef RT_USING_DFS_ROMFS
static const struct romfs_dirent _romfs_root[] = {
    {ROMFS_DIRENT_DIR, "flash", RT_NULL, 0},
    {ROMFS_DIRENT_DIR, "sdcard", RT_NULL, 0}};

const struct romfs_dirent romfs_root = {
    ROMFS_DIRENT_DIR, "/", (rt_uint8_t *)_romfs_root, sizeof(_romfs_root) / sizeof(_romfs_root[0])};

static void mount_romfs (void)
{
    if (dfs_mount(RT_NULL, "/", "rom", 0, &(romfs_root)) != 0)
    {
        LOG_E("rom mount to '/' failed!");
    }
}
#endif /* RT_USING_DFS_ROMFS */

#ifdef BSP_USING_SDCARD_FS

static void sd_mount(void *parameter)
{
    char *path = parameter;

    mmcsd_wait_cd_changed(RT_WAITING_FOREVER);
    if (rt_device_find(SDCARD_FS_BLK_DEV_NAME) == RT_NULL)
    {
        LOG_W("No block device found for '%s'", SDCARD_FS_BLK_DEV_NAME);
        return;
    }

    if (dfs_mount(SDCARD_FS_BLK_DEV_NAME, path, "elm", 0, 0) == RT_EOK)
    {
        LOG_I("sd card mount to '%s'", path);
    }
    else
    {
        LOG_W("sd card mount to '%s' failed!", path);
    }
}

#endif /* BSP_USING_SDCARD_FS */

static int filesystem_init (void)
{
#ifndef RT_USING_DFS_ROMFS
    const char *flash_path = "/";
#else
    const char *flash_path = "/flash";
    mount_romfs();
#endif

#ifdef BSP_USING_SPI_FLASH_FS
    /* Create a block device using the flash abstraction layer */
    if (fal_partition_find(SPI_FLASH_FS_PART_NAME) != NULL && fal_blk_device_create(SPI_FLASH_FS_PART_NAME) != NULL)
    {
        /* Mount the FAT file system to the root directory */
        if (dfs_mount(SPI_FLASH_FS_PART_NAME, flash_path, "elm", 0, 0) != 0)
        {
            if (!dfs_mkfs("elm", SPI_FLASH_FS_PART_NAME))
            {
                if (dfs_mount(SPI_FLASH_FS_PART_NAME, flash_path, "elm", 0, 0) != 0)
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
#endif

#ifdef BSP_USING_SDCARD_FS
#ifndef RT_USING_DFS_ROMFS
    char *sdcard_path = "/";
#else
    char *sdcard_path = "/sdcard";
#endif
    rt_thread_startup(rt_thread_create("sd_mount", sd_mount, sdcard_path,
                                        2048, RT_THREAD_PRIORITY_MAX - 2, 20));
#endif

    return RT_EOK;
}
INIT_ENV_EXPORT(filesystem_init);

#endif /* RT_USING_DFS */
