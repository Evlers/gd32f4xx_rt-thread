/*
 * Copyright (c) 2006-2024, LGT Development Team
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-07-04   Evlers      first implementation
 */

#ifndef _RANDOM_H_
#define _RANDOM_H_

#define RANDOM_MAX 0x7FFFFFFF

int random_number(void);
int random_number_range(unsigned int min, unsigned int max);
void random_bytes (uint8_t *bytes, uint32_t size);
void random_string(char *str, unsigned int len);
void random_hex_string(char *str, unsigned int len);

#endif /* _RANDOM_H_ */
