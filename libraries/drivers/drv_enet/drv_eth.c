/*
 * Copyright (c) 2006-2024 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-06-13   Evlers      first implementation
 * 2024-08-27   Evlers      close flow control and osf function to fix dma tx stop bug
 */

#include <stdint.h>
#include <stdbool.h>

#include "board.h"

#include "drv_config.h"
#include "drv_eth.h"

#include <netif/ethernetif.h>
#include <lwipopts.h>

/* debug option */
// #define ETH_RX_DUMP
// #define ETH_TX_DUMP

#define DBG_COLOR
#define DBG_TAG             "drv.enet"
#define DBG_LVL             DBG_INFO
#include <rtdbg.h>

/* unique device ID register base address of the gd32f4xxx */
#define UID_BASE                    (0x1FFF7A10U)

struct frame
{
    rt_uint32_t length;
    rt_uint32_t buffer;
    enet_descriptors_struct *rx_fs_desc;
    enet_descriptors_struct *rx_ls_desc;
    rt_uint8_t seg_count;
};

/* ENET RxDMA/TxDMA descriptor */
extern enet_descriptors_struct rxdesc_tab[ENET_RXBUF_NUM], txdesc_tab[ENET_TXBUF_NUM];

/* global transmit and receive descriptors pointers */
extern enet_descriptors_struct *dma_current_txdesc;
extern enet_descriptors_struct *dma_current_rxdesc;

/* ethernet device */
struct eth_device eth_dev;
/* record of the rx descriptor */
static struct frame rx_frame;
/* interface address info, hw address */
static rt_uint8_t mac_addr[NETIF_MAX_HWADDR_LEN];

#ifdef RT_LWIP_USING_HW_CHECKSUM
static rt_uint32_t rx_err_cnt;
#endif


#if defined(ETH_RX_DUMP) || defined(ETH_TX_DUMP)
#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')
static void dump_hex (const rt_uint8_t *ptr, rt_size_t buflen)
{
    unsigned char *buf = (unsigned char *)ptr;
    int i, j;
    static rt_mutex_t mutex = NULL;

    if (mutex == NULL)
    {
        mutex = rt_mutex_create("eth_dump_hex", RT_NULL);
        RT_ASSERT(mutex != NULL);
    }

    rt_mutex_take(mutex, RT_WAITING_FOREVER);

    for (i = 0; i < buflen; i += 16)
    {
        rt_kprintf("%08X: ", i);

        for (j = 0; j < 16; j++)
            if (i + j < buflen)
                rt_kprintf("%02X ", buf[i + j]);
            else
                rt_kprintf("   ");
        rt_kprintf(" ");

        for (j = 0; j < 16; j++)
            if (i + j < buflen)
                rt_kprintf("%c", __is_print(buf[i + j]) ? buf[i + j] : '.');
        rt_kprintf("\n");
    }

    rt_mutex_release(mutex);
}
#endif

/**
 * @brief ETH MSP Initialization
 *        This function configures the hardware resources used in this example:
 *           - Peripheral's GPIO Configuration
 *           - NVIC configuration for interrupt priority
 *           - Ethernet PHY interface selection
 *        This function belongs to weak function, users can rewrite this function according to different needs
 *
 * @return None
 */
