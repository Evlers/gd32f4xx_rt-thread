/*
 * Copyright (c) 2006-2023, Evlers Developers
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-02-02   Evlers      first implementation
 * 2024-02-29   Evlers      Add chip uid get api
 */

#include "board.h"

#include "rthw.h"
#include "rtthread.h"

#include "utility.h"


void get_chip_uuid (uint8_t *uid, uint8_t len)
{
    memcpy(uid, (const void *)0x1FFF7A10, MIN(len, 12));
}

#ifdef UTILITY_USING_REBOOT

static void reboot (void)
{
    NVIC_SystemReset();
}
MSH_CMD_EXPORT(reboot, Restart the system);

#endif /* UTILITY_USING_REBOOT */

uint8_t sum (uint8_t *ptr, uint32_t n)
{
    uint8_t check = 0;
    while(n--)
    {
        check += *ptr;
        ptr++;
    }

    return check;
}
