/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-05-23     supperthomas init
 * 2022-10-07     supperthomas add the support of RT_USING_SERIAL_V2
 * 2024-09-20     not automatically initialized and needs to be invoked manually at the board level
 * 2024-09-20     add support for define behavior if buffer is full
 */

#include "rtdevice.h"
#include "SEGGER_RTT.h"

static struct rt_serial_device _serial_segger_rtt;

typedef struct
{
    struct rt_serial_device *serial;
} drv_segger_rtt_cfg_t;

static drv_segger_rtt_cfg_t m_segger_rtt_cfg =
{
    .serial = &_serial_segger_rtt,
};


static rt_err_t _rtt_cfg(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    return RT_EOK;
}

static rt_err_t _rtt_ctrl(struct rt_serial_device *serial, int cmd, void *arg)
{
    return RT_EOK;
}

static int _rtt_putc(struct rt_serial_device *serial, char c)
{
    SEGGER_RTT_PutChar(RTT_DEFAULT_BUFFER_INDEX, c);
    return RT_EOK;
}

static int _rtt_getc(struct rt_serial_device *serial)
{
    return SEGGER_RTT_GetKey();
}

#ifdef RT_USING_SERIAL_V2
static rt_ssize_t mcu_transmit(struct rt_serial_device     *serial,
                                rt_uint8_t           *buf,
                                rt_size_t             size,
                                rt_uint32_t           tx_flag)
{
    RT_ASSERT(serial != RT_NULL);
    RT_ASSERT(buf != RT_NULL);

    SEGGER_RTT_Write(RTT_DEFAULT_BUFFER_INDEX, buf, size);
    rt_hw_serial_isr(serial, RT_SERIAL_EVENT_TX_DMADONE);

    return size;
}
#endif

static struct rt_uart_ops _segger_rtt_ops =
{
    _rtt_cfg,
    _rtt_ctrl,
    _rtt_putc,
    _rtt_getc,
#ifdef RT_USING_SERIAL_V2
    .transmit = mcu_transmit
#endif
};

static void segger_rtt_check(void)
{
#ifdef RT_USING_SERIAL_V2
    struct rt_serial_rx_fifo *rx_fifo;
    rx_fifo = (struct rt_serial_rx_fifo *)_serial_segger_rtt.serial_rx;
    RT_ASSERT(rx_fifo != RT_NULL);
    while (SEGGER_RTT_HasKey())
    {
        rt_ringbuffer_putchar(&(rx_fifo->rb), SEGGER_RTT_GetKey());
        rt_hw_serial_isr(&_serial_segger_rtt, RT_SERIAL_EVENT_RX_IND);
    }
#else  //RT_USING_SERIAL_V1
    while (SEGGER_RTT_HasKey())
    {
        rt_hw_serial_isr(&_serial_segger_rtt, RT_SERIAL_EVENT_RX_IND);
    }
#endif
}

int rt_hw_segger_rtt_init(void)
{
    SEGGER_RTT_Init();
    rt_thread_idle_sethook(segger_rtt_check);
    SEGGER_RTT_SetTerminal(RTT_DEFAULT_TERMINAL_INDEX);

#if defined(SEGGER_RTT_UP_MODE_NO_BLOCK_SKIP)
    const unsigned int flags = SEGGER_RTT_MODE_NO_BLOCK_SKIP;
#elif defined(SEGGER_RTT_UP_MODE_NO_BLOCK_TRIM)
    const unsigned int flags = SEGGER_RTT_MODE_NO_BLOCK_TRIM;
#elif defined(SEGGER_RTT_UP_MODE_BLOCK_IF_FIFO_FULL)
    const unsigned int flags = SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL;
#endif

    SEGGER_RTT_ConfigUpBuffer(RTT_DEFAULT_BUFFER_INDEX, "rttup", NULL, 0, flags);

    SEGGER_RTT_ConfigDownBuffer(RTT_DEFAULT_BUFFER_INDEX, "rttdown", NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);

    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    _serial_segger_rtt.ops = &_segger_rtt_ops;
    _serial_segger_rtt.config = config;
    m_segger_rtt_cfg.serial = &_serial_segger_rtt;
    rt_hw_serial_register(&_serial_segger_rtt, "segger-rtt", \
                          RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,  &m_segger_rtt_cfg);

    return 0;
}
