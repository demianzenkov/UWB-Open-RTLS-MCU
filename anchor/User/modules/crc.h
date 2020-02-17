#include "stdint.h"

#define CRC8_POLY 0x8B

uint8_t CRC_08 (uint8_t init, uint8_t* pBuf, uint16_t len)
{
	uint8_t crc = init;
	
	while (len--)
	{
		crc = crc ^ *pBuf++;
		for (uint8_t i=0; i < 8; i++)
			crc = (crc & 0x80) ? ((crc << 1) ^ CRC8_POLY) : (crc << 1);
	}
	
	return crc;
}

#define CRC16_POLY 0x1021

uint16_t CRC_16 (uint16_t init, uint8_t* pBuf, uint16_t len)
{
	uint16_t crc = init;	
	while (len--)
	{
		crc ^= *pBuf++ << 8;
		for (uint8_t index = 0; index < 8; index++)
			crc = crc & 0x8000 ? (crc << 1) ^ CRC16_POLY : crc << 1;
	}
	return crc;
}
