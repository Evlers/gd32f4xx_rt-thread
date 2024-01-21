/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date             Author          Notes
 * 2024-01-20       Evlers          first version
 */

#include "stdint.h"
#include "drv_sdio_crc.h"

/**
 * @brief Calculate CRC7 bit by bit
 * 
 * @param crc The last or initialized crc value
 * @param byte Bit data
 * @param bits The number of bit to be calculated (max is 8)
 * @return uint16_t Output CRC7 value
 */
static uint8_t calc_crc7 (uint8_t crc, uint8_t byte, uint8_t bits)
{
    for (int i = bits - 1; i >= 0; i --)
	{
		uint8_t bit = (byte >> i) & 0x0001;
		uint8_t crchigh = (crc >> 6) & 0x0001;
		uint8_t xorb  = crchigh ^ bit;
		crc <<= 1;
		crc  &= 0x7F;
        crc  ^= (xorb << 3) | xorb;
	}

	return crc;
}

/**
 * @brief Calculate CRC16 bit by bit
 * 
 * @param crc The last or initialized crc value
 * @param byte Bit data
 * @param bits The number of bit to be calculated (max is 8)
 * @return uint16_t Output CRC16 value
 */
static uint16_t calc_crc16 (uint16_t crc, uint8_t byte, uint8_t bits)
{
	for (int i = bits - 1; i >= 0; i --)
	{
		uint16_t bit = (byte >> i) & 0x0001;
		uint16_t crchigh = (crc >> 15) & 0x0001;
		uint16_t xorb  = crchigh ^ bit;
		crc <<= 1;
		crc  &= 0xFFFF;
        crc  ^= (xorb << 12) | (xorb << 5) | xorb;
	}

	return crc;
}

/**
 * @brief Compute CRC7 bit by bit for the data buffer
 * 
 * @param ptr Bit data buffer
 * @param bit_num Number of bits
 * @return uint16_t Output CRC7 value
 */
uint8_t sdio_crc7_calc (const uint8_t *ptr, uint32_t bit_num)
{
	uint32_t index = 0;
	uint8_t crc = 0;

	while (index < bit_num)
	{
		uint8_t calc_bit_num = (bit_num - index) >= 8 ? 8 : bit_num - index;
		crc = calc_crc7(crc, ptr[index / 8], calc_bit_num);
		index += calc_bit_num;
	}

	return crc;
}

/**
 * @brief Compute CRC16 bit by bit for the data buffer
 * 
 * @param ptr Bit data buffer
 * @param bit_num Number of bits
 * @return uint16_t Output CRC16 value
 */
uint16_t sdio_crc16_calc (const uint8_t *ptr, uint32_t bit_num)
{
	uint32_t index = 0;
	uint16_t crc = 0;

	while (index < bit_num)
	{
		uint8_t calc_bit_num = (bit_num - index) >= 8 ? 8 : bit_num - index;
		crc = calc_crc16(crc, ptr[index / 8], calc_bit_num);
		index += calc_bit_num;
	}

	return crc;
}


/**
 * The SDIO data CRC16 value needs to be calculated separately for each data wire
 * Calculated CRC16 on a bitwise basis for each data wire.
 * After calculating the CRC value on each data line, it needs to be combined into bus data and sent out.
 * Because the CRC16 value is only transmitted on each data line.
 */

/**
 * @brief Split the bus data onto each data wire
 * 
 * @param each_data_wire Output the data of each data wire
 * @param bus_data Input bus data
 * @param len Bus data length
 */
static void split_4bit_bus_data (uint8_t *each_data_wire[4], uint8_t *bus_data, uint32_t len)
{
	for (uint32_t i = 0; i < len; i ++)
	{
		uint8_t temp = i >= len ? 0 : bus_data[i];
		uint8_t j = i / 4;
		
		each_data_wire[3][j] <<= 1;
		each_data_wire[2][j] <<= 1;
		each_data_wire[1][j] <<= 1;
		each_data_wire[0][j] <<= 1;
		
		if (temp & 0x80) each_data_wire[3][j] |= 0x01;
		else each_data_wire[3][j] &= ~0x01;
		
		if (temp & 0x40) each_data_wire[2][j] |= 0x01;
		else each_data_wire[2][j] &= ~0x01;
		
		if (temp & 0x20) each_data_wire[1][j] |= 0x01;
		else each_data_wire[1][j] &= ~0x01;
		
		if (temp & 0x10) each_data_wire[0][j] |= 0x01;
		else each_data_wire[0][j] &= ~0x01;
		
		each_data_wire[3][j] <<= 1;
		each_data_wire[2][j] <<= 1;
		each_data_wire[1][j] <<= 1;
		each_data_wire[0][j] <<= 1;
		
		if (temp & 0x08) each_data_wire[3][j] |= 0x01;
		else each_data_wire[3][j] &= ~0x01;
		
		if (temp & 0x04) each_data_wire[2][j] |= 0x01;
		else each_data_wire[2][j] &= ~0x01;
		
		if (temp & 0x02) each_data_wire[1][j] |= 0x01;
		else each_data_wire[1][j] &= ~0x01;
		
		if (temp & 0x01) each_data_wire[0][j] |= 0x01;
		else each_data_wire[0][j] &= ~0x01;
	}
}

