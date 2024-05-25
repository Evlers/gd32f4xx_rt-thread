/*
 * Copyright (c) 2006-2023, Evlers Developers
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-02-29   Evlers      first implementation
 */

#ifndef _UTILITY_H_
#define _UTILITY_H_

#define BV(x)				(1<<(x))
#define MIN(a, b)           (((a) < (b)) ? (a) : (b))
#define MAX(a, b)           (((a) > (b)) ? (a) : (b))
#define SWAP16(x)			((x>>8) | (x<<8))
#define SWAP32(x)			(((x>>24)&0x000000FF) | ((x>>8)&0x0000FF00) | \
							((x<<8)&0x00FF0000)  | ((x<<24)&0xFF000000))

#define dbg_log_hex(lvl, color_n, fmt, buf, len)            \
    do                                                      \
    {                                                       \
        _DBG_LOG_HDR(lvl, color_n);                         \
        rt_kprintf("%s(%u)", fmt, len);                     \
        if (fmt && len > 16) rt_kprintf("\n");              \
        for (uint32_t i = 0; i < len; i ++) {               \
            if (i != 0 && !(i%16))                          \
                rt_kprintf("\n");                           \
            if ((len > 16) && !(i%8))                       \
                rt_kprintf(" ");                            \
            rt_kprintf(" %02X", ((uint8_t *)buf)[i]);       \
        }                                                   \
        _DBG_LOG_X_END;                                     \
    }                                                       \
    while (0)

#if (DBG_LEVEL >= DBG_LOG)
#define LOG_B(fmt, buf, len) dbg_log_hex("B", 0, fmt, buf, len)
#else
#define LOG_B(...)
#endif


void get_chip_uuid (uint8_t *uid, uint8_t len);
uint8_t sum (uint8_t *ptr, uint32_t n);

#endif /* _UTILITY_H_ */
