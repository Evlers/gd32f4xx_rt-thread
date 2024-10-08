/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author          Notes
 * 2019-08-15     xiangxistu      the first version
 * 2019-09-19     xiaofan         use lwip_netdev_ops instead of ppp_netdev_ops
 * 2024-09-13     Evlers          fix an bug where refresh caused the dns of other network devices to be set
 */

#include "lwip/opt.h"
#include "ppp_netif.h"

#ifdef RT_USING_NETDEV

#include "lwip/ip.h"
#include "lwip/netdb.h"
#include <netdev.h>

extern const struct netdev_ops lwip_netdev_ops;

/**
 * add ppp_netdev into netdev
 *
 * @param ppp_netif      the piont of netif control block
 *
 * @return  RT_EOK       execute successful
 * @return  -ERR_IF      netif error, can't alloc memory for netdev
 */
rt_err_t ppp_netdev_add(struct netif *ppp_netif)
{
#define LWIP_NETIF_NAME_LEN 2
#define PPP_NETIF_ADDR_LEN  6
    int result = 0;
    struct netdev *netdev = RT_NULL;
    char name[LWIP_NETIF_NAME_LEN + 1] = {0};

    RT_ASSERT(ppp_netif);

    netdev = (struct netdev *)rt_calloc(1, sizeof(struct netdev));
    if (netdev == RT_NULL)
    {
        return -ERR_IF;
    }

#ifdef SAL_USING_LWIP
    extern int sal_lwip_netdev_set_pf_info(struct netdev *netdev);

    /* set the lwIP network interface device protocol family information */
    sal_lwip_netdev_set_pf_info(netdev);
#endif /* SAL_USING_LWIP */

    rt_strncpy(name, ppp_netif->name, LWIP_NETIF_NAME_LEN);
    result = netdev_register(netdev, name, (void *)ppp_netif);

    /* OUI 00-04-A3 (hex): Microchip Technology, Inc. */
    netdev->hwaddr[0] = 0x95;
    netdev->hwaddr[1] = 0x45;
    netdev->hwaddr[2] = 0x68;
    /* set MAC address, only for test */
    netdev->hwaddr[3] = 0x39;
    netdev->hwaddr[4] = 0x68;
    netdev->hwaddr[5] = 0x52;

    /* Update netdev info after registered */
    netdev->flags = ppp_netif->flags;
    netdev->mtu = ppp_netif->mtu;
    netdev->ops = &lwip_netdev_ops;
    netdev->hwaddr_len =  PPP_NETIF_ADDR_LEN;
    netdev->ip_addr = ppp_netif->ip_addr;
    netdev->gw = ppp_netif->gw;
    netdev->netmask = ppp_netif->netmask;

    return result;
}

/**
 * refresh ppp_netif state, include DNS and netdev flag information
 *
 * @param ppp_netif      the piont of netif control block
 *
 * @return  RT_EOK       execute successful
 */
rt_err_t ppp_netdev_refresh(struct netif *ppp_netif)
{
    extern const ip_addr_t *dns_getserver(u8_t numdns);
    extern void dns_setserver(u8_t numdns, const ip_addr_t *dnsserver);

    char name[LWIP_NETIF_NAME_LEN + 1];
    struct netdev *netdev = RT_NULL;

    RT_ASSERT(ppp_netif);

    rt_strncpy(name, ppp_netif->name, LWIP_NETIF_NAME_LEN);
    netdev = netdev_get_by_name(name);
    netdev->mtu = ppp_netif->mtu;
#if RT_USING_LWIP_VER_NUM >= 0x20102
    ip_addr_t dns_server[2] = { netdev->dns_servers[0], netdev->dns_servers[1] };
#else
    ip_addr_t dns_server[2] = { *dns_getserver(0), *dns_getserver(1) };
#endif /* RT_USING_LWIP_VER_NUM >= 0x20102 */

    /* sometime we can get second dns server but first dns server is empty, wo need do something to fix it */
    if (!ip_addr_isany_val(dns_server[0]))
    {
        netdev_set_dns_server(netdev, 0, &dns_server[0]);
    }
    else
    {
#define DEF_DNS_SERVER "114.114.114.114"
        ip_addr_t dns_server;
        inet_aton(DEF_DNS_SERVER, &dns_server);
#if RT_USING_LWIP_VER_NUM >= 0x20102
        /* Here we only need to set the dns server of the corresponding network device,
         * but do not need to configure all network devices.
         */
        netdev_set_dns_server(netdev, 0, &dns_server);
#else
        dns_setserver(0, &dns_server);
#endif /* RT_USING_LWIP_VER_NUM >= 0x20102 */
    }

    netdev_set_dns_server(netdev, 1, &dns_server[1]);

    return RT_EOK;
}

/**
 * delete netdev of ppp_netif create
 *
 * @param ppp_netif      the piont of netif control block
 */
void ppp_netdev_del(struct netif *ppp_netif)
{
    char name[LWIP_NETIF_NAME_LEN + 1];
    struct netdev *netdev = RT_NULL;

    RT_ASSERT(ppp_netif);

    rt_strncpy(name, ppp_netif->name, LWIP_NETIF_NAME_LEN);
    netdev = netdev_get_by_name(name);
    if (netdev)
    {
        netdev_unregister(netdev);
        rt_free(netdev);
    }
}

#endif  /* RT_USING_NETDEV */
