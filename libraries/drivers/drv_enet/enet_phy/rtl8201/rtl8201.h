/*
 * Copyright (c) 2006-2024 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-06-25   Evlers      first implementation
 * 2024-07-22   Evlers      add traditional LED Function Selection
 */

#ifndef _RTL8201_H_
#define _RTL8201_H_

/*---------------------------------------------------------------------
 * Includes
 *---------------------------------------------------------------------
 */
#include "rtthread.h"
#include "enet_phy.h"

/*---------------------------------------------------------------------
 *  Macro Const Definitions
 *---------------------------------------------------------------------
 */
#define PHY_NAME     "RTL8201"

#define RTL8201_ID1  (0x001CU)
#define RTL8201_ID2  (0x32U)

/*---------------------------------------------------------------------
 *  Typedef Struct Declarations
 *---------------------------------------------------------------------
 */
typedef struct {
    bool loopback;
    uint8_t speed;
    bool auto_negotiation;
    uint8_t duplex;
    bool txc_input;
    uint8_t led_sel;
} rtl8201_config_t;

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */
/*---------------------------------------------------------------------
 * Exported Functions
 *---------------------------------------------------------------------
 */
void rtl8201_reset(rt_phy_t *phy);
void rtl8201_basic_mode_default_config(rt_phy_t *phy, rtl8201_config_t *config);
bool rtl8201_basic_mode_init(rt_phy_t *phy, rtl8201_config_t *config);
void rtl8201_get_phy_status(rt_phy_t *phy, enet_phy_status_t *status);

#if defined(__cplusplus)
}
#endif /* __cplusplus */
#endif /* _RTL8201_H_ */
