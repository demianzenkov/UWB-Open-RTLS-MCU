#include "dwm1000.h"
#include "une_twr.h"
#include "settings.h"

extern DeviceSettings settings;

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
  
  if(settings.pb_settings.message.DWRxAntDelay == 0) {
    settings.pb_settings.message.DWRxAntDelay = DEFAULT_RX_ANT_DLY;
  }
  if(settings.pb_settings.message.DWTxAntDelay == 0) {
    settings.pb_settings.message.DWTxAntDelay = DEFAULT_TX_ANT_DLY;
  }
  
  if(settings.pb_settings.message.DWRxAntDelay < 0) {
    dwt_setrxantennadelay(0);
  }
  else {
    dwt_setrxantennadelay(settings.pb_settings.message.DWRxAntDelay);
  }
  
  if(settings.pb_settings.message.DWTxAntDelay < 0) {
    dwt_settxantennadelay(0);
  }
  else {
    dwt_settxantennadelay(settings.pb_settings.message.DWTxAntDelay);
  }
    
  return RC_ERR_NONE;
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