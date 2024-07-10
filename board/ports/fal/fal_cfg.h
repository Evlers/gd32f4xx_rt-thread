/*
 * Copyright (c) 2006-2023, Evlers Developers
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-01-27   Evlers      first implementation
 * 2024-07-10   Evlers      add support for nor flash
 */

#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_

#include <rtthread.h>
#include <board.h>

#define NOR_FLASH_DEV_NAME              "norflash"
#define ONCHIP_FLASH_DEV_NAME           "onchip_flash"
#define GD32_ONCHIP_FLASH_ADDR          GD32_FLASH_START_ADRESS
#define GD32_ONCHIP_FLASH_SIZE          GD32_FLASH_SIZE

extern struct fal_flash_dev nor_flash;
extern const struct fal_flash_dev gd32_onchip_flash;

/* flash device table */
#define FAL_FLASH_DEV_TABLE                 \
{                                           \
    &nor_flash,                             \
    &gd32_onchip_flash,                     \
}

/* ====================== Partition Configuration ========================== */
#ifdef FAL_PART_HAS_TABLE_CFG

/* partition table */
/*      magic_word          partition name      flash device name           offset          size            reserved        */
#define FAL_PART_TABLE                                                                                                      \
{                                                                                                                           \
    { FAL_PART_MAGIC_WORD,  "application",      ONCHIP_FLASH_DEV_NAME,               0,   2048 * 1024,          0 },        \
    { FAL_PART_MAGIC_WORD,  "whd_firmware",     NOR_FLASH_DEV_NAME,                  0,   1024 * 1024,          0 },        \
    { FAL_PART_MAGIC_WORD,  "whd_clm",          NOR_FLASH_DEV_NAME,        1024 * 1024,   1024 * 1024,          0 },        \
    { FAL_PART_MAGIC_WORD,  "easyflash",        NOR_FLASH_DEV_NAME,        2048 * 1024,     64 * 1024,          0 },        \
    { FAL_PART_MAGIC_WORD,  "filesystem",       NOR_FLASH_DEV_NAME,        3072 * 1024,  13312 * 1024,          0 },        \
}

#endif /* FAL_PART_HAS_TABLE_CFG */
#endif /* _FAL_CFG_H_ */
