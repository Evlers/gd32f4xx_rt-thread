/*
 * Copyright (c) 2019 xiaofan <xfan1024@live.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author          Notes
 * 2019-09-19     xiaofan         the first version
 * 2020-04-13     xiangxistu      transplant into cmux
 */

#ifndef __CMUX_CHAT_H__
#define __CMUX_CHAT_H__

#include <rtthread.h>
#include <rtdevice.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MODEM_CHAT_RESP_LIST(F) \
    F(MODEM_CHAT_RESP_OK,         "OK"), \
    F(MODEM_CHAT_RESP_ERROR,      "ERROR") \

#define DEFINE_MODEM_RESP_ID_TABLE(id, s) id

enum {
    MODEM_CHAT_RESP_LIST(DEFINE_MODEM_RESP_ID_TABLE),
    MODEM_CHAT_RESP_MAX,
    MODEM_CHAT_RESP_NOT_NEED,
};

struct modem_chat_data {
    const char* transmit;
    rt_uint8_t expect;      // use CHAT_RESP_xxx
    rt_uint8_t retries;
    rt_uint8_t timeout;     // second
    rt_bool_t  ignore_cr;   // ignore CR character if it is RT_TRUE
};

rt_err_t cmux_modem_chat(rt_device_t serial, const struct modem_chat_data *data, rt_size_t len);

#ifdef  __cplusplus
    }
#endif

#endif  /* __CMUX_CHAT_H__ */
