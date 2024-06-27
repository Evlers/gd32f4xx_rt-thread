/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-06-25   Evlers      first implementation
 */

#ifndef _DP83848_H_
#define _DP83848_H_

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
#define PHY_NAME     "DP83848"

#define DP83848_ID1  (0x2000U)
#define DP83848_ID2  (0x17U)

/*---------------------------------------------------------------------
 *  Typedef Struct Declarations
 *---------------------------------------------------------------------
 */
typedef struct {
    bool loopback;
    uint8_t speed;
    bool auto_negotiation;
    uint8_t duplex;
} dp83848_config_t;

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */
/*---------------------------------------------------------------------
 * Exported Functions
 *---------------------------------------------------------------------
 */
void dp83848_reset(rt_phy_t *phy);
void dp83848_basic_mode_default_config(rt_phy_t *phy, dp83848_config_t *config);
bool dp83848_basic_mode_init(rt_phy_t *phy, dp83848_config_t *config);
void dp83848_get_phy_status(rt_phy_t *phy, enet_phy_status_t *status);

#if defined(__cplusplus)
}
#endif /* __cplusplus */
#endif /* _DP83848_H_ */