rt_weak void gd32_msp_eth_init (void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);

    /* enable SYSCFG clock */
    rcu_periph_clock_enable(RCU_SYSCFG);

    /* RMII is selected for the Ethernet MAC */
    syscfg_enet_phy_interface_config(SYSCFG_ENET_PHY_RMII);

    /* PA1: ETH_RMII_REF_CLK */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_1);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_1);

    /* PA2: ETH_MDIO */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_2);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_2);

    /* PA7: ETH_RMII_CRS_DV */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_7);

    gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_1);
    gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_2);
    gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_7);

    /* PB11: ETH_RMII_TX_EN */
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_11);

    /* PB12: ETH_RMII_TXD0 */
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_12);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_12);

    /* PB13: ETH_RMII_TXD1 */
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_13);

    gpio_af_set(GPIOB, GPIO_AF_11, GPIO_PIN_11);
    gpio_af_set(GPIOB, GPIO_AF_11, GPIO_PIN_12);
    gpio_af_set(GPIOB, GPIO_AF_11, GPIO_PIN_13);

    /* PC1: ETH_MDC */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_1);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_1);

    /* PC4: ETH_RMII_RXD0 */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_4);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_4);

    /* PC5: ETH_RMII_RXD1 */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_5);

    gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_1);
    gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_4);
    gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_5);

    NVIC_SetPriority(ENET_IRQn, 2);
}

static void enet_default_init (void)
{
    uint32_t reg_value = 0U;

    /* MAC */
    /* configure ENET_MAC_CFG register */
    reg_value = ENET_MAC_CFG;
    reg_value &= MAC_CFG_MASK;
    reg_value |= ENET_WATCHDOG_ENABLE | ENET_JABBER_ENABLE | ENET_INTERFRAMEGAP_96BIT \
                 | ENET_SPEEDMODE_10M | ENET_MODE_HALFDUPLEX | ENET_LOOPBACKMODE_DISABLE \
                 | ENET_CARRIERSENSE_ENABLE | ENET_RECEIVEOWN_ENABLE \
                 | ENET_RETRYTRANSMISSION_ENABLE | ENET_BACKOFFLIMIT_10 \
                 | ENET_DEFERRALCHECK_DISABLE \
                 | ENET_TYPEFRAME_CRC_DROP_DISABLE \
                 | ENET_AUTO_PADCRC_DROP_DISABLE \
                 | ENET_CHECKSUMOFFLOAD_DISABLE;
    ENET_MAC_CFG = reg_value;

    /* configure ENET_MAC_FRMF register */
    ENET_MAC_FRMF = ENET_SRC_FILTER_DISABLE | ENET_DEST_FILTER_INVERSE_DISABLE \
                    | ENET_MULTICAST_FILTER_PERFECT | ENET_UNICAST_FILTER_PERFECT \
                    | ENET_PCFRM_PREVENT_ALL | ENET_BROADCASTFRAMES_ENABLE \
                    | ENET_PROMISCUOUS_DISABLE | ENET_RX_FILTER_ENABLE;

    /* configure ENET_MAC_HLH, ENET_MAC_HLL register */
    ENET_MAC_HLH = 0x0U;

    ENET_MAC_HLL = 0x0U;

    /* configure ENET_MAC_FCTL, ENET_MAC_FCTH register */
    reg_value = ENET_MAC_FCTL;
    reg_value &= MAC_FCTL_MASK;
    reg_value |= MAC_FCTL_PTM(0) | ENET_ZERO_QUANTA_PAUSE_DISABLE \
                 | ENET_PAUSETIME_MINUS4 | ENET_UNIQUE_PAUSEDETECT \
                 | ENET_RX_FLOWCONTROL_DISABLE | ENET_TX_FLOWCONTROL_DISABLE;
    ENET_MAC_FCTL = reg_value;

    /* configure ENET_MAC_VLT register */
    ENET_MAC_VLT = ENET_VLANTAGCOMPARISON_16BIT | MAC_VLT_VLTI(0);

    /* DMA */
    /* configure ENET_DMA_CTL register */
    reg_value = ENET_DMA_CTL;
    reg_value &= DMA_CTL_MASK;
    reg_value |= ENET_TCPIP_CKSUMERROR_DROP | ENET_RX_MODE_STOREFORWARD \
                 | ENET_FLUSH_RXFRAME_ENABLE | ENET_TX_MODE_STOREFORWARD \
                 | ENET_TX_THRESHOLD_64BYTES | ENET_RX_THRESHOLD_64BYTES \
                 | ENET_SECONDFRAME_OPT_DISABLE;
    ENET_DMA_CTL = reg_value;

    /* configure ENET_DMA_BCTL register */
    reg_value = ENET_DMA_BCTL;
    reg_value &= DMA_BCTL_MASK;
    reg_value = ENET_ADDRESS_ALIGN_ENABLE | ENET_ARBITRATION_RXTX_2_1 \
                | ENET_RXDP_32BEAT | ENET_PGBL_32BEAT | ENET_RXTX_DIFFERENT_PGBL \
                | ENET_FIXED_BURST_ENABLE | ENET_MIXED_BURST_DISABLE \
                | ENET_NORMAL_DESCRIPTOR;
    ENET_DMA_BCTL = reg_value;
}

