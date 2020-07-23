#include "une_tdoa.h"
#include "settings.h"


extern DeviceSettings settings;

UNE_TDOA::UNE_TDOA(DWM1000 * dwm)
{
  this->dwm = dwm;
}

/* Blink node routine */
S08 UNE_TDOA::tdoaBlinkNodeRoutine()
{
  sendBlinkWithDelay();
  return RC_ERR_NONE;
}


void UNE_TDOA::sendImmediate(U08 * packet, U08 size)
{
  dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
  
  dwt_writetxdata(size, packet, 0); /* Zero offset in TX buffer. */
  dwt_writetxfctrl(size, 0, 1); /* Zero offset in TX buffer, ranging. */
  
  /* Start transmission. */
  dwt_starttx(DWT_START_TX_IMMEDIATE);
  
  /* Poll DW1000 until TX frame sent event set. See NOTE 5 below.
  * STATUS register is 5 bytes long but, as the event we are looking at is in the first byte of the register, we can use this simplest API
  * function to access it.*/
  while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS))
  { };
  
  /* Clear TX frame sent event. */
  dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
}

S08 UNE_TDOA::sendBlinkWithDelay()
{
  if (!blink_started) 
  {
    sendImmediate(blink_msg, sizeof(blink_msg));
    blink_started = 1;
    return RC_ERR_NONE;
  }
 
  blink_msg[TDOA_MSG_BLINK_ID] = settings.pb_settings.message.NodeID;
  blink_msg[TDOA_MSG_BLINK_L] = (blink_n >> 8) & 0xFF;
  blink_msg[TDOA_MSG_BLINK_H] = blink_n & 0xFF;
  blink_n++;
    
  tx_time = dwt_readtxtimestamphi32();
  
  tx_time64 = tx_time;
  tx_time64 = tx_time64 << 8;
  
  tx_time64 += (U64)(((float)(settings.pb_settings.message.TdoaPollPeriod)/1000)/DWT_TIME_UNITS);
  tx_time64 &= 0xFFFFFFFE00UL;
  tx_time = tx_time64 >> 8;
  
  for (int i=0; i<4; i++) {
    blink_msg[TDOA_BLINK_TS+i] = (tx_time >> (i*8)) & 0xFF;
  }
  
  dwt_setdelayedtrxtime(tx_time);
  /* Zero offset in TX buffer. */
  dwt_writetxdata(sizeof(blink_msg), blink_msg, 0);
  /* Zero offset in TX buffer, ranging. */
  dwt_writetxfctrl(sizeof(blink_msg), 0, 1);	
  S08 ret = dwt_starttx(DWT_START_TX_DELAYED);
  if (ret == DWT_ERROR) {
    return RC_ERR_HW;
  }
  
  /* Poll DW1000 until TX frame sent event set. See NOTE 9 below. */
  while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS))
  { };
  /* Clear TXFRS event. */
  dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
  
  return RC_ERR_NONE;
}

