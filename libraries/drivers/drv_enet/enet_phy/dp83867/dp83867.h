/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-06-25   Evlers      first implementation
 */

#ifndef _DP83867_H_
#define _DP83867_H_

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
#define PHY_NAME     "DP83867"

#define DP83867_ID1  (0x2000U)
#define DP83867_ID2  (0x28U)

/*---------------------------------------------------------------------
 *  Typedef Struct Declarations
 *---------------------------------------------------------------------
 */
typedef struct {
    bool loopback;
    uint8_t speed;
    bool auto_negotiation;
    uint8_t duplex;
} dp83867_config_t;

typedef enum {
    DP83867_RX_DELAY_0P25_NS = 0,
    DP83867_RX_DELAY_0P50_NS,
    DP83867_RX_DELAY_0P75_NS,
    DP83867_RX_DELAY_1P00_NS,
    DP83867_RX_DELAY_1P25_NS,
    DP83867_RX_DELAY_1P50_NS,
    DP83867_RX_DELAY_1P75_NS,
    DP83867_RX_DELAY_2P00_NS,
    DP83867_RX_DELAY_2P25_NS,
    DP83867_RX_DELAY_2P50_NS,
    DP83867_RX_DELAY_2P75_NS,
    DP83867_RX_DELAY_3P00_NS,
    DP83867_RX_DELAY_3P25_NS,
    DP83867_RX_DELAY_3P50_NS,
    DP83867_RX_DELAY_3P75_NS,
    DP83867_RX_DELAY_4P00_NS
} dp83867_rgmii_rx_delay_t;

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */
/*---------------------------------------------------------------------
 * Exported Functions
 *---------------------------------------------------------------------
 */
void dp83867_reset(rt_phy_t *phy);
void dp83867_basic_mode_default_config(rt_phy_t *phy, dp83867_config_t *config);
bool dp83867_basic_mode_init(rt_phy_t *phy, dp83867_config_t *config);
void dp83867_get_phy_status(rt_phy_t *phy, enet_phy_status_t *status);
void dp83867_set_mdi_crossover_mode(rt_phy_t *phy, enet_phy_crossover_mode_t mode);

#if defined(__cplusplus)
}
#endif /* __cplusplus */
#endif /* _DP83867_H_ */