static void config_store_forward_mode (bool rsf, bool tsf, uint8_t rx_threshold, uint8_t tx_threshold)
{
    uint32_t reg_value = 0U;

    /* configure store_forward_mode related registers */
    reg_value = ENET_DMA_CTL;
    /* configure ENET_DMA_CTL register */
    reg_value &= ~(ENET_DMA_CTL_RSFD | ENET_DMA_CTL_TSFD | ENET_DMA_CTL_RTHC | ENET_DMA_CTL_TTHC);
    reg_value |= rsf ? ENET_DMA_CTL_RSFD : 0;
    reg_value |= tsf ? ENET_DMA_CTL_TSFD : 0;
    reg_value |= rx_threshold | tx_threshold;
    ENET_DMA_CTL = reg_value;
}

static void config_checksum (enet_chksumconf_enum checksum, enet_frmrecept_enum recept)
{
    uint32_t reg_value = 0U;

    /* configure checksum */
    if (RESET != ((uint32_t)checksum & ENET_CHECKSUMOFFLOAD_ENABLE))
    {
        ENET_MAC_CFG |= ENET_CHECKSUMOFFLOAD_ENABLE;

        reg_value = ENET_DMA_CTL;
        /* configure ENET_DMA_CTL register */
        reg_value &= ~ENET_DMA_CTL_DTCERFD;
        reg_value |= ((uint32_t)checksum & ENET_DMA_CTL_DTCERFD);
        ENET_DMA_CTL = reg_value;
    }

    /* configure recept */
    ENET_MAC_FRMF |= (uint32_t)recept;
}


/* ENET initialization function */
static rt_err_t rt_gd32_eth_init (rt_device_t dev)
{
    gd32_msp_eth_init();

    /* configure NVIC */
    NVIC_EnableIRQ(ENET_IRQn);

    /* enable ethernet clock  */
    rcu_periph_clock_enable(RCU_ENET);
    rcu_periph_clock_enable(RCU_ENETTX);
    rcu_periph_clock_enable(RCU_ENETRX);

    /* reset ethernet on AHB bus */
    enet_deinit();
    enet_software_reset();

    /* initialize ENET peripheral with generally concerned parameters */
    enet_default_init();

    /* configure checksum */
#ifdef RT_LWIP_USING_HW_CHECKSUM
    /* enabled the hardware check function, but the data is not discarded and only set the status error flag */
    config_checksum(ENET_AUTOCHECKSUM_ACCEPT_FAILFRAMES, ENET_BROADCAST_FRAMES_PASS);
#else
    config_checksum(ENET_NO_AUTOCHECKSUM, ENET_BROADCAST_FRAMES_PASS);
#endif

    /* enable the automatic zero-quanta generation function */
    enet_flowcontrol_feature_enable(ENET_ZERO_QUANTA_PAUSE);

    /* initialize MAC address in ethernet MAC */
    enet_mac_address_set(ENET_MAC_ADDRESS0, mac_addr);

    /* initialize descriptors list: chain/ring mode */
    enet_descriptors_chain_init(ENET_DMA_TX);
    enet_descriptors_chain_init(ENET_DMA_RX);

    /* enable ethernet Rx interrrupt */
    for (int i = 0; i < ENET_RXBUF_NUM; i ++)
    {
        enet_rx_desc_immediate_receive_complete_interrupt(&rxdesc_tab[i]);
    }

#ifdef RT_LWIP_USING_HW_CHECKSUM
    /* enable the TCP, UDP and ICMP checksum insertion for the Tx frames */
    for (int i = 0; i < ENET_TXBUF_NUM; i ++)
    {
        enet_transmit_checksum_config(&txdesc_tab[i], ENET_CHECKSUM_TCPUDPICMP_FULL);
    }
#endif

    config_store_forward_mode(true, true, ENET_RX_THRESHOLD_64BYTES, ENET_TX_THRESHOLD_64BYTES);

    /* enabled ENET interrupt */
    enet_interrupt_enable(ENET_DMA_INT_NIE);
    enet_interrupt_enable(ENET_DMA_INT_RIE);

    /* enable MAC and DMA transmission and reception */
    enet_enable();

    return RT_EOK;
}

