#ifndef __CRC_DEF
#define __CRC_DEF

#include "stdint.h"
#include "prj_defs.h"

#define CRC8_POLY 0x8B
#define CRC16_POLY 0x1021

class CRC
{
public:
  static U08 CRC_08 (U08 init, U08* pBuf, U16 len)
  {
    U08 crc = init;
    
    while (len--)
    {
      crc = crc ^ *pBuf++;
      for (U08 i=0; i < 8; i++)
	crc = (crc & 0x80) ? ((crc << 1) ^ CRC8_POLY) : (crc << 1);
    }
    
    return crc;
  }
  
  static U16 CRC_16 (U16 init, U08* pBuf, U16 len)
  {
    U16 crc = init;	
    while (len--)
    {
      crc ^= *pBuf++ << 8;
      for (U08 index = 0; index < 8; index++)
	crc = crc & 0x8000 ? (crc << 1) ^ CRC16_POLY : crc << 1;
    }
    return crc;
  }
};

#endif /* __CRC_DEF */