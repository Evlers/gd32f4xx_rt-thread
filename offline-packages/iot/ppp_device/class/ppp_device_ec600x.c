/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date             Author          Notes
 * 2024-05-15       Evlers          first implementation
 */

#include <ppp_device.h>
#include <ppp_chat.h>
#include <rtdevice.h>

#define DBG_TAG    "ppp.ec600x"

#ifdef PPP_DEVICE_DEBUG
#define DBG_LVL   DBG_LOG
#else
#define DBG_LVL   DBG_INFO
#endif
#include <rtdbg.h>


#define EC600X_POWER_ON  PIN_HIGH
#define EC600X_POWER_OFF PIN_LOW
#ifndef EC600X_POWER_PIN
#include <drv_gpio.h>
#define EC600X_POWER_PIN -1
#endif

#ifndef PKG_USING_CMUX
static const struct modem_chat_data rst_mcd[] =
{
    {"+++",          MODEM_CHAT_RESP_NOT_NEED,        30, 1, RT_TRUE},
    {"ATH",          MODEM_CHAT_RESP_OK,              30, 1, RT_FALSE},
};

static const struct modem_chat_data wait_startup[] =
{
    {NULL,           MODEM_CHAT_RESP_RDY,             1, 30, RT_FALSE},
    {"AT",           MODEM_CHAT_RESP_OK,              10, 1, RT_FALSE},
    {"ATE0",         MODEM_CHAT_RESP_OK,              1,  1, RT_FALSE},
};

static const struct modem_chat_data wait_net[] =
{
    {"AT+CGREG?",    MODEM_CHAT_RESP_CGREG,           1,  1, RT_FALSE},
};
#endif

static const struct modem_chat_data mcd[] =
{
    {PPP_APN_CMD,    MODEM_CHAT_RESP_OK,              1,  5, RT_FALSE},
    {PPP_DAIL_CMD,   MODEM_CHAT_RESP_CONNECT,         1, 30, RT_FALSE},
};

static rt_err_t ppp_ec600x_prepare(struct ppp_device *device)
{
    rt_err_t err;
#ifndef PKG_USING_CMUX
    if (device->power_pin >= 0)
    {
        rt_pin_write(device->power_pin, EC600X_POWER_OFF);
        rt_thread_mdelay(100);
        rt_pin_write(device->power_pin, EC600X_POWER_ON);
    }
    else
    {
        err = modem_chat(device->uart, rst_mcd, sizeof(rst_mcd) / sizeof(rst_mcd[0]));
        if (err)
            return err;
    }

    /* wait module startup */
    if (modem_chat(device->uart, wait_startup, sizeof(wait_startup) / sizeof(wait_startup[0])))
    {
        LOG_E("Waiting for module startup timed out");
    }

    /* wait network ready */
    uint8_t count = 30;
    while (modem_chat(device->uart, wait_net, sizeof(wait_net) / sizeof(wait_net[0])) != RT_EOK && count --)
    {
        rt_thread_mdelay(1000);
    }

    if (count == 0)
    {
        LOG_E("Waiting for network registration timed out");
    }
#endif

    if ((err = modem_chat(device->uart, mcd, sizeof(mcd) / sizeof(mcd[0]))) != RT_EOK)
    {
        LOG_E("Unable to switch to PPP mode!");
    }

    return err;
}

/* ppp_ec600x_ops for ppp_device_ops , a common interface */
static struct ppp_device_ops ec600x_ops =
{
    .prepare = ppp_ec600x_prepare,
};

/**
 * register ec600x into ppp_device
 *
 * @return  =0:   ppp_device register successfully
 *          <0:   ppp_device register failed
 */
int ppp_ec600x_register(void)
{
    struct ppp_device *ppp_device = RT_NULL;

    ppp_device = rt_malloc(sizeof(struct ppp_device));
    if(ppp_device == RT_NULL)
    {
        LOG_E("No memory for ec600x ppp_device.");
        return -RT_ENOMEM;
    }

    ppp_device->power_pin = EC600X_POWER_PIN;
    if (ppp_device->power_pin >= 0)
    {
        rt_pin_mode(ppp_device->power_pin, PIN_MODE_OUTPUT);
        rt_pin_write(ppp_device->power_pin, EC600X_POWER_OFF);
    }
    ppp_device->ops = &ec600x_ops;

    LOG_D("ppp ec600x is registering ppp_device");

    return ppp_device_register(ppp_device, PPP_DEVICE_NAME, RT_NULL, RT_NULL);
}
INIT_COMPONENT_EXPORT(ppp_ec600x_register);