static rt_err_t rt_gd32_eth_open (rt_device_t dev, rt_uint16_t oflag)
{
    LOG_D("eth open");
    return RT_EOK;
}

static rt_err_t rt_gd32_eth_close (rt_device_t dev)
{
    LOG_D("eth close");
    return RT_EOK;
}

static rt_ssize_t rt_gd32_eth_read (rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    LOG_D("eth read");
    rt_set_errno(-RT_ENOSYS);
    return 0;
}

static rt_ssize_t rt_gd32_eth_write (rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    LOG_D("eth write");
    rt_set_errno(-RT_ENOSYS);
    return 0;
}

static rt_err_t rt_gd32_eth_control (rt_device_t dev, int cmd, void *args)
{
    switch (cmd)
    {
    case NIOCTL_GADDR:
        /* get mac address */
        if (args)
        {
            SMEMCPY(args, mac_addr, 6);
        }
        else
        {
            return -RT_ERROR;
        }
        break;

    default :
        break;
    }

    return RT_EOK;
}

static void resume_dma_transfer (void)
{
    uint32_t dma_tbu_flag, dma_tu_flag;

    /* check Tx buffer unavailable flag status */
    dma_tbu_flag = (ENET_DMA_STAT & ENET_DMA_STAT_TBU);
    dma_tu_flag = (ENET_DMA_STAT & ENET_DMA_STAT_TU);

    if ((RESET != dma_tbu_flag) || (RESET != dma_tu_flag))
    {
        /* clear TBU and TU flag */
        ENET_DMA_STAT = (dma_tbu_flag | dma_tu_flag);
        /* resume DMA transmission by writing to the TPEN register */
        ENET_DMA_TPEN = 0U;
    }
}

static rt_err_t wait_dma_transfer_complete (enet_descriptors_struct *dma_tx_desc)
{
    rt_tick_t start = rt_tick_get();

    while (dma_tx_desc->status & ENET_TDES0_DAV)
    {
        resume_dma_transfer();
        rt_thread_yield();
        if ((rt_tick_get() - start) > rt_tick_from_millisecond(100))
        {
            /* restart DMA and MAC transmission */
            enet_disable();
            enet_enable();
        }
    }

    return RT_EOK;
}

