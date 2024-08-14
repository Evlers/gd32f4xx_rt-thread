/*
 * Copyright (c) 2006-2024 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2022-01-25     iysheng           first version
 */

#include <board.h>
#include <sys/time.h>

#define DBG_TAG             "drv.rtc"
#define DBG_LVL             DBG_INFO

#include <rtdbg.h>

#ifdef RT_USING_RTC

#define RTC_CLOCK_SOURCE_IRC32K
// #define RTC_CLOCK_SOURCE_LXTAL

typedef struct {
    struct rt_device rtc_dev;
} gd32_rtc_device;

static gd32_rtc_device g_gd32_rtc_dev;
static __IO uint32_t prescaler_a = 0, prescaler_s = 0;

static time_t get_rtc_timestamp(void)
{
    time_t rtc_counter;

    struct tm t = { 0 };
    rtc_parameter_struct rtc_initpara;

    rtc_current_time_get(&rtc_initpara);

    t.tm_year = (((rtc_initpara.year & 0xF0) >> 4) * 10) + (rtc_initpara.year & 0x0F) + 100;
    t.tm_mon = rtc_initpara.month;
    t.tm_mday = (((rtc_initpara.date & 0xF0) >> 4) * 10) + (rtc_initpara.date & 0x0F);
    t.tm_hour = rtc_initpara.hour;
    t.tm_min = (((rtc_initpara.minute & 0xF0) >> 4) * 10) + (rtc_initpara.minute &0x0F);
    t.tm_sec = (((rtc_initpara.second & 0xF0) >> 4) * 10) + (rtc_initpara.second & 0x0F);
    t.tm_wday = rtc_initpara.day_of_week - 1;

    rtc_counter = mktime(&t);

    return rtc_counter;
}

static rt_err_t set_rtc_timestamp(time_t time_stamp)
{
    uint32_t rtc_counter;

    rtc_counter = (uint32_t)time_stamp;

    struct tm *t = localtime((const time_t*)&rtc_counter);
    rtc_parameter_struct rtc_initpara;

    rtc_initpara.factor_asyn = prescaler_a;
    rtc_initpara.factor_syn = prescaler_s;
    rtc_initpara.year = ((t->tm_year / 10 % 10) << 4) | (t->tm_year % 10);
    rtc_initpara.month = t->tm_mon;
    rtc_initpara.date = ((t->tm_mday / 10 % 10) << 4) | (t->tm_mday % 10);
    rtc_initpara.hour = t->tm_hour;
    rtc_initpara.minute = ((t->tm_min / 10 % 10) << 4) | (t->tm_min % 10);
    rtc_initpara.second = ((t->tm_sec / 10 % 10) << 4) | (t->tm_sec % 10);
    rtc_initpara.day_of_week = t->tm_wday + 1;
    rtc_initpara.display_format = RTC_24HOUR;
    rtc_initpara.am_pm = RTC_AM;

    rtc_init(&rtc_initpara);

    return RT_EOK;
}

static rt_err_t rt_gd32_rtc_control(rt_device_t dev, int cmd, void *args)
{
    rt_err_t result = RT_EOK;

    RT_ASSERT(dev != RT_NULL);
    switch (cmd)
    {
    case RT_DEVICE_CTRL_RTC_GET_TIME:
        *(rt_uint32_t *)args = get_rtc_timestamp();
        break;

    case RT_DEVICE_CTRL_RTC_SET_TIME:
        if (set_rtc_timestamp(*(rt_uint32_t *)args))
        {
            result = -RT_ERROR;
        }
        break;
    }

    return result;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops g_gd32_rtc_ops =
{
    RT_NULL,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    rt_gd32_rtc_control
};
#endif

/*!
    \brief      RTC configuration function
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void rtc_pre_config (void)
{
    #if defined (RTC_CLOCK_SOURCE_IRC32K)
          rcu_osci_on(RCU_IRC32K);
          rcu_osci_stab_wait(RCU_IRC32K);
          rcu_rtc_clock_config(RCU_RTCSRC_IRC32K);

          prescaler_s = 0x13F;
          prescaler_a = 0x63;
    #elif defined (RTC_CLOCK_SOURCE_LXTAL)
          rcu_osci_on(RCU_LXTAL);
          rcu_osci_stab_wait(RCU_LXTAL);
          rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);

          prescaler_s = 0xFF;
          prescaler_a = 0x7F;
    #else
    #error RTC clock source should be defined.
    #endif /* RTC_CLOCK_SOURCE_IRC32K */

    rcu_periph_clock_enable(RCU_RTC);
    rtc_register_sync_wait();
}

