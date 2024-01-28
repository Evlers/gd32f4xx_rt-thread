/*
 * Copyright (c) 2006-2023, Evlers Developers
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-01-27   Evlers      first implementation
 */

#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_

#include <rtthread.h>
#include <board.h>

#define GD32_ONCHIP_FLASH_ADDR          GD32_FLASH_START_ADRESS + GD32_FLASH_SIZE - GD32_ONCHIP_FLASH_SIZE
#define GD32_ONCHIP_FLASH_SIZE          (1024 * 1024)

extern const struct fal_flash_dev gd32_onchip_flash;

/* flash device table */
#define FAL_FLASH_DEV_TABLE                 \
{                                           \
    &gd32_onchip_flash,                     \
}

/* ====================== Partition Configuration ========================== */
#ifdef FAL_PART_HAS_TABLE_CFG

/* partition table */
/*      magic_word          partition name      flash name          offset          size            reserved        */
#define FAL_PART_TABLE                                                                                              \
{                                                                                                                   \
    { FAL_PART_MAGIC_WORD,  "whd_firmware",     "onchip_flash",     0,              448 * 1024,         0 },        \
    { FAL_PART_MAGIC_WORD,  "whd_clm",          "onchip_flash",     448 * 1024,     32 * 1024,          0 },        \
    { FAL_PART_MAGIC_WORD,  "easyflash",        "onchip_flash",     480 * 1024,     32 * 1024,          0 },        \
    { FAL_PART_MAGIC_WORD,  "filesystem",       "onchip_flash",     512 * 1024,     512 * 1024,         0 },        \
}

#endif /* FAL_PART_HAS_TABLE_CFG */
#endif /* _FAL_CFG_H_ */
