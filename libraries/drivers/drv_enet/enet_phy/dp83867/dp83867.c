/*
 * Copyright (c) 2006-2024 RT-Thread Development Team
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

#include "dp83867_regs.h"
#include "dp83867.h"

/*---------------------------------------------------------------------
 * Internal API
 *---------------------------------------------------------------------
 */
bool dp83867_check_id(rt_phy_t *phy)
{
    uint16_t id1, id2;

    phy->bus->ops->read(phy->bus, phy->addr, DP83867_PHYIDR1, &id1, sizeof(id1));
    phy->bus->ops->read(phy->bus, phy->addr, DP83867_PHYIDR2, &id2, sizeof(id2));

    if (DP83867_PHYIDR1_OUI_MSB_GET(id1) == DP83867_ID1 && DP83867_PHYIDR2_OUI_LSB_GET(id2) == DP83867_ID2)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*---------------------------------------------------------------------
 * API
 *---------------------------------------------------------------------
 */
void dp83867_reset(rt_phy_t *phy)
{
    uint16_t data;

    /* PHY reset */
    data = DP83867_BMCR_RESET_SET(1);
    phy->bus->ops->write(phy->bus, phy->addr, DP83867_BMCR, data, sizeof(data));

    /* wait until the reset is completed */
    do {
        phy->bus->ops->read(phy->bus, phy->addr, DP83867_BMCR, &data, sizeof(data));
    } while (DP83867_BMCR_RESET_GET(data));
}

static void dp83867_write_phy_ext(rt_phy_t *phy, uint32_t phy_addr, uint16_t addr, uint16_t data)
{
    uint16_t value;

    /* set the control register for register address */
    value = DP83867_REGCR_FUNCTION_SET(0) | DP83867_REGCR_DEVAD_SET(0x1f);
    phy->bus->ops->write(phy->bus, phy_addr, DP83867_REGCR, value, sizeof(value));

    /* write the specified register address */
    phy->bus->ops->write(phy->bus, phy_addr, DP83867_ADDAR, &addr, sizeof(addr));

    /* set the control register for register data */
    value = DP83867_REGCR_FUNCTION_SET(1) | DP83867_REGCR_DEVAD_SET(0x1f);
    phy->bus->ops->write(phy->bus, phy_addr, DP83867_REGCR, &value, sizeof(value));

    /* write the specified register data */
    phy->bus->ops->write(phy->bus, phy_addr, DP83867_ADDAR, &data, sizeof(data));
}

static uint16_t dp83867_read_phy_ext(rt_phy_t *phy, uint32_t phy_addr, uint16_t addr)
{
    uint16_t value;

    /* set the control register for register address */
    value = DP83867_REGCR_FUNCTION_SET(0) | DP83867_REGCR_DEVAD_SET(0x1f);
    phy->bus->ops->write(phy->bus, phy_addr, DP83867_REGCR, &value, sizeof(value));

    /* write the specified register address */
    phy->bus->ops->write(phy->bus, phy_addr, DP83867_ADDAR, &addr, sizeof(addr));

    /* set the control register for register data */
    value = DP83867_REGCR_FUNCTION_SET(1) | DP83867_REGCR_DEVAD_SET(0x1f);
    phy->bus->ops->write(phy->bus, phy_addr, DP83867_REGCR, &value, sizeof(value));

    /* read the specified register data */
    phy->bus->ops->read(phy->bus, phy_addr, DP83867_ADDAR, &value, sizeof(value));
    return value;
}

void dp83867_basic_mode_default_config(rt_phy_t *phy, dp83867_config_t *config)
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

bool dp83867_basic_mode_init(rt_phy_t *phy, dp83867_config_t *config)
{
    uint16_t data = 0;

    data |= DP83867_BMCR_RESET_SET(0)                        /* Normal operation */
         |  DP83867_BMCR_LOOPBACK_SET(config->loopback)      /* Configure PCS loopback mode */
         |  DP83867_BMCR_ANE_SET(config->auto_negotiation)   /* Configure Auto-Negotiation */
         |  DP83867_BMCR_PWD_SET(0)                          /* Normal operation */
         |  DP83867_BMCR_ISOLATE_SET(0)                      /* Normal operation */
         |  DP83867_BMCR_RESTART_AN_SET(0)                   /* Normal operation (ignored when Auto-Negotiation is disabled) */
         |  DP83867_BMCR_COLLISION_TEST_SET(0);              /* Normal operation */

    if (config->auto_negotiation == false)
    {
        data |= DP83867_BMCR_SPEED0_SET(config->speed) | DP83867_BMCR_SPEED1_SET(config->speed >> 1); /* Set port speed */
        data |= DP83867_BMCR_DUPLEX_SET(config->duplex);                                              /* Set duplex mode */
    }

    /* check the id of dp83867 */
    if (dp83867_check_id(phy) == false)
    {
        return false;
    }

    phy->bus->ops->write(phy->bus, phy->addr, DP83867_BMCR, &data, sizeof(data));
    phy->bus->ops->read(phy->bus, phy->addr, DP83867_BMCR, &data, sizeof(data));

    return true;
}

void dp83867_get_phy_status(rt_phy_t *phy, enet_phy_status_t *status)
{
    uint16_t data;

    phy->bus->ops->read(phy->bus, phy->addr, DP83867_PHYSTS, &data, sizeof(data));
    status->enet_phy_link = DP83867_PHYSTS_LINK_STATUS_GET(data);
    status->enet_phy_speed = DP83867_PHYSTS_SPEED_SELECTION_GET(data) == 0 ? enet_phy_port_speed_10mbps : DP83867_PHYSTS_SPEED_SELECTION_GET(data) == 1 ? enet_phy_port_speed_100mbps : enet_phy_port_speed_1000mbps;
    status->enet_phy_duplex = DP83867_PHYSTS_DUPLEX_MODE_GET(data);
}

void dp83867_set_mdi_crossover_mode(rt_phy_t *phy, enet_phy_crossover_mode_t mode)
{
    uint16_t data;

    data = dp83867_read_phy_ext(phy, phy->addr, DP83867_PHYCR);
    data &= ~DP83867_PHYCR_MDI_CROSSOVER_MASK;
    data |= DP83867_PHYCR_MDI_CROSSOVER_SET(mode);
    dp83867_write_phy_ext(phy, phy->addr, DP83867_PHYCR, data);
}
