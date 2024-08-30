
/*
 * Copyright (c) 2006-2024 LGT Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-08-30     Evlers       first implementation
 */

#include "board.h"

#include "rthw.h"
#include "rtthread.h"

#include "utility.h"


void get_chip_uuid (uint8_t *uid, uint8_t len)
{
    memcpy(uid, (const void *)0x1FFF7A10, MIN(len, 12));
}

#ifdef UTILITY_USING_REBOOT

static void reboot (void)
{
    NVIC_SystemReset();
}
MSH_CMD_EXPORT(reboot, Restart the system);

#endif /* UTILITY_USING_REBOOT */

#ifdef UTILITY_USING_PRINT_CLK
static void print_clk (void)
{
    rt_kprintf("HXTAL Clock:\t\t%uMHz\n", HXTAL_VALUE / 1000000);
    rt_kprintf("SystemCoreClock:\t%uMHz\n", SystemCoreClock / 1000000);
    rt_kprintf("System Clock:\t\t%uMHz\n", rcu_clock_freq_get(CK_SYS) / 1000000);
	rt_kprintf("AHB Clock:\t\t%uMHz\n", rcu_clock_freq_get(CK_AHB) / 1000000);
	rt_kprintf("APB1 Clock:\t\t%uMHz\n", rcu_clock_freq_get(CK_APB1) / 1000000);
	rt_kprintf("APB2 Clock:\t\t%uMHz\n", rcu_clock_freq_get(CK_APB2) / 1000000);
}
MSH_CMD_EXPORT(print_clk, print system clock);
#endif

void mem_reverse (uint8_t* buffer, int length)
{
    uint8_t* head = buffer;
    uint8_t* tail = buffer + length;

    if(length > 0)
    {
        uint8_t ch;

        tail--;
        while(head < tail)
        {
            ch = *head;
            *head++ = *tail;    /* h向尾部移动 */
            *tail-- = ch;       /* t向头部移动 */
        }
    }
}

unsigned bcd2bin (unsigned char val)
{
    return (val & 0x0f) + (val >> 4) * 10;
}

unsigned char bin2bcd (unsigned val)
{
    return ((val / 10) << 4) + val % 10;
}

uint8_t sum (uint8_t *ptr, uint32_t n)
{
    uint8_t check = 0;
    while(n--)
    {
        check += *ptr;
        ptr++;
    }

    return check;
}

//x8+x2+x1+1
uint8_t crc8 (uint8_t *ptr, uint32_t len)
{
    uint8_t crc = 0;
    uint32_t i, j;

    for(i=0;i<len; i++)
    {
        crc ^= ptr[i];
        for(j=0;j<8;j++)
        {
            if(crc & 0x80) crc = (crc << 1) ^ 0x07;
            else crc <<= 1;
        }
    }
    return crc;
}

//x8+x5+x4+1
uint8_t crc8_maxim (uint8_t *ptr, uint32_t len)
{
    uint8_t crc;
    uint32_t i;
    crc = 0;
    for(i=0;i< len; i++)
    {
        crc ^= ptr[i];
        for(i = 0;i < 8;i++)
        {
            if(crc & 0x01) crc = (crc >> 1) ^ 0x8C;
            else crc >>= 1;
        }
    }
    return crc;
}

static const uint16_t  crc16_table[256];

uint16_t crc16_ibm (uint16_t crc, uint8_t *ptr, uint32_t len)
{
    for(uint32_t i = 0; i < len; i ++)
    {
        uint16_t data = ptr[i];
        crc = (crc >> 8) ^ crc16_table[(crc ^ data) & 0xff];
    }
    return crc;
}

uint16_t crc16_xmodem (uint8_t *ptr, uint32_t len)
{
    uint8_t i;
    uint16_t crc=0;

    while(len-- != 0)
    {
        for(i=0x80; i != 0; i /= 2)
        {
            if((crc & 0x8000)!=0)
            {
                crc <<= 1;
                crc ^= 0x1021;
            }
            else
            {
                crc <<= 1;
            }

            if((*ptr&i) != 0)
            {
                crc ^= 0x1021;
            }
        }
        ptr++;
    }
    return (crc);
}

uint16_t crc16_modbus (uint8_t *ptr, uint32_t len)
{
    uint32_t crc = 0xFFFF;

    for (uint32_t i = 0; i < len; i ++)
    {
        uint32_t c = *(ptr + i) & 0x00FF;
        crc ^= c;
        for (uint32_t j = 0; j < 8; j ++)
        {
            if (crc & 0x0001)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return(crc);
}

uint32_t crc32 (uint8_t *ptr, uint32_t len)
{
    int j;
    uint32_t i;
    uint32_t crc, mask;

    crc = 0xFFFFFFFF;
    for (i = 0; i < len; i++)
    {
        crc = crc ^ (uint32_t)ptr[i];
        for (j = 7; j >= 0; j--)
        {
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
    }
    return ~crc;
}

void crc32_init_ctx (struct crc32_ctx_t *ctx)
{
    ctx->crc_value = 0xFFFFFFFF;
}

uint32_t crc32_calculate (struct crc32_ctx_t *ctx, uint8_t *ptr, uint32_t len)
{
    int j;
    uint32_t i;
    uint32_t mask;

    for (i = 0; i < len; i++)
    {
        ctx->crc_value = ctx->crc_value ^ (uint32_t)ptr[i];
        for (j = 7; j >= 0; j--)
        {
            mask = -(ctx->crc_value & 1);
            ctx->crc_value = (ctx->crc_value >> 1) ^ (0xEDB88320 & mask);
        }
    }

    return ~ctx->crc_value;
}

/** CRC table for the CRC-16. The poly is 0x8005 (x^16 + x^15 + x^2 + 1) */
static const uint16_t  crc16_table[256] = {
        0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
        0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
        0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
        0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
        0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
        0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
        0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
        0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
        0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
        0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
        0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
        0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
        0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
        0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
        0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
        0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
        0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
        0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
        0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
        0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
        0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
        0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
        0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
        0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
        0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
        0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
        0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
        0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
        0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
        0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
        0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
        0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};
