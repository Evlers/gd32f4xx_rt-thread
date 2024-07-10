/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date             Author      Notes
 * 2024-06-04       Evlers      first implementation
 * 2024-06-18       Evlers      configure each name using definition
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <spi_flash.h>
#include <drv_spi.h>
#include <spi_flash_sfud.h>

#ifndef SPI_FLASH_BUS_NAME
#define SPI_FLASH_BUS_NAME "spi1"
#endif

#ifndef SPI_FLASH_CS_PIN_NAME
#define SPI_FLASH_CS_PIN_NAME "PA.0"
#endif

#ifndef SPI_FLASH_DEVICE_NAME
#define SPI_FLASH_DEVICE_NAME SPI_FLASH_BUS_NAME "0"
#endif

#ifndef SPI_FLASH_BLK_DEVICE_NAME
#define SPI_FLASH_BLK_DEVICE_NAME "norflash"
#endif

static int rt_sfud_flash_init (void)
{
    static struct rt_spi_device spi_dev_w25q;

    /* attach a device on SPI bus with CS pin */
    rt_spi_bus_attach_device_cspin(&spi_dev_w25q, SPI_FLASH_DEVICE_NAME, SPI_FLASH_BUS_NAME, rt_pin_get(SPI_FLASH_CS_PIN_NAME), NULL);

    /* initialize SPI Flash device */
    rt_sfud_flash_probe(SPI_FLASH_BLK_DEVICE_NAME, SPI_FLASH_DEVICE_NAME);

    return 0;
}
INIT_DEVICE_EXPORT(rt_sfud_flash_init);
