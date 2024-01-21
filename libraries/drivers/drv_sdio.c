/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date             Author          Notes
 * 2023-12-30       Evlers          first version
 * 2024-01-21       Evlers          Add support for byte stream data transfer software CRC16
 */

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>

#include <drv_sdio.h>
#include "drv_sdio_crc.h"

#define DBG_TAG             "drv.sdio"
#define DBG_LVL             DBG_ERROR
#define DBG_COLOR
#include <rtdbg.h>

#ifdef BSP_USING_SDIO

#define SDIO_TX_RX_COMPLETE_TIMEOUT_LOOPS       (1000000)

#define RTHW_SDIO_LOCK(_sdio)                   rt_mutex_take(&_sdio->mutex, RT_WAITING_FOREVER)
#define RTHW_SDIO_UNLOCK(_sdio)                 rt_mutex_release(&_sdio->mutex);


struct sdio_pkg
{
    struct rt_mmcsd_cmd *cmd;
    void *buff;
    rt_uint32_t flag;
};

struct rthw_sdio
{
    struct rt_mmcsd_host *host;
    struct gd32_sdio_des sdio_des;
    struct rt_event event;
    struct rt_mutex mutex;
    struct sdio_pkg *pkg;
};
rt_align(SDIO_ALIGN)

static rt_uint8_t cache_buf[SDIO_BUFF_SIZE];

static rt_uint32_t gd32_sdio_clk_get(uint32_t hw_sdio)
{
    return SDIO_CLOCK_FREQ;
}

/*!
    \brief      get the data block size
    \param[in]  bytesnumber: the number of bytes
    \param[out] none
    \retval     data block size
      \arg        SDIO_DATABLOCKSIZE_1BYTE: block size = 1 byte
      \arg        SDIO_DATABLOCKSIZE_2BYTES: block size = 2 bytes
      \arg        SDIO_DATABLOCKSIZE_4BYTES: block size = 4 bytes
      \arg        SDIO_DATABLOCKSIZE_8BYTES: block size = 8 bytes
      \arg        SDIO_DATABLOCKSIZE_16BYTES: block size = 16 bytes
      \arg        SDIO_DATABLOCKSIZE_32BYTES: block size = 32 bytes
      \arg        SDIO_DATABLOCKSIZE_64BYTES: block size = 64 bytes
      \arg        SDIO_DATABLOCKSIZE_128BYTES: block size = 128 bytes
      \arg        SDIO_DATABLOCKSIZE_256BYTES: block size = 256 bytes
      \arg        SDIO_DATABLOCKSIZE_512BYTES: block size = 512 bytes
      \arg        SDIO_DATABLOCKSIZE_1024BYTES: block size = 1024 bytes
      \arg        SDIO_DATABLOCKSIZE_2048BYTES: block size = 2048 bytes
      \arg        SDIO_DATABLOCKSIZE_4096BYTES: block size = 4096 bytes
      \arg        SDIO_DATABLOCKSIZE_8192BYTES: block size = 8192 bytes
      \arg        SDIO_DATABLOCKSIZE_16384BYTES: block size = 16384 bytes
*/
static uint32_t sd_datablocksize_get(uint16_t bytesnumber)
{
    uint8_t exp_val = 0;
    /* calculate the exponent of 2 */
    while(1 != bytesnumber){
        bytesnumber >>= 1;
        ++exp_val;
    }
    return DATACTL_BLKSZ(exp_val);
}

