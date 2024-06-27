/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-06-25   Evlers      first implementation
 */

#ifndef _ENET_PHY_H_
#define _ENET_PHY_H_

typedef enum
{
  enet_phy_link_down = 0,
  enet_phy_link_up
} enet_phy_link_status_t;

typedef enum
{
    enet_phy_port_speed_10mbps = 0,
    enet_phy_port_speed_100mbps,
    enet_phy_port_speed_1000mbps
} enet_phy_port_speed_t;

typedef enum
{
    enet_phy_duplex_half = 0,
    enet_phy_duplex_full
} enet_phy_duplex_mode_t;

typedef enum
{
    enet_phy_mdi_crossover_manual_mdi = 0,
    enet_phy_mdi_crossover_manual_mdix,
    enet_phy_mdi_crossover_automatic
} enet_phy_crossover_mode_t;

typedef struct
{
    uint8_t enet_phy_link;
    uint8_t enet_phy_speed;
    uint8_t enet_phy_duplex;
} enet_phy_status_t;

#endif /* _ENET_PHY_H_ */
