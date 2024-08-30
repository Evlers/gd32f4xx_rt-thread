/*
 * Copyright (c) 2006-2024 LGT Development Team
 *
 * Change Logs:
 * Date        	Author     	Notes
 * 2024-07-04	Evlers      first implementation
 * 2024-08-19	Evlers		add trng peripheral
 */

#include <stdlib.h>
#include <stdint.h>

#include "random.h"
#include "rtthread.h"
#include "board.h"

static unsigned int last_seed = 1;

static ErrStatus trng_ready_check (void)
{
    uint32_t timeout = 0;
    FlagStatus trng_flag = RESET;
    ErrStatus reval = SUCCESS;

    /* check wherther the random data is valid */
    do
    {
        timeout ++;
        trng_flag = trng_flag_get(TRNG_FLAG_DRDY);
    } while((RESET == trng_flag) &&(0xFFFF > timeout));
    
    if (RESET == trng_flag)
    {   
        /* ready check timeout */
        rt_kprintf("Error: TRNG can't ready\n");
        trng_flag = trng_flag_get(TRNG_FLAG_CECS);
        rt_kprintf("Clock error current status: %d\n", trng_flag);
        trng_flag = trng_flag_get(TRNG_FLAG_SECS);
        rt_kprintf("Seed error current status: %d\n", trng_flag);  
        reval = ERROR;
    }

    /* return check status */
    return reval;
}

static ErrStatus trng_configuration (void)
{
    ErrStatus reval = SUCCESS;

    /* TRNG module clock enable */
    rcu_periph_clock_enable(RCU_TRNG);

    /* TRNG registers reset */
    trng_deinit();
    trng_enable();
    /* check TRNG work status */
    reval = trng_ready_check();

    return reval;
}

static int trng_init (void)
{
    uint8_t retry = 0;

    while ((ERROR == trng_configuration()) && retry < 3)
    {
        rt_kprintf("TRNG init fail\n");
        rt_kprintf("TRNG init retry\n");
        retry ++;
    }

    return RT_EOK;
}
INIT_PREV_EXPORT(trng_init);

static int do_random(unsigned int seed)
{
    srand(trng_get_true_random_data() ^ seed);
    return rand();
}

int random_number(void)
{
    unsigned int seed = (unsigned int) rt_tick_get();
    last_seed += (seed >> ((seed ^ last_seed) % 3));
    return do_random(last_seed ^ seed);
}

// random number range interval [min, max)
int random_number_range(unsigned int min, unsigned int max)
{
    return (random_number() % (max - min)) + min;
}

/**
 * @brief Get the random number data
 *
 * @param bytes Byte array data
 * @param size Array size
 */
void random_bytes (uint8_t *bytes, uint32_t size)
{
    for (uint32_t i = 0; i < size; i ++)
    {
        bytes[i] = random_number();
    }
}

void random_string(char *str, unsigned int len)
{
    unsigned int i, flag, seed, random;

    if (NULL == str)
        return ;
    len -= 1;
    seed = (unsigned int) random_number();
    seed += (unsigned int) ((size_t)str ^ seed);

    random = (unsigned int)do_random(seed);

    for (i = 0; i < len; i ++)
    {
        random = do_random(seed ^ random);
        flag = (unsigned int)random % 3;
        switch (flag)
        {
            case 0:
                str[i] = 'A' + do_random(random ^ (i & flag)) % 26;
            break;
            case 1:
                str[i] = 'a' + do_random(random ^ (i & flag)) % 26;
            break;
            case 2:
                str[i] = '0' + do_random(random ^ (i & flag)) % 10;
            break;
            default:
                str[i] = 'x';
            break;
        }
        random += ((0xb433e5c6 ^ random)  << (i & flag));
    }

    str[len] = '\0';
}

void random_hex_string(char *str, unsigned int len)
{
    unsigned int i, flag, seed, random;

    if (NULL == str)
        return ;
    len -= 1;
    seed = (unsigned int) random_number();
    seed += (unsigned int) ((size_t)str ^ seed);

    random = (unsigned int)do_random(seed);

    for (i = 0; i < len; i ++)
    {
        random = do_random(seed ^ random);
        flag = (unsigned int)random % 2;
        switch (flag)
        {
            case 0:
                str[i] = '0' + do_random(random ^ (i & flag)) % 9;
            break;
            case 1:
                str[i] = 'a' + do_random(random ^ (i & flag)) % 5;
            break;
        }
        random += ((0xb433e5c6 ^ random)  << (i & flag));
    }

    str[len] = '\0';
}