static void rthw_sdio_wait_completed(struct rthw_sdio *sdio)
{
    rt_uint32_t status;
    struct rt_mmcsd_cmd *cmd = sdio->pkg->cmd;
    struct rt_mmcsd_data *data = cmd->data;
    int err_level = DBG_ERROR;

    if (rt_event_recv(&sdio->event, 0xffffffff, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                      rt_tick_from_millisecond(5000), &status) != RT_EOK)
    {
        LOG_E("wait completed timeout!!");
        cmd->err = -RT_ETIMEOUT;
        return;
    }

    if (sdio->pkg == RT_NULL)
    {
        return;
    }

    cmd->resp[0] = sdio_response_get(SDIO_RESPONSE0);
    if (resp_type(cmd) == RESP_R2)
    {
        cmd->resp[1] = sdio_response_get(SDIO_RESPONSE1);
        cmd->resp[2] = sdio_response_get(SDIO_RESPONSE2);
        cmd->resp[3] = sdio_response_get(SDIO_RESPONSE3);
    }

    if (status & HW_SDIO_ERRORS)
    {
        if ((status & SDIO_STAT_CCRCERR) && (resp_type(cmd) & (RESP_R3 | RESP_R4)))
        {
            cmd->err = RT_EOK;
        }
        else
        {
            cmd->err = -RT_ERROR;
        }

        if (status & SDIO_STAT_CMDTMOUT)
        {
            cmd->err = -RT_ETIMEOUT;
        }

        if (status & SDIO_STAT_DTCRCERR)
        {
            data->err = -RT_ERROR;
        }

        if (status & SDIO_STAT_DTTMOUT)
        {
            data->err = -RT_ETIMEOUT;
        }

        if (cmd->err == RT_EOK)
        {
            goto __print_status;
        }
        else
        {
            if ((cmd->cmd_code == 5) || (cmd->cmd_code == 8))
            {
                err_level = DBG_WARNING;
            }
            dbg_log(err_level, "error: 0x%08x, %s%s%s%s%s%s%s cmd: %d arg: 0x%08x data_dir: %c len: %d blksize: %d\n",
                    status,
                    status & SDIO_STAT_CCRCERR  ? "CCRCFAIL "   : "",
                    status & SDIO_STAT_DTCRCERR ? "DCRCFAIL "   : "",
                    status & SDIO_STAT_CMDTMOUT ? "CTIMEOUT "   : "",
                    status & SDIO_STAT_DTTMOUT  ? "DTIMEOUT "   : "",
                    status & SDIO_STAT_TXURE    ? "TXUNDERR "   : "",
                    status & SDIO_STAT_RXORE    ? "RXOVERR "    : "",
                    status == 0                 ? "NULL"        : "",
                    cmd->cmd_code,
                    cmd->arg,
                    data ? (data->flags & DATA_DIR_WRITE ?  'w' : 'r') : '-',
                    data ? data->blks * data->blksize : 0,
                    data ? data->blksize : 0
                   );
        }
    }
    else
    {
        cmd->err = RT_EOK;
        __print_status:
        LOG_D("status: 0x%08X [%08X %08X %08X %08X]", status, cmd->resp[0], cmd->resp[1], cmd->resp[2], cmd->resp[3]);
    }
}

