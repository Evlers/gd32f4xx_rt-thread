/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-06-13   Evlers      first implementation
 */

#include <stdint.h>
#include <stdbool.h>

#include "rtthread.h"

#ifdef RT_USING_PHY
#include <rtdevice.h>

#define DBG_TAG           "enet.phy"
#define DBG_LVL           DBG_INFO
#include <rtdbg.h>

#include "drv_mdio.h"
#include "drv_enet_phy.h"

#include "netif/ethernetif.h"
#include "board.h"

typedef struct
{
    uint32_t periph;
    struct eth_device *eth_dev;
    phy_device_t *phy_dev;
} eth_phy_handle_t;

extern struct eth_device eth_dev;
static phy_device_t phy_dev;
struct rt_phy_ops phy_ops;

static eth_phy_handle_t eth_phy_handle =
{
    .periph  = ENET,
    .eth_dev   = &eth_dev,
    .phy_dev   = &phy_dev,
};

rt_weak void phy_reset (const uint32_t *periph)
{
    rt_base_t reset_pin = rt_pin_get(PHY_RESET_PIN_NAME);

    if (reset_pin < 0) return ;

    rt_pin_mode(reset_pin, PIN_MODE_OUTPUT);

    rt_pin_write(reset_pin, PIN_LOW);
    rt_thread_mdelay(100);
    rt_pin_write(reset_pin, PIN_HIGH);
    rt_thread_mdelay(100);
}

static void config_mode_and_speed (uint32_t mode, uint32_t speed)
{
    uint32_t reg_value = 0U, media_temp = 0U;

    /* read the result of the auto-negotiation */
    media_temp = mode | speed;

    /* after configuring the PHY, use mediamode to configure registers */
    reg_value = ENET_MAC_CFG;
    /* configure ENET_MAC_CFG register */
    reg_value &= (~(ENET_MAC_CFG_SPD | ENET_MAC_CFG_DPM | ENET_MAC_CFG_LBM));
    reg_value |= media_temp;
    ENET_MAC_CFG = reg_value;
}

rt_weak rt_phy_status phy_init (void *object, rt_uint32_t phy_addr, rt_uint32_t src_clock_hz)
{
    rt_mdio_t *mdio_bus = rt_hw_mdio_register(object, "phy_mdio");

    if (RT_NULL == mdio_bus)
    {
        return PHY_STATUS_FAIL;
    }

    eth_phy_handle.phy_dev->phy.bus = mdio_bus;

    /* initialize the mdio */
    if (!mdio_bus->ops->init(mdio_bus, src_clock_hz))
    {
        return PHY_STATUS_FAIL;
    }

#if defined(BSP_USING_ENET_PHY_DP83848)
    dp83848_config_t phy_config;

    dp83848_reset(&eth_phy_handle.phy_dev->phy);
    dp83848_basic_mode_default_config(&eth_phy_handle.phy_dev->phy, &phy_config);
    if (dp83848_basic_mode_init(&eth_phy_handle.phy_dev->phy, &phy_config) == true) {
        return PHY_STATUS_OK;
    } else {
        return PHY_STATUS_FAIL;
    }
#endif

#if defined(BSP_USING_ENET_PHY_DP83867)
    dp83867_config_t phy_config;

    dp83867_reset(&eth_phy_handle.phy_dev->phy);
    #if defined(__DISABLE_AUTO_NEGO) && __DISABLE_AUTO_NEGO
    dp83867_set_mdi_crossover_mode(&eth_phy_handle.phy_dev->phy, enet_phy_mdi_crossover_manual_mdix);
    #endif
    dp83867_basic_mode_default_config(&eth_phy_handle.phy_dev->phy, &phy_config);
    if (dp83867_basic_mode_init(&eth_phy_handle.phy_dev->phy, &phy_config) == true) {
        return PHY_STATUS_OK;
    } else {
        return PHY_STATUS_FAIL;
    }
#endif

#if defined(BSP_USING_ENET_PHY_LAN8720)
    lan8720_config_t phy_config;

    lan8720_reset(&eth_phy_handle.phy_dev->phy);
    lan8720_basic_mode_default_config(&eth_phy_handle.phy_dev->phy, &phy_config);
    if (lan8720_basic_mode_init(&eth_phy_handle.phy_dev->phy, &phy_config) == true) {
        return PHY_STATUS_OK;
    } else {
        return PHY_STATUS_FAIL;
    }
#endif

#if defined(BSP_USING_ENET_PHY_RTL8201)
    rtl8201_config_t phy_config;

    rtl8201_reset(&eth_phy_handle.phy_dev->phy);
    rtl8201_basic_mode_default_config(&eth_phy_handle.phy_dev->phy, &phy_config);
    if (rtl8201_basic_mode_init(&eth_phy_handle.phy_dev->phy, &phy_config) == true) {
        return PHY_STATUS_OK;
    } else {
        return PHY_STATUS_FAIL;
    }
#endif

#if defined(BSP_USING_ENET_PHY_RTL8211)
    rtl8211_config_t phy_config;

    rtl8211_reset(&eth_phy_handle.phy_dev->phy);
    rtl8211_basic_mode_default_config(&eth_phy_handle.phy_dev->phy, &phy_config);
    if (rtl8211_basic_mode_init(&eth_phy_handle.phy_dev->phy, &phy_config) == true) {
        return PHY_STATUS_OK;
    } else {
        return PHY_STATUS_FAIL;
    }
#endif
}

