#include "wake.h"
#include "crc.h"
#include "string.h"

Wake::Wake()
{
  sem = NULL;
  if (BSP_OS::semCreate(&sem, 1, (char*)"wake_sem") != RC_ERR_NONE)
  {
    sem = NULL;
  }
}

S08 Wake::lock (void)
{
  if ((sem == 0) || (&sem == 0)) {
    return RC_ERR_PARAM;
  }
  
  if (osSemaphoreWait(sem, osWaitForever) == osOK )
    return RC_ERR_NONE;
  else 
    return RC_ERR_TIMEOUT;
  
}

S08 Wake::unlock (void)
{
  if ((sem == 0) ||  (&sem == 0)) {
    return RC_ERR_PARAM;
  }
  
  osSemaphoreRelease (sem);
  return RC_ERR_NONE;
}

S08 Wake::init()
{
  wake.addr = 0;	
  wake.rx_st = WAIT_FEND;
  wake.err_cnt = 0;
  
  return RC_ERR_NONE;
}

S08 Wake::rxHandler(U08 byte)
{
  lock();
  U08 pre_temp;

  if(byte == FEND)
  {
    wake.pre = byte;
    wake.crc = 0;
    wake.rx_st = WAIT_CMD;
    wake.crc = crc16(0xFFFF, byte);
    unlock();
    return RC_ERR_NONE;
  }

  if(wake.rx_st == WAIT_FEND) {
    unlock();
    return RC_ERR_NONE;
  }

  pre_temp = wake.pre;
  wake.pre = byte;

  /* byte-stufing */
  if(pre_temp == FESC)
  {
    if(byte == TFESC)
      byte = FESC;	
    else if(byte == TFEND)				
      byte = FEND;					
    else
    {
      wake.rx_st = WAIT_FEND;
      wake.err_cnt++;
      unlock();
      return RC_ERR_DATA;
    }
  }
  else
  {
    if(byte == FESC) { 
      unlock();
      return RC_ERR_NONE;
    }
  }

  switch(wake.rx_st)
  {
    /* Wait addr receive */
/*
  case WAIT_ADDR:
    if(byte & 0x80)						
    {
      byte = byte & 0x7F;				
      if(byte == 0 || byte == wake.addr)	
      {
	wake.crc = Wake_Do_crc8(wake.crc, byte);
	wake.sta = WAIT_CMD;		
	return RC_ERR_NONE;
      }
      wake.sta = WAIT_FEND;
      return RC_ERR_ADDR;
    }					
    wake.sta = WAIT_CMD;
*/
    
    /* Wait data receive */
  case WAIT_CMD:
    if(byte & 0x80)	// CMD byte should have zero MSB (wake protocol)
    {
      wake.rx_st = WAIT_FEND;
      wake.err_cnt++;
      return RC_ERR_DATA;
    }
    wake.cmd = byte;				
    wake.crc = crc16(wake.crc, byte);	
    wake.rx_st = WAIT_NBT1;				
    break;
    
  case WAIT_NBT1:
    wake.len = byte;
    wake.crc = crc16(wake.crc, byte);
    wake.ptr = 0;	
    wake.rx_st = WAIT_NBT2;
    break;
    
  case WAIT_NBT2:
    wake.len += (byte << 8);
    if(wake.len >= WAKE_FRAME)		
    {
      wake.rx_st = WAIT_FEND;
      wake.err_cnt++;
      return RC_ERR_DATA;			
    }
    
    wake.crc = crc16(wake.crc, byte);
    wake.ptr = 0;
    wake.rx_st = WAIT_DATA;
    break;
    
    
  case WAIT_DATA:
    if (wake.ptr < wake.len)			
    {
      wake.dbuf[wake.ptr++] = byte;
      wake.crc = crc16(wake.crc, byte);
      break;
    }
    wake.rx_st = WAIT_CRC1;
    
  case WAIT_CRC1:
    wake.crc_pack = byte;
    wake.rx_st = WAIT_CRC2;
    break;
    
  case WAIT_CRC2:
    wake.crc_pack += (byte << 8);
    if (wake.crc != wake.crc_pack)
    {
      wake.rx_st = WAIT_FEND;
      wake.err_cnt++;
      unlock();
      return RC_ERR_CRC;
    }
    wake.rx_st = WAIT_FIN;
    unlock();
    return TRUE;
  }
  unlock();
  return RC_ERR_NONE;
}

S08 Wake::prepareBuf(U08 * in_buf, U16 in_len, U08 cmd, 
		     U08 * out_buf, U16 * out_len)
{
  lock();
  if (in_buf != NULL && in_len != 0 && in_buf != wake.dbuf) {
    memcpy(wake.dbuf, in_buf, in_len);
  }
  wake.len = in_len;
  wake.cmd = cmd;
  
  U16 tmp;
  U16 _crc16;
  U16 cnt = 1;

  // start
  _crc16 = crc16 (0xFFFF, FEND);
  *out_buf++ = FEND;

  // addr
  /*if (!(wake.addr & 0x80))
  {
    crc16 = crc16 (crc16, wake.addr);
    tmp = (wake.addr | 0x80);
    cnt += _buf_add (&out_buf, tmp);
  }*/

  // cmd
  tmp = wake.cmd & 0x7F;
  _crc16 = crc16 (_crc16, tmp);
  cnt += _buf_add (&out_buf, tmp);

  // len
  tmp = wake.len;
  _crc16 = crc16(_crc16, (U08)tmp);
  cnt += _buf_add(&out_buf, (U08)tmp);
  _crc16 = crc16(_crc16, (U08)(tmp >> 8));
  cnt += _buf_add(&out_buf, (U08)(tmp >> 8));

  // data
  for (U16 i=0; i < wake.len; i++)
  {
    tmp = wake.dbuf[i];
    _crc16 = crc16(_crc16, tmp);
    cnt += _buf_add(&out_buf, tmp);
  }

  // crc16
  cnt += _buf_add (&out_buf, (U08)_crc16);
  cnt += _buf_add (&out_buf, (U08)(_crc16 >> 8));
  *out_len = cnt;
  
  unlock();

  return RC_ERR_NONE;
}

U08 Wake::_buf_add (U08** pp, U08 byte)
{
  U08 ret;
  U08* p_buf = *pp;

  if(byte == FEND)
  {
    *p_buf++ = FESC;
    *p_buf++ = TFEND;
    ret = 2;
  }
  else if(byte == FESC)
  {
    *p_buf++ = FESC;
    *p_buf++ = TFESC;
    ret = 2;
  }
  else
  {
    *p_buf++ = byte;
    ret = 1;
  }
  *pp = p_buf;
  return ret;
}


U16 Wake::crc16 (U16 init, U08 byte)
{
  U16 crc16;
  crc16 = CRC::CRC_16(init, &byte, 1);
  return crc16;
}