static void rthw_sdio_transfer_by_dma(struct rthw_sdio *sdio, struct sdio_pkg *pkg)
{
    int size;
    dma_multi_data_parameter_struct dma_struct;

    RT_ASSERT(sdio != RT_NULL);
    RT_ASSERT(pkg != RT_NULL);
    RT_ASSERT(pkg->cmd->data != RT_NULL);
    RT_ASSERT(pkg->buff != RT_NULL);

    size = pkg->cmd->data->blks * pkg->cmd->data->blksize;

    /* clear all the interrupt flags */
    dma_flag_clear(SDIO_DMA, SDIO_DMA_CHANNEL, DMA_FLAG_FEE);
    dma_flag_clear(SDIO_DMA, SDIO_DMA_CHANNEL, DMA_FLAG_SDE);
    dma_flag_clear(SDIO_DMA, SDIO_DMA_CHANNEL, DMA_FLAG_TAE);
    dma_flag_clear(SDIO_DMA, SDIO_DMA_CHANNEL, DMA_FLAG_HTF);
    dma_flag_clear(SDIO_DMA, SDIO_DMA_CHANNEL, DMA_FLAG_FTF);
    dma_channel_disable(SDIO_DMA, SDIO_DMA_CHANNEL);
    dma_deinit(SDIO_DMA, SDIO_DMA_CHANNEL);

    sdio_dma_enable();
    if (pkg->cmd->data->flags & DATA_DIR_WRITE)
    {
        /* configure the DMA channel */
        dma_struct.periph_addr        = (uint32_t)&SDIO_FIFO;
        dma_struct.memory0_addr       = (uint32_t)pkg->buff;
        dma_struct.direction          = DMA_MEMORY_TO_PERIPH;
        dma_struct.number             = size;
        dma_struct.periph_inc         = DMA_PERIPH_INCREASE_DISABLE;
        dma_struct.memory_inc         = DMA_MEMORY_INCREASE_ENABLE;
        dma_struct.circular_mode      = DMA_CIRCULAR_MODE_DISABLE;

        if (pkg->cmd->data->flags & DATA_STREAM)
        {
            dma_struct.periph_width       = DMA_PERIPH_WIDTH_32BIT;
            dma_struct.memory_width       = DMA_MEMORY_WIDTH_8BIT;
            dma_struct.priority           = DMA_PRIORITY_ULTRA_HIGH;
            dma_struct.periph_burst_width = DMA_PERIPH_BURST_4_BEAT;
            dma_struct.memory_burst_width = DMA_MEMORY_BURST_SINGLE;
            dma_struct.critical_value     = DMA_FIFO_4_WORD;
        }
        else
        {
            dma_struct.periph_width       = DMA_PERIPH_WIDTH_32BIT;
            dma_struct.memory_width       = DMA_MEMORY_WIDTH_32BIT;
            dma_struct.priority           = DMA_PRIORITY_ULTRA_HIGH;
            dma_struct.periph_burst_width = DMA_PERIPH_BURST_4_BEAT;
            dma_struct.memory_burst_width = DMA_MEMORY_BURST_4_BEAT;
            dma_struct.critical_value     = DMA_FIFO_4_WORD;
        }
        dma_multi_data_mode_init(SDIO_DMA, SDIO_DMA_CHANNEL, &dma_struct);

        dma_flow_controller_config(SDIO_DMA, SDIO_DMA_CHANNEL, DMA_FLOW_CONTROLLER_PERI);
        dma_channel_subperipheral_select(SDIO_DMA, SDIO_DMA_CHANNEL, SDIO_DMA_SUBPERI);
        dma_channel_enable(SDIO_DMA, SDIO_DMA_CHANNEL);
    }
    else if (pkg->cmd->data->flags & DATA_DIR_READ)
    {
        /* configure the DMA channel */
        dma_struct.periph_addr        = (uint32_t)&SDIO_FIFO;
        dma_struct.memory0_addr       = (uint32_t)pkg->buff;
        dma_struct.direction          = DMA_PERIPH_TO_MEMORY;
        dma_struct.number             = size;
        dma_struct.periph_inc         = DMA_PERIPH_INCREASE_DISABLE;
        dma_struct.memory_inc         = DMA_MEMORY_INCREASE_ENABLE;
        dma_struct.circular_mode      = DMA_CIRCULAR_MODE_DISABLE;

        if (pkg->cmd->data->flags & DATA_STREAM)
        {
            dma_struct.periph_width       = DMA_PERIPH_WIDTH_32BIT;
            dma_struct.memory_width       = DMA_MEMORY_WIDTH_8BIT;
            dma_struct.priority           = DMA_PRIORITY_ULTRA_HIGH;
            dma_struct.periph_burst_width = DMA_PERIPH_BURST_SINGLE;
            dma_struct.memory_burst_width = DMA_MEMORY_BURST_SINGLE;
            dma_struct.critical_value     = DMA_FIFO_4_WORD;
        }
        else
        {
            dma_struct.periph_width       = DMA_PERIPH_WIDTH_32BIT;
            dma_struct.memory_width       = DMA_MEMORY_WIDTH_32BIT;
            dma_struct.priority           = DMA_PRIORITY_ULTRA_HIGH;
            dma_struct.periph_burst_width = DMA_PERIPH_BURST_4_BEAT;
            dma_struct.memory_burst_width = DMA_MEMORY_BURST_4_BEAT;
            dma_struct.critical_value     = DMA_FIFO_4_WORD;
        }
        dma_multi_data_mode_init(SDIO_DMA, SDIO_DMA_CHANNEL, &dma_struct);

        dma_flow_controller_config(SDIO_DMA, SDIO_DMA_CHANNEL, DMA_FLOW_CONTROLLER_PERI);
        dma_channel_subperipheral_select(SDIO_DMA, SDIO_DMA_CHANNEL, SDIO_DMA_SUBPERI);
        dma_channel_enable(SDIO_DMA, SDIO_DMA_CHANNEL);

        /* enable the DSM(data state machine) for data transfer */
        sdio_dsm_enable();
    }
}