/* txpkt chainmode */
static rt_err_t txpkt_chainmode (rt_uint32_t frame_length)
{
    rt_uint32_t buf_cnt = 0, index = 0;

    /* check if the descriptor is owned by the ethernet dma (when set) or cpu (when reset) */
    if ((dma_current_txdesc->status & ENET_TDES0_DAV) != RESET)
    {
        /* return error: dav bit set */
        return RT_ERROR;
    }

    if (frame_length == 0)
    {
        return RT_ERROR;
    }

    if (frame_length > ENET_MAX_FRAME_SIZE)
    {
        buf_cnt = frame_length / ENET_MAX_FRAME_SIZE;
        if(frame_length % ENET_MAX_FRAME_SIZE)
        {
          buf_cnt += 1;
        }
    }
    else
    {
        buf_cnt = 1;
    }

    if (buf_cnt == 1)
    {
        /* setting the last segment and first segment bits (in this case a frame is transmitted in one descriptor) */
        dma_current_txdesc->status |= ENET_TDES0_LSG | ENET_TDES0_FSG;

        /* setting the frame length: bits[12:0] */
        dma_current_txdesc->control_buffer_size = (frame_length & 0x00001FFF);

        /* set dav bit of the tx descriptor status: gives the buffer back to ethernet dma */
        dma_current_txdesc->status |= ENET_TDES0_DAV;

        /* selects the next dma tx descriptor list for next buffer to send */
        dma_current_txdesc = (enet_descriptors_struct*) (dma_current_txdesc->buffer2_next_desc_addr);
    }
    else
    {
        for (index = 0; index < buf_cnt; index ++)
        {
            /* clear first and last segments */
            dma_current_txdesc->status &= ~(ENET_TDES0_LSG | ENET_TDES0_FSG);

            /* set first segments */
            if (index == 0)
            {
                dma_current_txdesc->status |= ENET_TDES0_FSG;
            }

            /* set size */
            dma_current_txdesc->control_buffer_size = (ENET_MAX_FRAME_SIZE & 0x00001FFF);

            /* set last segments */
            if (index == (buf_cnt - 1))
            {
                dma_current_txdesc->status |= ENET_TDES0_LSG;
                dma_current_txdesc->control_buffer_size = ((frame_length - ((buf_cnt - 1) * ENET_MAX_FRAME_SIZE)) & 0x00001FFF);
            }

            /* set dav bit of the tx descriptor status: gives the buffer back to ethernet dma */
            dma_current_txdesc->status |= ENET_TDES0_DAV;

            /* selects the next dma tx descriptor list for next buffer to send */
            dma_current_txdesc = (enet_descriptors_struct*) (dma_current_txdesc->buffer2_next_desc_addr);
        }
    }

    resume_dma_transfer();

    return RT_EOK;
}

/* transmit data */
rt_err_t rt_gd32_eth_tx (rt_device_t dev, struct pbuf *p)
{
    rt_err_t ret = -RT_ERROR;
    struct pbuf *q;
    uint8_t *buffer = NULL;
    uint32_t payload_offset, copy_count, buffer_offset = 0, frame_length = 0;
    enet_descriptors_struct *dma_tx_desc = dma_current_txdesc;

    buffer = (uint8_t *)(dma_tx_desc->buffer1_addr);

    for (q = p; q != NULL; q = q->next)
    {
        copy_count = q->len;
        payload_offset = 0;

        /* if the buffer is not enough to store the following data, split the data frame into each buffer */
        while ((copy_count + buffer_offset) > ENET_MAX_FRAME_SIZE)
        {
            /* copy only data frames to buffer */
            memcpy(buffer + buffer_offset, (uint8_t *)q->payload + payload_offset, (ENET_MAX_FRAME_SIZE - buffer_offset));

            /* gets the descriptor of the next new buffer */
            dma_tx_desc = (enet_descriptors_struct *)(dma_tx_desc->buffer2_next_desc_addr);

            /* check DMA own status within timeout */
            if (wait_dma_transfer_complete(dma_tx_desc) != RT_EOK)
            {
                LOG_W("Wait for dma transfer complete timeout");
                goto __error;
            }

            /* gets the buffer first address of this descriptor */
            buffer = (uint8_t *)(dma_tx_desc->buffer1_addr);

            /* calculate the parameters needed for the next copy of the data frame */
            copy_count -= ENET_MAX_FRAME_SIZE - buffer_offset;      /* calculate the amount of data remaining to be copied */
            payload_offset += ENET_MAX_FRAME_SIZE - buffer_offset;  /* calculate the offset address of the data to be copied next */
            frame_length += ENET_MAX_FRAME_SIZE - buffer_offset;    /* calculates the length of the data frame that has been sent */
            buffer_offset = 0;                                      /* the new buffer copies the remaining data frames from the first address */
        }

        /* check DMA own status within timeout */
        if (wait_dma_transfer_complete(dma_tx_desc) != RT_EOK)
        {
            LOG_W("Wait for dma transfer to complete timeout");
            goto __error;
        }

        /* continue to copy the remaining data */
        memcpy(buffer + buffer_offset, (uint8_t *)q->payload + payload_offset, copy_count);
        buffer_offset = buffer_offset + copy_count;
        frame_length += copy_count;
    }

    __error:
    if (frame_length != p->tot_len)
    {
        LOG_W("sent frame length: %u, total length: %u", frame_length, p->tot_len);
    }

    /* transmit descriptors to give to DMA */
    ret = txpkt_chainmode(frame_length);

#ifdef ETH_TX_DUMP
    if (buffer != NULL)
    {
        dump_hex(buffer, frame_length);
    }
#endif

    LOG_D("transmit frame length :%d", framelength);

    resume_dma_transfer();

    return ret;
}

