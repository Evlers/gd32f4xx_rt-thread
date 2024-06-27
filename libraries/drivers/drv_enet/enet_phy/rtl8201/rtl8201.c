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

#include "rtl8201_regs.h"
#include "rtl8201.h"

/*---------------------------------------------------------------------
 * Internal API
 *---------------------------------------------------------------------
 */
static bool rtl8201_check_id(rt_phy_t *phy)
{
    uint16_t id1, id2;

    phy->bus->ops->read(phy->bus, phy->addr, RTL8201_PHYID1, &id1, sizeof(id1));
    phy->bus->ops->read(phy->bus, phy->addr, RTL8201_PHYID2, &id2, sizeof(id2));

    if (RTL8201_PHYID1_OUI_MSB_GET(id1) == RTL8201_ID1 && RTL8201_PHYID2_OUI_LSB_GET(id2) == RTL8201_ID2) {
        return true;
    } else {
        return false;
    }
}

/*---------------------------------------------------------------------
 * API
 *---------------------------------------------------------------------
 */
void rtl8201_reset(rt_phy_t *phy)
{
    uint16_t data;

    /* PHY reset */
    data = RTL8201_BMCR_RESET_SET(1);
    phy->bus->ops->write(phy->bus, phy->addr, RTL8201_BMCR, &data, sizeof(data));

    /* wait until the reset is completed */
    do {
        phy->bus->ops->read(phy->bus, phy->addr, RTL8201_BMCR, &data, sizeof(data));
    } while (RTL8201_BMCR_RESET_GET(data));
}

void rtl8201_basic_mode_default_config(rt_phy_t *phy, rtl8201_config_t *config)
{
    config->loopback         = false;                        /* Disable PCS loopback mode */
    #if defined(__DISABLE_AUTO_NEGO) && (__DISABLE_AUTO_NEGO)
    config->auto_negotiation = false;                        /* Disable Auto-Negotiation */
    config->speed            = enet_phy_port_speed_100mbps;
    config->duplex           = enet_phy_duplex_full;
    #else
    config->auto_negotiation = true;                         /* Enable Auto-Negotiation */
    #endif
    config->txc_input        = false;                        /* Set TXC as output mode */
}

bool rtl8201_basic_mode_init(rt_phy_t *phy, rtl8201_config_t *config)
{
    uint16_t data = 0;

    data |= RTL8201_BMCR_RESET_SET(0)                        /* Normal operation */
         |  RTL8201_BMCR_LOOPBACK_SET(config->loopback)      /* configure PCS loopback mode */
         |  RTL8201_BMCR_ANE_SET(config->auto_negotiation)   /* configure Auto-Negotiation */
         |  RTL8201_BMCR_PWD_SET(0)                          /* Normal operation */
         |  RTL8201_BMCR_ISOLATE_SET(0)                      /* Normal operation */
         |  RTL8201_BMCR_RESTART_AN_SET(0)                   /* Normal operation (ignored when Auto-Negotiation is disabled) */
         |  RTL8201_BMCR_COLLISION_TEST_SET(0);              /* Normal operation */

    if (config->auto_negotiation == 0) {
        data |= RTL8201_BMCR_SPEED0_SET(config->speed);      /* Set port speed */
        data |= RTL8201_BMCR_DUPLEX_SET(config->duplex);     /* Set duplex mode */
    }

    /* check the id of rtl8201 */
    if (rtl8201_check_id(phy) == false) {
        return false;
    }

    phy->bus->ops->write(phy->bus, phy->addr, RTL8201_BMCR, &data, sizeof(data));

    /* select page 7 */
    data = 7;
    phy->bus->ops->write(phy->bus, phy->addr, RTL8201_PAGESEL, &data, sizeof(data));

    /* set txc direction */
    phy->bus->ops->read(phy->bus, phy->addr, RTL8201_RMSR_P7, &data, sizeof(data));
    data &= ~RTL8201_RMSR_P7_RG_RMII_CLKDIR_MASK;
    data |= RTL8201_RMSR_P7_RG_RMII_CLKDIR_SET(config->txc_input);
    phy->bus->ops->write(phy->bus, phy->addr, RTL8201_RMSR_P7, &data, sizeof(data));

    return true;
}

void rtl8201_get_phy_status(rt_phy_t *phy, enet_phy_status_t *status)
{
    uint16_t data;

    phy->bus->ops->read(phy->bus, phy->addr, RTL8201_BMSR, &data, sizeof(data));
    status->enet_phy_link = RTL8201_BMSR_LINK_STATUS_GET(data);

    phy->bus->ops->read(phy->bus, phy->addr, RTL8201_BMCR, &data, sizeof(data));
    status->enet_phy_speed = RTL8201_BMCR_SPEED0_GET(data) == 0 ? enet_phy_port_speed_10mbps : enet_phy_port_speed_100mbps;
    status->enet_phy_duplex = RTL8201_BMCR_DUPLEX_GET(data);
}
