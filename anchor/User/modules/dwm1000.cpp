#include "dwm1000.h"
#include "string.h"
#include "main.h"
#include "cmsis_os.h"

DWM1000::DWM1000() 
{
  TX_ANT_DLY = 16436;
  RX_ANT_DLY = 16436;
}

err_te DWM1000::init() 
{
#ifdef SYNC_NODE
  is_sync_node = 1;
#endif
  
  reset_DW1000();
  if (dwt_initialise(DWT_LOADUCODE) == DWT_ERROR)
  {
    return ERR_INIT;
  }
  resetConfig();
  configDWM(&config);
  
  dwt_setrxantennadelay(RX_ANT_DLY);
  dwt_settxantennadelay(TX_ANT_DLY);
  
  return NO_ERR;
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

err_te DWM1000::receiveEnable()
{
  /*	
  Activate reception immediately.
  Manual reception activation is performed here but DW1000 offers several features that can be used to handle more complex scenarios or to
  optimise system's overall performance (e.g. timeout after a given time, automatic re-enabling of reception in case of errors, etc.).
  */
  if (dwt_rxenable(DWT_START_RX_IMMEDIATE) != DWT_SUCCESS)
    return ERR_DWM;
  return NO_ERR;
}


/*! ------------------------------------------------------------------------------------------------------------------
* @fn getTxTimestampU64()
*
* @brief Get the TX time-stamp in a 64-bit variable.
*        /!\ This function assumes that length of time-stamps is 40 bits, for both TX and RX!
*
* @param  none
*
* @return  64-bit value of the read time-stamp.
*/
uint64 DWM1000::getTxTimestampU64(void)
{
  uint8 ts_tab[5];
  uint64 ts = 0;
  int i;
  dwt_readtxtimestamp(ts_tab);
  for (i = 4; i >= 0; i--)
  {
    ts <<= 8;
    ts |= ts_tab[i];
  }
  return ts;
}

/*! ------------------------------------------------------------------------------------------------------------------
* @fn getRxTimestampU64()
*
* @brief Get the RX time-stamp in a 64-bit variable.
*        /!\ This function assumes that length of time-stamps is 40 bits, for both TX and RX!
*
* @param  none
*
* @return  64-bit value of the read time-stamp.
*/
uint64 DWM1000::getRxTimestampU64(void)
{
  uint8 ts_tab[5];
  uint64 ts = 0;
  int i;
  dwt_readrxtimestamp(ts_tab);
  for (i = 4; i >= 0; i--)
  {
    ts <<= 8;
    ts |= ts_tab[i];
  }
  return ts;
}

void DWM1000::sendSyncPacket()
{
  dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
  /* Write frame data to DW1000 and prepare transmission. See NOTE 4 below.*/
  dwt_writetxdata(sizeof(sync_msg), sync_msg, 0); /* Zero offset in TX buffer. */
  dwt_writetxfctrl(sizeof(sync_msg), 0, 0); /* Zero offset in TX buffer, no ranging. */
  sync_msg[6]++;
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

err_te DWM1000::receivePacket(packet_type_te packet_type) 
{
  memset(rx_buffer, 0, FRAME_LEN_MAX);
  if(receiveEnable() != DWT_SUCCESS) {
    HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
    osDelay(250);
    HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
    osDelay(250);
    return ERR_DWM;
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
      switch(packet_type) {
      case SYNC:
	if(memcmp(rx_buffer, sync_msg, 4) == 0) {
	  sync_n = rx_buffer[6];
	  return NO_ERR;
	}
	else {
	  return ERR_PACKET;
	}
	break;
      case BLYNK:
	if(memcmp(rx_buffer, blynk_msg, 4) == 0) {
	  tag_id = rx_buffer[5];
	  blynk_n = rx_buffer[6];
	  return NO_ERR;
	}
	else {
	  return ERR_PACKET;
	}
	break;
      default:
	break;
      }
    }
    
    /* Clear good RX frame event in the DW1000 status register. */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG);
  }
  else
  {
    /* Clear RX error events in the DW1000 status register. */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
    return ERR_NODATA;
  }
  return ERR_NODATA;
}

void DWM1000::collectSocketBuf(uint8_t * out_buf)
{
  out_buf[0] = anchor_id;
  out_buf[1] = tag_id;
  out_buf[2] = sync_n;
  out_buf[3] = blynk_n;
  memcpy(&out_buf[4], &sync_ts, sizeof(sync_ts));
  memcpy(&out_buf[12], &blynk_ts, sizeof(blynk_ts));
}

void DWM1000::testReceive()
{
  memset(rx_buffer, 0, FRAME_LEN_MAX);
  if(receiveEnable() != DWT_SUCCESS) {
    HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
    osDelay(250);
    HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
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
      HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
      osDelay(250);
      HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
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

