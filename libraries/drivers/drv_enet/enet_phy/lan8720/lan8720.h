/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-06-25   Evlers      first implementation
 */

#ifndef _LAN8720_H_
#define _LAN8720_H_

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
#define PHY_NAME     "LAN8720"

#define LAN8720_ID1  (0x0007U)
#define LAN8720_ID2  (0x30U)

/*---------------------------------------------------------------------
 *  Typedef Struct Declarations
 *---------------------------------------------------------------------
 */
typedef struct {
    bool loopback;
    uint8_t speed;
    bool auto_negotiation;
    uint8_t duplex;
} lan8720_config_t;

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */
/*---------------------------------------------------------------------
 * Exported Functions
 *---------------------------------------------------------------------
 */
void lan8720_reset(rt_phy_t *phy);
void lan8720_basic_mode_default_config(rt_phy_t *phy, lan8720_config_t *config);
bool lan8720_basic_mode_init(rt_phy_t *phy, lan8720_config_t *config);
void lan8720_get_phy_status(rt_phy_t *phy, enet_phy_status_t *status);

#if defined(__cplusplus)
}
#endif /* __cplusplus */
#endif /* _LAN8720_H_ */