static void rthw_sdio_send_command(struct rthw_sdio *sdio, struct sdio_pkg *pkg)
{
    struct rt_mmcsd_cmd *cmd = pkg->cmd;
    struct rt_mmcsd_data *data = cmd->data;

    rt_event_control(&sdio->event, RT_IPC_CMD_RESET, RT_NULL);

    /* save pkg */
    sdio->pkg = pkg;

    LOG_D("cmd: %d arg: 0x%08x response_type: %s%s%s%s%s%s%s%s%s data_dir: %c len: %d blksize: %d",
            cmd->cmd_code,
            cmd->arg,
            resp_type(cmd) == RESP_NONE ? "NONE": "",
            resp_type(cmd) == RESP_R1   ? "R1"  : "",
            resp_type(cmd) == RESP_R1B  ? "R1B" : "",
            resp_type(cmd) == RESP_R2   ? "R2"  : "",
            resp_type(cmd) == RESP_R3   ? "R3"  : "",
            resp_type(cmd) == RESP_R4   ? "R4"  : "",
            resp_type(cmd) == RESP_R5   ? "R5"  : "",
            resp_type(cmd) == RESP_R6   ? "R6"  : "",
            resp_type(cmd) == RESP_R7   ? "R7"  : "",
            data ? (data->flags & DATA_DIR_WRITE ?  'w' : 'r') : '-',
            data ? data->blks * data->blksize : 0,
            data ? data->blksize : 0
           );

    /* config data transfer */
    if (data != RT_NULL)
    {
        /* clear all DSM configuration */
        sdio_data_config(0, 0, SDIO_DATABLOCKSIZE_1BYTE);
        sdio_data_transfer_config(SDIO_TRANSMODE_BLOCK, SDIO_TRANSDIRECTION_TOCARD);
        sdio_dsm_disable();
        sdio_dma_disable();

        /* Hardware CRC16 computation is not supported in the case of byte stream transmission */
        if ((data->flags & DATA_STREAM) && (data->flags & DATA_DIR_WRITE))
        {
            if (data->blks == 1 && data->blksize < DRV_SDIO_CRC16_4BIT_BUS_DATA_MAX_LEN)
            {
                /* Using software crc */
                sdio_crc16_calc_4bit_bus((uint8_t *)pkg->buff, (uint8_t *)pkg->buff + data->blksize, data->blksize);
                data->blksize += 8;
            }
        }

        /* sdio data configure */
        sdio_data_config(HW_SDIO_DATATIMEOUT, data->blks * data->blksize, sd_datablocksize_get(data->blksize));
        sdio_data_transfer_config((data->flags & DATA_STREAM) ? SDIO_TRANSMODE_STREAM : SDIO_TRANSMODE_BLOCK, 
                                    (data->flags & DATA_DIR_READ) ? SDIO_TRANSDIRECTION_TOSDIO : SDIO_TRANSDIRECTION_TOCARD);

        sdio_operation_enable();

        /* DMA transfer config */
        rthw_sdio_transfer_by_dma(sdio, pkg);
    }

    /* enable interrupt */
    sdio_interrupt_flag_clear(SDIO_INT_FLAG_CMDSEND | SDIO_INT_FLAG_CMDRECV | HW_SDIO_ERRORS);
    sdio_interrupt_enable(SDIO_INTEN_CMDSENDIE | SDIO_INTEN_CMDRECVIE | HW_SDIO_ERRORS);
    if (data != RT_NULL)
    {
        sdio_interrupt_flag_clear(SDIO_INT_FLAG_DTEND);
        sdio_interrupt_enable(SDIO_INTEN_DTENDIE);
    }

    /* send command */
    uint32_t response_type;
    if (resp_type(cmd) == RESP_NONE)
        response_type = SDIO_RESPONSETYPE_NO;
    else if (resp_type(cmd) == RESP_R2)
        response_type = SDIO_RESPONSETYPE_LONG;
    else
        response_type = SDIO_RESPONSETYPE_SHORT;

    sdio_command_response_config(cmd->cmd_code, cmd->arg, response_type);
    sdio_wait_type_set(SDIO_WAITTYPE_NO);
    sdio_csm_enable();

    /* wait completed */
    rthw_sdio_wait_completed(sdio);

    /* Wait for the data transfer to complete */
    if (data != RT_NULL)
    {
        volatile rt_uint32_t count = SDIO_TX_RX_COMPLETE_TIMEOUT_LOOPS;

        while (count && (SDIO_STAT & (SDIO_STAT_TXRUN | SDIO_STAT_RXRUN)))
        {
            count--;
        }

        if ((count == 0) || (SDIO_STAT & HW_SDIO_ERRORS))
        {
            cmd->err = -RT_ERROR;
        }
    }

    /* close irq, keep sdio irq */
    SDIO_INTEN = SDIO_INTEN & SDIO_STAT_SDIOINT ? SDIO_STAT_SDIOINT : 0x00;

    /* clear pkg */
    sdio->pkg = RT_NULL;
}

