/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-06-25   Evlers      first implementation
 */

#ifndef _DRV_ENET_PHY_H_
#define _DRV_ENET_PHY_H_

#include "rtthread.h"
#include <rtdevice.h>

#if defined(BSP_USING_ENET_PHY_DP83848)
#include "dp83848.h"
#endif

#if defined(BSP_USING_ENET_PHY_DP83867)
#include "dp83867.h"
#endif

#if defined(BSP_USING_ENET_PHY_LAN8720)
#include "lan8720.h"
#endif

#if defined(BSP_USING_ENET_PHY_RTL8201)
#include "rtl8201.h"
#endif

#if defined(BSP_USING_ENET_PHY_RTL8211)
#include "rtl8211.h"
#endif

#ifndef PHY_AUTO_NEGO
#define PHY_AUTO_NEGO  (1U)
#endif

enum phy_link_status
{
   PHY_LINK_DOWN = 0U,
   PHY_LINK_UP
};

typedef struct
{
    rt_uint32_t phy_speed;
    rt_uint32_t phy_duplex;
} phy_info_t;

typedef struct
{
    rt_uint32_t phy_link;
    rt_phy_t phy;
    phy_info_t phy_info;
} phy_device_t;

#endif /* _DRV_ENET_PHY_H_ */




