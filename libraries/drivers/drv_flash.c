/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2023-12-11     Evlers            first version
 * 2024-01-29     Evlers            add interrupt critical section
 */

#include "board.h"

#ifdef BSP_USING_ON_CHIP_FLASH
#include "drv_flash.h"

#if defined(RT_USING_FAL)
#include "fal.h"
#endif

//#define DRV_DEBUG
#define LOG_TAG                "drv.flash"
#include <drv_log.h>


/**
 * Read data from flash.
 * @note This operation's units is word.
 *
 * @param addr flash address
 * @param buf buffer to store read data
 * @param size read bytes size
 *
 * @return result
 */
int gd32_flash_read(rt_uint32_t addr, rt_uint8_t *buf, size_t size)
{
    if ((addr + size) > GD32_FLASH_END_ADDRESS)
    {
        LOG_E("read outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return -RT_EINVAL;
    }
		
    memcpy(buf, ((const void*)addr), size);

	return size;
}

/**
 * Write data to flash.
 * @note This operation's units is word.
 * @note This operation must after erase. @see flash_erase.
 *
 * @param addr flash address
 * @param buf the write data buffer
 * @param size write bytes size
 *
 * @return result
 */
int gd32_flash_write(rt_uint32_t addr, const rt_uint8_t *buf, size_t size)
{
    rt_err_t result = RT_EOK;
    fmc_state_enum fmc_state = FMC_READY;

	if ((addr + size) > GD32_FLASH_END_ADDRESS)
    {
        LOG_E("write outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return -RT_EINVAL;
    }

    if (size < 1)
    {
        return -RT_EINVAL;
    }

	/* unlock the flash program erase controller */
	fmc_unlock();

	for (uint32_t i = 0; i < size; i ++)
    {
        rt_base_t level = rt_hw_interrupt_disable();

		/* clear pending flags */
		fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);

		/* write byte to the corresponding address */
		fmc_state = fmc_byte_program(addr, buf[i]);
        rt_hw_interrupt_enable(level);
		
		if (fmc_state != FMC_READY)
        {
            LOG_E("Write error of the flash, addr: 0x%08X, size: 0x%X, offset: %u, state: %u", addr, size, i, fmc_state);
            result = -RT_ERROR;
            break;
        }

		addr += sizeof(buf[0]);
	}

	/* lock the flash program erase controller */
	fmc_lock();

    if (result != RT_EOK)
    {
        return result;
    }

    return size;
}

/**
 * Erase data on flash.
 * @note This operation is irreversible.
 * @note This operation's units is different which on many chips.
 *
 * @param addr flash address
 * @param size erase bytes size
 *
 * @return result
 */
int gd32_flash_erase(rt_uint32_t addr, size_t size)
{
    rt_err_t result = RT_EOK;
    fmc_state_enum fmc_state = FMC_READY;

    if ((addr + size) > GD32_FLASH_END_ADDRESS)
    {
        LOG_E("ERROR: erase outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return -RT_EINVAL;
    }

    /* unlock the flash program erase controller */
	fmc_unlock();

    for (uint32_t i = 0; i < size; i += GD32_FLASH_PAGE_SIZE)
    {
        rt_base_t level = rt_hw_interrupt_disable();

        /* clear pending flags */
	    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);

        /* wait the erase operation complete */
        fmc_state = fmc_page_erase(addr + i);
        rt_hw_interrupt_enable(level);

	    if (fmc_state != FMC_READY)
        {
            LOG_E("Erase error of the flash, addr: 0x%08X, size: 0x%X, offset: %u, state: %u", addr, size, i, fmc_state);
            result = -RT_ERROR;
            break;
        }
    }
	
	/* lock the flash program erase controller */
	fmc_lock();

    if (result != RT_EOK)
    {
        return result;
    }

    return size;
}

#if defined(RT_USING_FAL)

static int fal_flash_read(long offset, rt_uint8_t *buf, size_t size);
static int fal_flash_write(long offset, const rt_uint8_t *buf, size_t size);
static int fal_flash_erase(long offset, size_t size);

const struct fal_flash_dev gd32_onchip_flash = 
{
    "onchip_flash",
    GD32_ONCHIP_FLASH_ADDR,
    GD32_ONCHIP_FLASH_SIZE,
    GD32_FLASH_PAGE_SIZE,
    {
        NULL,
        fal_flash_read,
        fal_flash_write,
        fal_flash_erase
    }
};

static int fal_flash_read(long offset, rt_uint8_t *buf, size_t size)
{
    return gd32_flash_read(gd32_onchip_flash.addr + offset, buf, size);
}

static int fal_flash_write(long offset, const rt_uint8_t *buf, size_t size)
{
    return gd32_flash_write(gd32_onchip_flash.addr + offset, buf, size);
}

static int fal_flash_erase(long offset, size_t size)
{
    return gd32_flash_erase(gd32_onchip_flash.addr + offset, size);
}


#endif /* RT_USING_FAL */

#endif /* BSP_USING_ON_CHIP_FLASH */


