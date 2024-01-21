#ifndef _DRV_SDIO_CRC_H_
#define _DRV_SDIO_CRC_H_

/* The maximum bus data length calculated by CRC16 */
#define DRV_SDIO_CRC16_4BIT_BUS_DATA_MAX_LEN        512

uint8_t sdio_crc7_calc (const uint8_t *ptr, uint32_t bit_num);
uint16_t sdio_crc16_calc (const uint8_t *ptr, uint32_t bit_num);
void sdio_crc16_calc_4bit_bus (uint8_t *ptr, uint8_t *crc, uint16_t len);

#endif /* _DRV_SDIO_CRC_H_ */