/* rxpkt chainmode */
static rt_err_t rxpkt_chainmode (void)
{
#ifdef RT_LWIP_USING_HW_CHECKSUM
    /* using ENET_AUTOCHECKSUM_ACCEPT_FAILFRAMES checksum config */
    if ((dma_current_rxdesc->status & ENET_RDES0_ERRS) != RESET)
    {
        rx_err_cnt ++;

        /* do not drop data, hand it over to lwip for verification */
        // LOG_W("receive error data frame");
        // return RT_ERROR;
    }
#endif

    /* check if the descriptor is owned by the ethernet dma (when set) or cpu (when reset) */
    if ((dma_current_rxdesc->status & ENET_RDES0_DAV) != RESET)
    {
        /* return error: dav bit set */
        return RT_ERROR;
    }

    /* last descriptor */
    if ((dma_current_rxdesc->status & ENET_RDES0_LDES) != RESET)
    {
        rx_frame.seg_count ++;
        if (rx_frame.seg_count == 1)
        {
            rx_frame.rx_fs_desc = dma_current_rxdesc;
        }
        rx_frame.rx_ls_desc = dma_current_rxdesc;
        rx_frame.length = enet_desc_information_get(dma_current_rxdesc, RXDESC_FRAME_LENGTH);;
        rx_frame.buffer = rx_frame.rx_fs_desc->buffer1_addr;

        /* Selects the next DMA Rx descriptor list for next buffer to read */
        dma_current_rxdesc = (enet_descriptors_struct *) (dma_current_rxdesc->buffer2_next_desc_addr);

        return RT_EOK;
    }

    /* first descriptor */
    else if ((dma_current_rxdesc->status & ENET_RDES0_FDES) != RESET)
    {
        rx_frame.seg_count = 1;
        rx_frame.rx_fs_desc = dma_current_rxdesc;
        rx_frame.rx_ls_desc = NULL;
        dma_current_rxdesc = (enet_descriptors_struct *) (dma_current_rxdesc->buffer2_next_desc_addr);
    }

    /* continue receiving data frames */
    else
    {
        if (rx_frame.seg_count >= ENET_RXBUF_NUM)
        {
            /* the buffer has been used up, can not continue to store, directly push the data to lwip */
            LOG_W("the data frame is too long to continue to be stored in the buffer");
            return RT_EOK;
        }
        rx_frame.seg_count ++;
        dma_current_rxdesc = (enet_descriptors_struct *) (dma_current_rxdesc->buffer2_next_desc_addr);
    }

    return RT_ERROR;
}