/*!
    \brief      configure the RTC
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void rtc_configuration (void)
{
    rtc_parameter_struct rtc_initpara = { 0 };

    /* setup RTC time value */
    rtc_initpara.factor_asyn = prescaler_a;
    rtc_initpara.factor_syn = prescaler_s;
    rtc_initpara.year = 0x23;
    rtc_initpara.day_of_week = RTC_SATURDAY;
    rtc_initpara.month = RTC_OCT;
    rtc_initpara.date = 0x14;
    rtc_initpara.display_format = RTC_24HOUR;
    rtc_initpara.am_pm = RTC_AM;

    rtc_init(&rtc_initpara);
}

static int rt_hw_rtc_init(void)
{
    rt_err_t ret;

    rcu_periph_clock_enable(RCU_PMU);
    pmu_backup_write_enable();

    rtc_pre_config();

    /* get RTC clock entry selection */
    uint32_t RTCSRC_FLAG = GET_BITS(RCU_BDCTL, 8, 9);

    /* check if RTC has aready been configured */
    if ((0xAA55 != RTC_BKP0) || (0x00 == RTCSRC_FLAG))
    {
        /* backup data register value is not correct or not yet programmed
        or RTC clock source is not configured (when the first time the program
        is executed or data in RCU_BDCTL is lost due to Vbat feeding) */
        rtc_configuration();

        RTC_BKP0 = 0xAA55;
    }

    rcu_all_reset_flag_clear();

    /* RTC timestamp configuration */
    rtc_timestamp_enable(RTC_TIMESTAMP_FALLING_EDGE);
    rtc_interrupt_enable(RTC_INT_TIMESTAMP);
    rtc_flag_clear(RTC_FLAG_TS | RTC_FLAG_TSOVR);

#ifdef RT_USING_DEVICE_OPS
    g_gd32_rtc_dev.rtc_dev.ops         = &g_gd32_rtc_ops;
#else
    g_gd32_rtc_dev.rtc_dev.init        = RT_NULL;
    g_gd32_rtc_dev.rtc_dev.open        = RT_NULL;
    g_gd32_rtc_dev.rtc_dev.close       = RT_NULL;
    g_gd32_rtc_dev.rtc_dev.read        = RT_NULL;
    g_gd32_rtc_dev.rtc_dev.write       = RT_NULL;
    g_gd32_rtc_dev.rtc_dev.control     = rt_gd32_rtc_control;
#endif
    g_gd32_rtc_dev.rtc_dev.type        = RT_Device_Class_RTC;
    g_gd32_rtc_dev.rtc_dev.rx_indicate = RT_NULL;
    g_gd32_rtc_dev.rtc_dev.tx_complete = RT_NULL;
    g_gd32_rtc_dev.rtc_dev.user_data   = RT_NULL;

    ret = rt_device_register(&g_gd32_rtc_dev.rtc_dev, "rtc", \
        RT_DEVICE_FLAG_RDWR);
    if (ret != RT_EOK)
    {
        LOG_E("failed register internal rtc device, err=%d", ret);
    }

    return ret;
}
INIT_DEVICE_EXPORT(rt_hw_rtc_init);
#endif