static void rthw_sdio_request(struct rt_mmcsd_host *host, struct rt_mmcsd_req *req)
{
    struct sdio_pkg pkg;
    struct rthw_sdio *sdio = host->private_data;
    struct rt_mmcsd_data *data;

    RTHW_SDIO_LOCK(sdio);

    if (req->cmd != RT_NULL)
    {
        memset(&pkg, 0, sizeof(pkg));
        data = req->cmd->data;
        pkg.cmd = req->cmd;

        if (data != RT_NULL)
        {
            rt_uint32_t size = data->blks * data->blksize;

            RT_ASSERT(size <= SDIO_BUFF_SIZE);
            if (data->flags & DATA_STREAM)
            {
                /* CRC16 value with a 4-bit bus width requires 8 bytes */
                RT_ASSERT(size <= (SDIO_BUFF_SIZE - 8));
            }

            /* Use an already-aligned cache buffer */
            pkg.buff = cache_buf;
            if (data->flags & DATA_DIR_WRITE)
            {
                memcpy(cache_buf, data->buf, size);
            }
        }

        rthw_sdio_send_command(sdio, &pkg);

        /* Copy data from the aligned cache buffer */
        if ((data != RT_NULL) && (data->flags & DATA_DIR_READ))
        {
            memcpy(data->buf, cache_buf, data->blksize * data->blks);
        }
    }

    if (req->stop != RT_NULL)
    {
        memset(&pkg, 0, sizeof(pkg));
        pkg.cmd = req->stop;
        rthw_sdio_send_command(sdio, &pkg);
    }

    RTHW_SDIO_UNLOCK(sdio);

    mmcsd_req_complete(sdio->host);
}