/* receive data*/
struct pbuf *rt_gd32_eth_rx (rt_device_t dev)
{
    struct pbuf *p = NULL, *q;
    uint32_t len;
    uint8_t *buffer;
    rt_uint32_t buffer_offset, payload_offset = 0, copy_count = 0;
    enet_descriptors_struct *dma_rx_desc;

    if (rxpkt_chainmode() != RT_EOK)
    {
        return NULL;
    }

    /* obtain the size of the packet and put it into the "len" variable. */
    len = rx_frame.length;
    buffer = (uint8_t *)rx_frame.buffer;

    LOG_D("receive frame len : %d", len);

    if (len > 0)
    {
        /* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
        p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);

        if (p != NULL)
        {
            dma_rx_desc = rx_frame.rx_fs_desc;
            buffer_offset = 0;

            for (q = p; q != NULL; q = q->next)
            {
                copy_count = q->len;
                payload_offset = 0;

                while ((copy_count + buffer_offset) > ENET_MAX_FRAME_SIZE)
                {
                    /* copy data to pbuf */
                    memcpy((uint8_t *)q->payload + payload_offset, buffer + buffer_offset, (ENET_MAX_FRAME_SIZE - buffer_offset));

                    /* point to next descriptor */
                    dma_rx_desc = (enet_descriptors_struct *)(dma_rx_desc->buffer2_next_desc_addr);
                    buffer = (uint8_t *)(dma_rx_desc->buffer1_addr);

                    copy_count = copy_count - (ENET_MAX_FRAME_SIZE - buffer_offset);
                    payload_offset = payload_offset + (ENET_MAX_FRAME_SIZE - buffer_offset);
                    buffer_offset = 0;
                }

                /* copy remaining data in buffer */
                memcpy((uint8_t *)q->payload + payload_offset, (uint8_t *)buffer + buffer_offset, copy_count);
                buffer_offset = buffer_offset + copy_count;
            }

            /* release descriptors to DMA */
            /* point to first descriptor */
            dma_rx_desc = rx_frame.rx_fs_desc;

            /* set dav bit in Rx descriptors: gives the buffers back to DMA */
            for (uint32_t i = 0; i < rx_frame.seg_count; i ++)
            {
                dma_rx_desc->status |= ENET_RDES0_DAV;
                dma_rx_desc = (enet_descriptors_struct *)(dma_rx_desc->buffer2_next_desc_addr);
            }

            rx_frame.seg_count = 0;

#ifdef ETH_RX_DUMP
            dump_hex(buffer, len);
#endif
        }
        else
        {
            LOG_W("pbuf alloc faild, length: %u", len);
        }
    }

    /* when rx buffer unavailable flag is set: clear it and resume reception */
    if (ENET_DMA_STAT & ENET_DMA_STAT_RBU)
    {
        /* clear RBU flag */
        ENET_DMA_STAT = ENET_DMA_STAT_RBU;
        /* resume DMA reception by writing to the RPEN register */
        ENET_DMA_RPEN = 0U;
    }

    return p;
}

void ENET_IRQHandler (void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    /* frame received */
    if (enet_interrupt_flag_get(ENET_DMA_INT_FLAG_RS) == SET)
    {
        /* give the semaphore to wakeup LwIP task */
        if (eth_device_ready(&eth_dev) != RT_EOK)
        {
            LOG_W("eth_device_ready failed");
        }
        enet_interrupt_flag_clear(ENET_DMA_INT_FLAG_RS_CLR);
    }

    enet_interrupt_flag_clear(ENET_DMA_INT_FLAG_NI_CLR);

    /* leave interrupt */
    rt_interrupt_leave();
}

