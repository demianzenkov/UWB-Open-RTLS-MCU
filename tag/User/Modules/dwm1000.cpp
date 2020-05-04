#include "dwm1000.h"
#include "une_twr.h"



DWM1000::DWM1000() 
{
  
}

S08 DWM1000::init() 
{
  reset_DW1000();
  port_set_dw1000_slowrate();
  if (dwt_initialise(DWT_LOADUCODE) == DWT_ERROR)
  {
    return RC_ERR_HW;
  }
  port_set_dw1000_fastrate();
  
  resetConfig(&config);
  configDWM(&config);
  
  dwt_setrxantennadelay(RX_ANT_DLY);
  dwt_settxantennadelay(TX_ANT_DLY);
  
  UNE_TWR::initDWM();
    
  return RC_ERR_NONE;
}

void DWM1000::blink()
{
  dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
  /* Write frame data to DW1000 and prepare transmission. See NOTE 4 below.*/
  dwt_writetxdata(sizeof(tx_msg), tx_msg, 0); /* Zero offset in TX buffer. */
  dwt_writetxfctrl(sizeof(tx_msg), 0, 1); /* Zero offset in TX buffer, ranging. */
  
  /* Start transmission. */
  dwt_starttx(DWT_START_TX_IMMEDIATE);
  
  /* Poll DW1000 until TX frame sent event set. See NOTE 5 below.
  * STATUS register is 5 bytes long but, as the event we are looking at is in the first byte of the register, we can use this simplest API
  * function to access it.*/
  while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS))
  { };
  
  /* Clear TX frame sent event. */
  dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
  
  /* Increment the blink frame sequence number (modulo 256). */
  tx_msg[BLINK_FRAME_SN_IDX]++;
}

void DWM1000::configDWM(dwt_config_t * config)
{
  dwt_configure(config);
}

void DWM1000::resetConfig(dwt_config_t * config)
{
  *config = {
    2,               /* Channel number. */
    DWT_PRF_64M,     /* Pulse repetition frequency. */
    DWT_PLEN_1024,   /* Preamble length. Used in TX only. */
    DWT_PAC32,       /* Preamble acquisition chunk size. Used in RX only. */
    9,               /* TX preamble code. Used in TX only. */
    9,               /* RX preamble code. Used in RX only. */
    1,               /* 0 to use standard SFD, 1 to use non-standard SFD. */
    DWT_BR_110K,     /* Data rate. */
    DWT_PHRMODE_STD, /* PHY header mode. */
    (1025 + 64 - 32) /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
  };
};

void DWM1000::receiveEnable()
{
  dwt_rxenable(DWT_START_RX_IMMEDIATE);
}

/*! ------------------------------------------------------------------------------------------------------------------
* @fn getTimestampU64()
*
* @brief Get the RX and TX time-stamp in a 64-bit variable.
*        /!\ This function assumes that length of time-stamps is 40 bits, for both TX and RX!
*
* @param  none
*
* @return  64-bit value of the read time-stamp.
*/
uint64 DWM1000::getTimestampU64(ts_type_te ts_type)
{
  uint8 ts_tab[5];
  uint64 ts = 0;
  int i;
  if (ts_type == RX_TS) {
    dwt_readrxtimestamp(ts_tab);
  }
  else if (ts_type == TX_TS) {
    dwt_readtxtimestamp(ts_tab);
  }
  else 
    return 0;
  for (i = 4; i >= 0; i--)
  {
    ts <<= 8;
    ts |= ts_tab[i];
  }
  return ts;
}