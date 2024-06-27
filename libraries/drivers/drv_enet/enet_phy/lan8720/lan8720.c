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

#include "lan8720_regs.h"
#include "lan8720.h"

/*---------------------------------------------------------------------
 * Internal API
 *---------------------------------------------------------------------
 */
static bool lan8720_check_id(rt_phy_t *phy)
{
    uint16_t id1, id2;

    phy->bus->ops->read(phy->bus, phy->addr, LAN8720_PHYID1, &id1, sizeof(id1));
    phy->bus->ops->read(phy->bus, phy->addr, LAN8720_PHYID2, &id2, sizeof(id2));

    if (LAN8720_PHYID1_OUI_MSB_GET(id1) == LAN8720_ID1 && LAN8720_PHYID2_OUI_LSB_GET(id2) == LAN8720_ID2) {
        return true;
    } else {
        return false;
    }
}

/*---------------------------------------------------------------------
 * API
 *---------------------------------------------------------------------
 */
void lan8720_reset(rt_phy_t *phy)
{
    uint16_t data;

    /* PHY reset */
    data = LAN8720_BMCR_RESET_SET(1);
    phy->bus->ops->write(phy->bus, phy->addr, LAN8720_BMCR, &data, sizeof(LAN8720_BMCR_RESET_SET(1)));

    /* wait until the reset is completed */
    do {
        phy->bus->ops->read(phy->bus, phy->addr, LAN8720_BMCR, &data, sizeof(data));
    } while (LAN8720_BMCR_RESET_GET(data));
}

void lan8720_basic_mode_default_config(rt_phy_t *phy, lan8720_config_t *config)
{
    config->loopback         = false;                        /* Disable PCS loopback mode */
    #if defined(__DISABLE_AUTO_NEGO) && __DISABLE_AUTO_NEGO
    config->auto_negotiation = false;                        /* Disable Auto-Negotiation */
    config->speed            = enet_phy_port_speed_100mbps;
    config->duplex           = enet_phy_duplex_full;
    #else
    config->auto_negotiation = true;                         /* Enable Auto-Negotiation */
    #endif
}

bool lan8720_basic_mode_init(rt_phy_t *phy, lan8720_config_t *config)
{
    uint16_t data = 0;

    data |= LAN8720_BMCR_RESET_SET(0)                        /* Normal operation */
         |  LAN8720_BMCR_LOOPBACK_SET(config->loopback)      /* configure PCS loopback mode */
         |  LAN8720_BMCR_ANE_SET(config->auto_negotiation)   /* configure Auto-Negotiation */
         |  LAN8720_BMCR_PWD_SET(0)                          /* Normal operation */
         |  LAN8720_BMCR_ISOLATE_SET(0)                      /* Normal operation */
         |  LAN8720_BMCR_RESTART_AN_SET(0);                  /* Normal operation (ignored when Auto-Negotiation is disabled) */

    if (config->auto_negotiation == 0) {
        data |= LAN8720_BMCR_SPEED_SET(config->speed);      /* Set port speed */
        data |= LAN8720_BMCR_DUPLEX_SET(config->duplex);     /* Set duplex mode */
    }

    /* check the id of lan8720 */
    if (lan8720_check_id(phy) == false) {
        return false;
    }

    phy->bus->ops->write(phy->bus, phy->addr, LAN8720_BMCR, &data, sizeof(data));

    return true;
}

void lan8720_get_phy_status(rt_phy_t *phy, enet_phy_status_t *status)
{
    uint16_t data;

    phy->bus->ops->read(phy->bus, phy->addr, LAN8720_BMSR, &data, sizeof(data));
    status->enet_phy_link = LAN8720_BMSR_LINK_STATUS_GET(data);

    phy->bus->ops->read(phy->bus, phy->addr, LAN8720_PSCSR, &data, sizeof(data));
    status->enet_phy_speed = LAN8720_PSCSR_SPEED_GET(data) == 1 ? enet_phy_port_speed_10mbps : enet_phy_port_speed_100mbps;
    status->enet_phy_duplex = LAN8720_PSCSR_DUPLEX_GET(data);
}