static rt_phy_status phy_get_link_status (rt_phy_t *phy, rt_bool_t *status)
{
    enet_phy_status_t phy_status;

    #if defined(BSP_USING_ENET_PHY_DP83867)
    dp83867_get_phy_status(phy, &phy_status);
    #endif

    #if defined(BSP_USING_ENET_PHY_DP83848)
    dp83848_get_phy_status(phy, &phy_status);
    #endif

    #if defined(BSP_USING_ENET_PHY_RTL8211)
    rtl8211_get_phy_status(phy, &phy_status);
    #endif

    #if defined(BSP_USING_ENET_PHY_RTL8201)
    rtl8201_get_phy_status(phy, &phy_status);
    #endif

    #if defined(BSP_USING_ENET_PHY_LAN8720)
    lan8720_get_phy_status(phy, &phy_status);
    #endif

    *status = phy_status.enet_phy_link;

    return PHY_STATUS_OK;
}

static rt_phy_status phy_get_link_speed_duplex (rt_phy_t *phy, rt_uint32_t *speed, rt_uint32_t *duplex)
{
    enet_phy_status_t phy_status;

    #if defined(BSP_USING_ENET_PHY_DP83867)
    dp83867_get_phy_status(phy, &phy_status);
    #endif

    #if defined(BSP_USING_ENET_PHY_DP83848)
    dp83848_get_phy_status(phy, &phy_status);
    #endif

    #if defined(BSP_USING_ENET_PHY_RTL8211)
    rtl8211_get_phy_status(phy, &phy_status);
    #endif

    #if defined(BSP_USING_ENET_PHY_RTL8201)
    rtl8201_get_phy_status(phy, &phy_status);
    #endif

    #if defined(BSP_USING_ENET_PHY_LAN8720)
    lan8720_get_phy_status(phy, &phy_status);
    #endif

    *speed  = phy_status.enet_phy_speed;
    *duplex = phy_status.enet_phy_duplex;

    return PHY_STATUS_OK;
}

static void phy_poll_status (void *parameter)
{
    int ret;
    phy_info_t phy_info;
    rt_bool_t status;
    phy_device_t *phy_dev;
    struct eth_device* eth_dev;
    char const *ps[] = { "10Mbps", "100Mbps", "1000Mbps" };
    uint32_t enet_speed[] = { ENET_SPEEDMODE_10M, ENET_SPEEDMODE_100M , 0/*ENET_SPEEDMODE_1000M*/ };
    uint32_t enet_duplex[] = { ENET_MODE_HALFDUPLEX, ENET_MODE_FULLDUPLEX };

    eth_phy_handle_t *eth_phy_handle = (eth_phy_handle_t *)parameter;

    eth_dev = eth_phy_handle->eth_dev;
    phy_dev = eth_phy_handle->phy_dev;

    phy_dev->phy.ops->get_link_status(&phy_dev->phy, &status);

    if (status)
    {
        phy_dev->phy.ops->get_link_speed_duplex(&phy_dev->phy, &phy_info.phy_speed, &phy_info.phy_duplex);

        ret = memcmp(&phy_dev->phy_info, &phy_info, sizeof(phy_info_t));
        if (ret != 0)
        {
            memcpy(&phy_dev->phy_info, &phy_info, sizeof(phy_info_t));
        }
    }

    if (phy_dev->phy_link != status)
    {
        phy_dev->phy_link = status ? PHY_LINK_UP : PHY_LINK_DOWN;
        eth_device_linkchange(eth_dev, status);
        LOG_I("PHY Status: %s", status ? "Link up" : "Link down\n");
        if (status == PHY_LINK_UP)
        {
            LOG_I("PHY Speed: %s", ps[phy_dev->phy_info.phy_speed]);
            LOG_I("PHY Duplex: %s\n", phy_dev->phy_info.phy_duplex & PHY_FULL_DUPLEX ? "full duplex" : "half duplex");

            /* configure the rate and mode of the negotiation result */
            config_mode_and_speed(enet_duplex[phy_dev->phy_info.phy_duplex], enet_speed[phy_dev->phy_info.phy_speed]);
        }
    }
}

static void phy_init_thread_entry (void *args)
{
    rt_timer_t phy_status_timer;

    eth_phy_handle_t *eth_phy_handle = (eth_phy_handle_t *)args;

    /* hardware reset phy */
    phy_reset((uint32_t *)&eth_phy_handle->periph);

    LOG_D("initialize the phy\n");
    if (eth_phy_handle->phy_dev->phy.ops->init((void *)eth_phy_handle->periph, 0, rcu_clock_freq_get(CK_AHB)) != PHY_STATUS_OK)
    {
        LOG_E("no any PHY device is detected! Please check your hardware!\n");
    }

    phy_status_timer = rt_timer_create("phy link", phy_poll_status, eth_phy_handle, RT_TICK_PER_SECOND, RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER);

    if (!phy_status_timer || rt_timer_start(phy_status_timer) != RT_EOK)
    {
        LOG_E("failed to start link change detection timer\n");
    }
}

int phy_device_register (void)
{
    eth_phy_handle.periph = ENET;

    /* set phy ops */
    phy_ops.init = phy_init;
    phy_ops.get_link_status = phy_get_link_status;
    phy_ops.get_link_speed_duplex = phy_get_link_speed_duplex;
    eth_phy_handle.phy_dev->phy.ops = &phy_ops;

    /* set PHY address */
    eth_phy_handle.phy_dev->phy.addr = BSP_USING_ENET_PHY_ADDR;

    /* register phy device */
    rt_hw_phy_register(&eth_phy_handle.phy_dev->phy, PHY_NAME);

    /* start PHY monitor */
    rt_thread_startup(rt_thread_create("phy init", phy_init_thread_entry, &eth_phy_handle, 1024, RT_THREAD_PRIORITY_MAX - 2, 2));

    return RT_EOK;
}
INIT_PREV_EXPORT(phy_device_register);

#endif /* RT_USING_PHY */