static void rthw_sdio_iocfg(struct rt_mmcsd_host *host, struct rt_mmcsd_io_cfg *io_cfg)
{
    rt_uint32_t div, clk_src;
    rt_uint32_t clk = io_cfg->clock;
    struct rthw_sdio *sdio = host->private_data;

    clk_src = sdio->sdio_des.clk_get(sdio->sdio_des.hw_sdio);
    if (clk_src < 400 * 1000)
    {
        LOG_E("The clock rate is too low! rata:%d", clk_src);
        return;
    }

    if (clk > host->freq_max) clk = host->freq_max;

    if (clk > clk_src)
    {
        LOG_W("Setting rate is greater than clock source rate.");
        clk = clk_src;
    }

    LOG_D("clock: %d%s bus_width: %s%s%s power: %s%s%s",
            (clk >= 1000000) ? clk / 1000000 : clk / 1000,
            (clk >= 1000000) ? "MHz" : "KHz",
            io_cfg->bus_width == MMCSD_BUS_WIDTH_8 ? "8" : "",
            io_cfg->bus_width == MMCSD_BUS_WIDTH_4 ? "4" : "",
            io_cfg->bus_width == MMCSD_BUS_WIDTH_1 ? "1" : "",
            io_cfg->power_mode == MMCSD_POWER_OFF ? "OFF" : "",
            io_cfg->power_mode == MMCSD_POWER_UP ? "UP" : "",
            io_cfg->power_mode == MMCSD_POWER_ON ? "ON" : ""
           );

    RTHW_SDIO_LOCK(sdio);

    div = clk_src / clk;
    if ((clk == 0) || (div == 0))
    {
        sdio_hardware_clock_disable();
    }
    else
    {
        if (div < 2)
        {
            div = 2;
        }
        else if (div > 0xFF)
        {
            div = 0xFF;
        }
        div -= 2;

        /* SDIO_CLK IO frequency = SDIOCLK / (DIV[8:0] + 2) */
        sdio_clock_config(SDIO_SDIOCLKEDGE_FALLING, SDIO_CLOCKBYPASS_DISABLE, SDIO_CLOCKPWRSAVE_DISABLE, div);
    }

    if (io_cfg->bus_width == MMCSD_BUS_WIDTH_8)
    {
        sdio_bus_mode_set(SDIO_BUSMODE_8BIT);
    }
    else if (io_cfg->bus_width == MMCSD_BUS_WIDTH_4)
    {
        sdio_bus_mode_set(SDIO_BUSMODE_4BIT);
    }
    else
    {
        sdio_bus_mode_set(SDIO_BUSMODE_1BIT);
    }

    switch (io_cfg->power_mode)
    {
    case MMCSD_POWER_OFF:
        sdio_power_state_set(SDIO_POWER_OFF);
        sdio_clock_disable();
        sdio_hardware_clock_disable();
        break;
    case MMCSD_POWER_UP:
        // sdio_power_state_set(PWRCTL_PWRCTL(2));
        break;
    case MMCSD_POWER_ON:
        sdio_hardware_clock_enable();
        sdio_clock_enable();
        sdio_power_state_set(SDIO_POWER_ON);
        break;
    default:
        LOG_E("unknown power_mode %d", io_cfg->power_mode);
        break;
    }

    RTHW_SDIO_UNLOCK(sdio);
}

static void rthw_sdio_irq_update(struct rt_mmcsd_host *host, rt_int32_t enable)
{
    if (enable)
    {
        LOG_D("enable sdio interrupt");
        sdio_interrupt_flag_clear(SDIO_INT_FLAG_SDIOINT);
        sdio_interrupt_enable(SDIO_STAT_SDIOINT);
    }
    else
    {
        LOG_D("disable sdio interrupt");
        sdio_interrupt_disable(SDIO_STAT_SDIOINT);
    }
}

static rt_int32_t rthw_sd_delect(struct rt_mmcsd_host *host)
{
    LOG_I("try to detect device");
    return 0x01;
}

