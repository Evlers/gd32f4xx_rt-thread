/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date             Author          Notes
 * 2024-02-02       Evlers          first version
 */

#ifndef _DRV_SDIO_CRC_H_
#define _DRV_SDIO_CRC_H_

/* The maximum bus data length calculated by CRC16 */
#define DRV_SDIO_CRC16_4BIT_BUS_DATA_MAX_LEN        512
#define DRV_SDIO_CRC16_8BIT_BUS_DATA_MAX_LEN        512

uint8_t sdio_crc7_calc (const uint8_t *ptr, uint32_t bit_num);

void sdio_crc16_calc_1bit_bus (uint8_t crc[2], uint8_t *ptr, uint16_t len);
void sdio_crc16_calc_4bit_bus (uint8_t crc[8], uint8_t *ptr, uint16_t len);

#endif /* _DRV_SDIO_CRC_H_ */
