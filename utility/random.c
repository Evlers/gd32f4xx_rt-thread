#include <stdlib.h>
#include <stdint.h>

#include "random.h"
#include "rtthread.h"

static unsigned int last_seed = 1;

static int do_random(unsigned int seed)
{
    srand(seed);
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