static void rthw_sdio_irq_process(struct rt_mmcsd_host *host)
{
    int complete = 0;
    struct rthw_sdio *sdio = host->private_data;
    rt_uint32_t intstatus = SDIO_STAT;

    /* Check for errors */
    if (intstatus & HW_SDIO_ERRORS)
    {
        sdio_interrupt_flag_clear(HW_SDIO_ERRORS);
        complete = 1;
    }
    else
    {
        /* command response received (CRC check passed) */
        if (intstatus & SDIO_INT_FLAG_CMDRECV)
        {
            sdio_interrupt_flag_clear(SDIO_INT_FLAG_CMDRECV);

            if (sdio->pkg != RT_NULL)
            {
                /* Check whether data needs to be sent */
                if (!sdio->pkg->cmd->data)
                {
                    complete = 1;
                }
                else if ((sdio->pkg->cmd->data->flags & DATA_DIR_WRITE))
                {
                    /* Continue data transmission */
                    sdio_dsm_enable();
                }
            }
        }

        /* command sent (no response required) */
        if (intstatus & SDIO_INT_FLAG_CMDSEND)
        {
            sdio_interrupt_flag_clear(SDIO_INT_FLAG_CMDSEND);

            /* Check whether an answer is required */
            if (resp_type(sdio->pkg->cmd) == RESP_NONE)
            {
                complete = 1;
            }
        }

        /* data end (data counter, SDIO_DATACNT, is zero) */
        if (intstatus & SDIO_INT_FLAG_DTEND)
        {
            sdio_interrupt_flag_clear(SDIO_INT_FLAG_DTEND);
            complete = 1;
        }
    }

    /* SD I/O interrupt received */
    if ((intstatus & SDIO_INT_FLAG_SDIOINT) && (SDIO_INTEN & SDIO_INT_SDIOINT))
    {
        sdio_interrupt_flag_clear(SDIO_INT_FLAG_SDIOINT);
        sdio_irq_wakeup(host);
    }

    /* Complete a transfer */
    if (complete)
    {
        sdio_interrupt_disable(HW_SDIO_ERRORS);
        rt_event_send(&sdio->event, intstatus);
    }
}

/* mmcsd host interface */
static struct rt_mmcsd_host *host;
static const struct rt_mmcsd_host_ops ops =
{
    rthw_sdio_request,
    rthw_sdio_iocfg,
    rthw_sd_delect,
    rthw_sdio_irq_update,
};

static struct rt_mmcsd_host *sdio_host_create(struct gd32_sdio_des *sdio_des)
{
    struct rt_mmcsd_host *host;
    struct rthw_sdio *sdio = RT_NULL;

    if (sdio_des == RT_NULL)
    {
        LOG_E("%s", (sdio_des == RT_NULL ? "sdio_des is NULL" : ""));
        return RT_NULL;
    }

    sdio = rt_malloc(sizeof(struct rthw_sdio));
    if (sdio == RT_NULL)
    {
        LOG_E("malloc rthw_sdio fail");
        return RT_NULL;
    }
    rt_memset(sdio, 0, sizeof(struct rthw_sdio));

    host = mmcsd_alloc_host();
    if (host == RT_NULL)
    {
        LOG_E("mmcsd alloc host fail");
        rt_free(sdio);
        return RT_NULL;
    }

    rt_memcpy(&sdio->sdio_des, sdio_des, sizeof(struct gd32_sdio_des));
    sdio->sdio_des.hw_sdio = (sdio_des->hw_sdio == RT_NULL ? SDIO_BASE_ADDRESS : sdio_des->hw_sdio);
    sdio->sdio_des.clk_get = (sdio_des->clk_get == RT_NULL ? gd32_sdio_clk_get : sdio_des->clk_get);

    rt_event_init(&sdio->event, "sdio", RT_IPC_FLAG_FIFO);
    rt_mutex_init(&sdio->mutex, "sdio", RT_IPC_FLAG_FIFO);

    // set host defautl attributes
    host->ops = &ops;
    host->freq_min = 400 * 1000;
    host->freq_max = SDIO_MAX_FREQ;
    host->valid_ocr = VDD_32_33 | VDD_33_34;
#ifndef SDIO_USING_1_BIT
    host->flags = MMCSD_BUSWIDTH_4 | MMCSD_MUTBLKWRITE | MMCSD_SUP_SDIO_IRQ |MMCSD_SUP_HIGHSPEED;
#else
    host->flags = MMCSD_MUTBLKWRITE | MMCSD_SUP_SDIO_IRQ;
#endif
    host->max_seg_size = SDIO_BUFF_SIZE;
    host->max_dma_segs = 1;
    host->max_blk_size = 512;
    host->max_blk_count = 512;