/* Register the eth device */
static int rt_hw_gd32_eth_init (void)
{
    /* OUI 00-80-E1 STMICROELECTRONICS. */
    mac_addr[0] = 0x00;
    mac_addr[1] = 0x80;
    mac_addr[2] = 0xE1;
    /* generate MAC addr from 96bit unique ID (only for test). */
    mac_addr[3] = *(rt_uint8_t *)(UID_BASE + 4);
    mac_addr[4] = *(rt_uint8_t *)(UID_BASE + 2);
    mac_addr[5] = *(rt_uint8_t *)(UID_BASE + 0);

    eth_dev.parent.init       = rt_gd32_eth_init;
    eth_dev.parent.open       = rt_gd32_eth_open;
    eth_dev.parent.close      = rt_gd32_eth_close;
    eth_dev.parent.read       = rt_gd32_eth_read;
    eth_dev.parent.write      = rt_gd32_eth_write;
    eth_dev.parent.control    = rt_gd32_eth_control;
    eth_dev.parent.user_data  = RT_NULL;

    eth_dev.eth_rx = rt_gd32_eth_rx;
    eth_dev.eth_tx = rt_gd32_eth_tx;

    /* register eth device */
    if (eth_device_init(&eth_dev, "e0") == RT_EOK)
    {
        LOG_D("eth device init success");
    }
    else
    {
        LOG_E("eth device init faild\n");
        return -RT_ERROR;
    }

    return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_hw_gd32_eth_init);


#ifdef BSP_ETH_COMMAND_LINE_DEBUG
static void eth_tx_des_print (void)
{
    rt_kprintf("current tx descriptors[%p]:\n", dma_current_txdesc);
    rt_kprintf("\tstatus: 0x%08X\n", dma_current_txdesc->status);
    rt_kprintf("\tcontrol_buffer_size: 0x%08X\n", dma_current_txdesc->control_buffer_size);
    rt_kprintf("\tbuffer1_addr: 0x%08X\n", dma_current_txdesc->buffer1_addr);
    rt_kprintf("\tbuffer2_next_desc_addr: 0x%08X\n", dma_current_txdesc->buffer2_next_desc_addr);

    rt_kprintf("\ncurrent dma tx descriptors: %p\n", ENET_DMA_CTDADDR);

    rt_kprintf("all tx descriptors:\n");
    for (uint8_t i = 0; i < ENET_TXBUF_NUM; i ++)
    {
        rt_kprintf("tx descriptors[%p]:\n", &txdesc_tab[i]);
        rt_kprintf("\tstatus: 0x%08X\n", txdesc_tab[i].status);
        rt_kprintf("\tcontrol_buffer_size: 0x%08X\n", txdesc_tab[i].control_buffer_size);
        rt_kprintf("\tbuffer1_addr: 0x%08X\n", txdesc_tab[i].buffer1_addr);
        rt_kprintf("\tbuffer2_next_desc_addr: 0x%08X\n", txdesc_tab[i].buffer2_next_desc_addr);
    }
}
MSH_CMD_EXPORT(eth_tx_des_print, print tx descriptors);

static void eth_msc_print (void)
{
    rt_kprintf("rx packets: %u\n", ENET_MSC_RGUFCNT);
    rt_kprintf("rx align errors: %u\n", ENET_MSC_RFAECNT);
    rt_kprintf("rx align errors: %u\n", ENET_MSC_RFAECNT);
    rt_kprintf("rx crc errors: %u\n", ENET_MSC_RFCECNT);

    rt_kprintf("tx packets: %u\n", ENET_MSC_TGFCNT);
}
MSH_CMD_EXPORT(eth_msc_print, print mac statistics counters);

#ifdef RT_LWIP_USING_HW_CHECKSUM
static void eth_rx_err_print (void)
{
    rt_kprintf("receive error frame count: %u\n", rx_err_cnt);
}
MSH_CMD_EXPORT(eth_rx_err_print, print the number of eth rx errors);
#endif

static void eth_dma_status (void)
{
    rt_kprintf("0x%08X\n", ENET_DMA_STAT);
}
MSH_CMD_EXPORT(eth_dma_status, print eth dma status register);

#endif /* BSP_ETH_COMMAND_LINE_DEBUG */