/**
 * @brief Calculates the CRC16 value of the each data wire
 * 
 * @param crc_value Output the CRC16 value of each data wire
 * @param each_data_wire Input the data of each data wire
 * @param bit_num The number of bits per data wire
 */
static void calc_each_data_wire_crc16 (uint8_t crc_value[8], uint8_t *each_data_wire[4], uint32_t bit_num)
{
	uint16_t crc16;

	/* Calculate the CRC16/CCITT of the each data wire */
	crc16 = sdio_crc16_calc(each_data_wire[0], bit_num);
	crc_value[0] = (uint8_t )(crc16 >> 8);
	crc_value[1] = (uint8_t )crc16;
	
	crc16 = sdio_crc16_calc(each_data_wire[1], bit_num);
	crc_value[2] = (uint8_t )(crc16 >> 8);
	crc_value[3] = (uint8_t )crc16;
	
	crc16 = sdio_crc16_calc(each_data_wire[2], bit_num);
	crc_value[4] = (uint8_t )(crc16 >> 8);
	crc_value[5] = (uint8_t )crc16;
	
	crc16 = sdio_crc16_calc(each_data_wire[3], bit_num);
	crc_value[6] = (uint8_t )(crc16 >> 8);
	crc_value[7] = (uint8_t )crc16;
}

/**
 * @brief Merge CRC values for each data wire into the bus
 * 
 * @param output Merge to the bus for crc data
 * @param input CRC16 data for each data wire
 */
static void merge_crc_to_bus (uint8_t output[8], uint8_t input[8])
{
	uint8_t movebit1 = 0x80, movebit2 = 0x40;

	for (uint8_t i = 0; i < 8; i ++)
	{
		uint8_t k = i / 4;
		
		if (i == 4)
		{
			movebit1 = 0x80;
			movebit2 = 0x40;
		}
		
		for (uint8_t j = 0; j < 8; j += 2)
		{
			uint8_t temp = input[j + k];
			output[i] >>= 1;
			
			if (temp & movebit1) output[i] |= 0x80;
			else output[i] &= ~0x80;
			
			if(temp & movebit2) output[i] |= 0x08;
			else output[i] &= ~0x08;
		}

		movebit1 >>= 2;
		movebit2 >>= 2;
	}
}

/**
 * @brief Calculate CRC16 for SDIO bus data
 * 
 * @param ptr SDIO data (4wire)
 * @param crc Output CRC16 value[8]
 * @param len Length for sdio data (max is 512)
 */
void sdio_crc16_calc_4bit_bus (uint8_t *ptr, uint8_t *crc, uint16_t len)
{
	const int data_wire_buf_len = DRV_SDIO_CRC16_4BIT_BUS_DATA_MAX_LEN / 8 / 4;
	uint8_t data_wire[4][data_wire_buf_len] = { 0 };
	uint8_t *each_data_wire[4] = { data_wire[0], data_wire[1], data_wire[2], data_wire[3] };
	uint8_t crc_buff[8] = { 0 };

	/* The bus data length must not exceed the allowed range */
	if (len > DRV_SDIO_CRC16_4BIT_BUS_DATA_MAX_LEN) return ;

	/* Split the bus data onto each data wire */
	split_4bit_bus_data(each_data_wire, ptr, len);

	/* Calculates the CRC16 value of the each data wire */
	calc_each_data_wire_crc16(crc_buff, each_data_wire, len * 8 / 4);

	/* Merge CRC values for each data wire into the bus */
	merge_crc_to_bus(crc, crc_buff);
}