    /* link up host and sdio */
    sdio->host = host;
    host->private_data = sdio;

    return host;
}

void SDIO_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    /* Process All SDIO Interrupt Sources */
    rthw_sdio_irq_process(host);

    /* leave interrupt */
    rt_interrupt_leave();
}

/*!
    \brief      configure the GPIO of SDIO interface
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void gpio_config(void)
{
    /* configure the SDIO_DAT0(PC8), SDIO_DAT1(PC9), SDIO_DAT2(PC10), SDIO_DAT3(PC11), SDIO_CLK(PC12) and SDIO_CMD(PD2) */
    gpio_af_set(SDIO_CLK_PORT, GPIO_AF_12, SDIO_CLK_PIN);
    gpio_af_set(SDIO_CMD_PORT, GPIO_AF_12, SDIO_CMD_PIN);
    gpio_af_set(SDIO_D0_PORT,  GPIO_AF_12, SDIO_D0_PIN);
    gpio_af_set(SDIO_D1_PORT,  GPIO_AF_12, SDIO_D1_PIN);
    gpio_af_set(SDIO_D2_PORT,  GPIO_AF_12, SDIO_D2_PIN);
    gpio_af_set(SDIO_D3_PORT,  GPIO_AF_12, SDIO_D3_PIN);

    gpio_mode_set(SDIO_CLK_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SDIO_CLK_PIN);
    gpio_output_options_set(SDIO_CLK_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, SDIO_CLK_PIN);

    gpio_mode_set(SDIO_CMD_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, SDIO_CMD_PIN);
    gpio_output_options_set(SDIO_CMD_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, SDIO_CMD_PIN);

    gpio_mode_set(SDIO_D0_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, SDIO_D0_PIN);
    gpio_output_options_set(SDIO_D0_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, SDIO_D0_PIN);

    gpio_mode_set(SDIO_D1_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, SDIO_D1_PIN);
    gpio_output_options_set(SDIO_D1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, SDIO_D1_PIN);

    gpio_mode_set(SDIO_D2_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, SDIO_D2_PIN);
    gpio_output_options_set(SDIO_D2_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, SDIO_D2_PIN);

    gpio_mode_set(SDIO_D3_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, SDIO_D3_PIN);
    gpio_output_options_set(SDIO_D3_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, SDIO_D3_PIN);
}

/*!
    \brief      configure the RCU of SDIO and DMA
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void rcu_config(void)
{
    rcu_periph_clock_enable(SDIO_GPIO_CLK);
    rcu_periph_clock_enable(SDIO_GPIO_CMD);
    rcu_periph_clock_enable(SDIO_GPIO_D0);
    rcu_periph_clock_enable(SDIO_GPIO_D1);
    rcu_periph_clock_enable(SDIO_GPIO_D2);
    rcu_periph_clock_enable(SDIO_GPIO_D3);

    rcu_periph_clock_enable(SDIO_PERI_CLOCK);
    rcu_periph_clock_enable(SDIO_DMA_CLOCK);
}

int gd32_sdio_init(void)
{
    struct gd32_sdio_des sdio_des;

    /* configure the RCU and GPIO, deinitialize the SDIO */
    rcu_config();
    gpio_config();
    sdio_deinit();

    /* configure the sdio interrupt */
    nvic_irq_enable(SDIO_IRQn, 0, 0);

    /* Save the sdio peripheral address */
    sdio_des.hw_sdio = SDIO_BASE;

    /* Create mmcsd host for sdio */
    host = sdio_host_create(&sdio_des);
    if (host == RT_NULL)
    {
        rt_kprintf("%s host create fail\n");
        return -1;
    }

    /* The sdio interrupt is enabled by default */
    rthw_sdio_irq_update(host, 1);

    /* ready to change */
    mmcsd_change(host);

    return 0;
}
INIT_DEVICE_EXPORT(gd32_sdio_init);

#endif /* BSP_USING_SDIO */
