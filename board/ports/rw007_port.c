#include <rtthread.h>

#ifdef PKG_USING_RW007
#include <rtdevice.h>
#include <drv_spi.h>
#include <board.h>
#include <spi_wifi_rw007.h>

#define DBG_TAG             "RW007"
#define DBG_LVL             DBG_INFO
#include "rtdbg.h"

#define RW007_SPI_BUS_NAME      "spi2"
#define RW007_CS_PIN            GET_PIN(B, 6)
#define RW007_RST_PIN           GET_PIN(B, 7)
#define RW007_INT_BUSY_PIN      GET_PIN(B, 8)


extern void spi_wifi_isr(int vector);

#ifdef RW007_USING_POWER_MANAGE
#define RW007_PWR_PIN   GET_PIN(B, 9)

static void _wifi_pwr_on(void)
{
    // step 1: resume SPI pin
    gpio_af_set(GPIOB, GPIO_AF_6, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);

    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);

    // step 2: open power
    rt_pin_write(RW007_PWR_PIN, PIN_HIGH);

    // step 3: rest timing
    rt_pin_write(RW007_RST_PIN, PIN_LOW);
    rt_thread_delay(rt_tick_from_millisecond(100));
    rt_pin_write(RW007_RST_PIN, PIN_HIGH);

    /* Wait rw007 ready(exit busy stat) */
    while(!rt_pin_read(RW007_INT_BUSY_PIN))
    {
        rt_thread_delay(5);
    }

    // step 4: wait reset random pin state pass, enable interrupt
    rt_thread_delay(rt_tick_from_millisecond(200));
    rt_pin_mode(RW007_INT_BUSY_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_irq_enable(RW007_INT_BUSY_PIN, PIN_IRQ_ENABLE);
}

static void _wifi_pwr_down(void)
{
    // step 1: disable interrupt, close power
    rt_pin_irq_enable(RW007_INT_BUSY_PIN, PIN_IRQ_DISABLE);
    rt_pin_write(RW007_RST_PIN, PIN_LOW);
    rt_pin_write(RW007_PWR_PIN, PIN_LOW);

    rt_pin_mode(RW007_INT_BUSY_PIN, PIN_MODE_INPUT_PULLDOWN);

    /*
    * step 2: set spi: mosi miso clk csn pins avoid leak
    */
    gpio_af_set(GPIOB, GPIO_AF_0, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6);
    gpio_bit_reset(GPIOB, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6);

}

void wifi_powerswitch(void)
{
    static int onoff = 0;

    LOG_D("wifi %s\n", onoff ? "power on" : "power down");

    if (onoff)
    {
        rw007_powerswitch_request(rw007_power_switch_on);
        // rt_thread_delay(rt_tick_from_millisecond(650));
        rw007_wifi_state_reset();
    }
    else
    {
        rw007_powerswitch_request(rw007_power_switch_off);
    }

    onoff = !onoff;
}
MSH_CMD_EXPORT(wifi_powerswitch, "wifi power switch");
#endif /* RW007_USING_POWER_MANAGE */

static void rw007_gpio_init(void)
{
    /* Configure IO */
    rt_pin_mode(RW007_RST_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(RW007_INT_BUSY_PIN, PIN_MODE_INPUT_PULLDOWN);

    /* Reset rw007 and config mode */
    rt_pin_write(RW007_RST_PIN, PIN_LOW);

#ifdef RW007_USING_POWER_MANAGE
    rt_pin_mode(RW007_PWR_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(RW007_PWR_PIN, PIN_HIGH);
#endif /* RW007_USING_POWER_MANAGE */

    rt_thread_delay(rt_tick_from_millisecond(100));
    rt_pin_write(RW007_RST_PIN, PIN_HIGH);

    /* Wait rw007 ready(exit busy stat) */
    while(!rt_pin_read(RW007_INT_BUSY_PIN))
    {
        rt_thread_delay(5);
    }

    rt_thread_delay(rt_tick_from_millisecond(200));
    rt_pin_mode(RW007_INT_BUSY_PIN, PIN_MODE_INPUT_PULLUP);

#ifdef RW007_USING_POWER_MANAGE
    rw007_register_powerswitch_cb(_wifi_pwr_down, _wifi_pwr_on);
#endif /* RW007_USING_POWER_MANAGE */
    
}

int wifi_spi_device_init(void)
{
    char sn_version[32];

    rw007_gpio_init();

    rt_hw_spi_device_attach(RW007_SPI_BUS_NAME, "wspi", RW007_CS_PIN);

    rt_hw_wifi_init("wspi");

    rt_wlan_set_mode(RT_WLAN_DEVICE_STA_NAME, RT_WLAN_STATION);
    rt_wlan_set_mode(RT_WLAN_DEVICE_AP_NAME, RT_WLAN_AP);

    rw007_sn_get(sn_version);
    LOG_I("\nSN: [%s]\n", sn_version);
    rw007_version_get(sn_version);
    LOG_I("version: [%s]\n\n", sn_version);

    return 0;
}
INIT_APP_EXPORT(wifi_spi_device_init);


static void int_wifi_irq(void * p)
{
    ((void)p);
    spi_wifi_isr(0);
}

void spi_wifi_hw_init(void)
{
    rt_pin_attach_irq(RW007_INT_BUSY_PIN, PIN_IRQ_MODE_FALLING, int_wifi_irq, 0);
    rt_pin_irq_enable(RW007_INT_BUSY_PIN, RT_TRUE);
}

static void wifi_module_ota_enable(int argc, char *args[])
{
    enum rw007_ota_enable_mode enable;
    enum rw007_ota_upgrade_mode upgrade_mode;

    if (argc < 2)
    {
        rt_kprintf("ota_enable 1/0 1/0\n");
        return;
    }

    if (argc >= 2)
    {
        int enable_i = atoi(args[1]);

        enable = ((enable_i != rw007_ota_enable) ? rw007_ota_disable : rw007_ota_enable);
    }
    if (argc >= 3)
    {
        int upgrade_mode_i = atoi(args[2]);

        upgrade_mode = ((upgrade_mode_i != rw007_ota_upgrade_immediate) ? rw007_ota_upgrade_manual : rw007_ota_upgrade_immediate);
    }

    rw007_cfg_ota(enable, upgrade_mode);
}
MSH_CMD_EXPORT_ALIAS(wifi_module_ota_enable, ota_enable, enable WIFI module ota);

#endif /* PKG_USING_RW007 */
