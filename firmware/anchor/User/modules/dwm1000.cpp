#include "dwm1000.h"

#include "main.h"
#include "cmsis_os.h"
#include "string.h"
#include "settings.h"


DWM1000::DWM1000() 
{
}

S08 DWM1000::init() 
{
#ifdef SYNC_NODE
  is_sync_node = 1;
#endif
  
  reset_DW1000();
  if (dwt_initialise(DWT_LOADUCODE) == DWT_ERROR)
  {
    return RC_ERR_HW;
  }
  resetConfig();
  configDWM(&config);
  
  
  if(settings.pb_settings.message.DWRxAntDelay == 0) {
    settings.pb_settings.message.DWRxAntDelay = DEFAULT_RX_ANT_DLY;
  }
  else if(settings.pb_settings.message.DWRxAntDelay < 0) {
    dwt_setrxantennadelay(0);
  }
  else {
    dwt_setrxantennadelay(settings.pb_settings.message.DWRxAntDelay);
  }
  
  if(settings.pb_settings.message.DWTxAntDelay == 0) {
    settings.pb_settings.message.DWTxAntDelay = DEFAULT_TX_ANT_DLY;
  }
  else if(settings.pb_settings.message.DWTxAntDelay < 0) {
    dwt_settxantennadelay(0);
  }
  else {
    dwt_settxantennadelay(settings.pb_settings.message.DWTxAntDelay);
  }
  
  
  dwt_setpreambledetecttimeout(PRE_TIMEOUT);
  
  return RC_ERR_NONE;
}

void DWM1000::configDWM(dwt_config_t * config)
{
  dwt_configure(config);
}

void DWM1000::resetConfig()
{
  config = {
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

S08 DWM1000::receiveEnable()
{
  /*	
  Activate reception immediately.
  Manual reception activation is performed here but DW1000 offers several features that can be used to handle more complex scenarios or to
  optimise system's overall performance (e.g. timeout after a given time, automatic re-enabling of reception in case of errors, etc.).
  */
  if (dwt_rxenable(DWT_START_RX_IMMEDIATE) != DWT_SUCCESS)
    return RC_ERR_TIMEOUT;
  return RC_ERR_NONE;
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
  
  return (ts & 0xFFFFFFFFFFUL);
}

uint64 DWM1000::getSysTimeU64()
{
  uint8 ts_tab[5];
  uint64 ts = 0;
  int i;
 
  dwt_readsystime(ts_tab);
  
  for (i = 4; i >= 0; i--)
  {
    ts <<= 8;
    ts |= ts_tab[i];
  }
  
  return (ts & 0xFFFFFFFFFFUL);
}


void DWM1000::testReceive()
{
  memset(rx_buffer, 0, FRAME_LEN_MAX);
  if(receiveEnable() != DWT_SUCCESS) {
//    HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
    osDelay(250);
//    HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
    osDelay(250);
    return;
  };
  
  /* Poll until a frame is properly received or an error/timeout occurs. See NOTE 4 below.
  * STATUS register is 5 bytes long but, as the event we are looking at is in the first byte of the register, we can use this simplest API
  * function to access it. */
  while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR)))
  { };
  
  if (status_reg & SYS_STATUS_RXFCG)
  {
    /* A frame has been received, copy it to our local buffer. */
    frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
    if (frame_len <= FRAME_LEN_MAX)
    {
      dwt_readrxdata(rx_buffer, frame_len, 0);
//      HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
      osDelay(250);
//      HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
      osDelay(250);
    }
    
    /* Clear good RX frame event in the DW1000 status register. */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG);
  }
  else
  {
    /* Clear RX error events in the DW1000 status register. */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
  }
}

void DWM1000::checkRW(){
  const uint8 dataA[20] = {19, 18, 17, 16, 15, 14, 13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 11};
  static uint8 dataB[20];
  dwt_writetodevice(0x21, 0, 20, &dataA[0]);
  dwt_readfromdevice(0x21, 0, 20, &dataB[0]);
}

