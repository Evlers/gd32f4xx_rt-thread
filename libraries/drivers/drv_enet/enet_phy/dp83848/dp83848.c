/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-06-25   Evlers      first implementation
 */

/*---------------------------------------------------------------------
 * Includes
 *---------------------------------------------------------------------
 */
#include <stdint.h>
#include <stdbool.h>

#include "rtthread.h"
#include "rtdevice.h"

#include "dp83848_regs.h"
#include "dp83848.h"

/*---------------------------------------------------------------------
 * Internal API
 *---------------------------------------------------------------------
 */
static bool dp83848_check_id(rt_phy_t *phy)
{
    uint16_t id1, id2;

    phy->bus->ops->read(phy->bus, phy->addr, DP83848_PHYIDR1, &id1, sizeof(id1));
    phy->bus->ops->read(phy->bus, phy->addr, DP83848_PHYIDR2, &id2, sizeof(id2));

    if (DP83848_PHYIDR1_OUI_MSB_GET(id1) == DP83848_ID1 && DP83848_PHYIDR2_OUI_LSB_GET(id2) == DP83848_ID2) {
        return true;
    } else {
        return false;
    }
}

/*---------------------------------------------------------------------
 * API
 *---------------------------------------------------------------------
 */
void dp83848_reset(rt_phy_t *phy)
{
    uint16_t data;

    /* PHY reset */
    data = DP83848_BMCR_RESET_SET(1);
    phy->bus->ops->write(phy->bus, phy->addr, DP83848_BMCR, &data, sizeof(data));

    /* wait until the reset is completed */
    do {
        phy->bus->ops->read(phy->bus, phy->addr, DP83848_BMCR, &data, sizeof(data));
    } while (DP83848_BMCR_RESET_GET(data));
}

void dp83848_basic_mode_default_config(rt_phy_t *phy, dp83848_config_t *config)
{
    config->loopback         = false;                        /* Disable PCS loopback mode */
    #if defined(__DISABLE_AUTO_NEGO) && (__DISABLE_AUTO_NEGO)
    config->auto_negotiation = false;                        /* Disable Auto-Negotiation */
    config->speed            = enet_phy_port_speed_100mbps;
    config->duplex           = enet_phy_duplex_full;
    #else
    config->auto_negotiation = true;                         /* Enable Auto-Negotiation */
    #endif
}

bool dp83848_basic_mode_init(rt_phy_t *phy, dp83848_config_t *config)
{
    uint16_t data = 0;

    data |= DP83848_BMCR_RESET_SET(0)                        /* Normal operation */
         |  DP83848_BMCR_LOOPBACK_SET(config->loopback)      /* configure PCS loopback mode */
         |  DP83848_BMCR_ANE_SET(config->auto_negotiation)   /* configure Auto-Negotiation */
         |  DP83848_BMCR_PWD_SET(0)                          /* Normal operation */
         |  DP83848_BMCR_ISOLATE_SET(0)                      /* Normal operation */
         |  DP83848_BMCR_RESTART_AN_SET(0)                   /* Normal operation (ignored when Auto-Negotiation is disabled) */
         |  DP83848_BMCR_COLLISION_TEST_SET(0);              /* Normal operation */

    if (config->auto_negotiation == false) {
        data |= DP83848_BMCR_SPEED0_SET(config->speed);      /* Set port speed */
        data |= DP83848_BMCR_DUPLEX_SET(config->duplex);     /* Set duplex mode */
    }

    /* check the id of dp83848 */
    if (dp83848_check_id(phy) == false) {
        return false;
    }

    phy->bus->ops->write(phy->bus, phy->addr, DP83848_BMCR, &data, sizeof(data));

    return true;
}

void dp83848_get_phy_status(rt_phy_t *phy, enet_phy_status_t *status)
{
    uint16_t data;

    phy->bus->ops->read(phy->bus, phy->addr, DP83848_PHYSTS, &data, sizeof(data));
    status->enet_phy_link = DP83848_PHYSTS_LINK_STATUS_GET(data);
    status->enet_phy_speed = DP83848_PHYSTS_SPEED_STATUS_GET(data) ? enet_phy_port_speed_10mbps : enet_phy_port_speed_100mbps;
    status->enet_phy_duplex = DP83848_PHYSTS_DUPLEX_STATUS_GET(data);
}
