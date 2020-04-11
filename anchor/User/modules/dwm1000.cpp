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
  reset_DW1000();
  if (dwt_initialise(DWT_LOADUCODE) == DWT_ERROR)
  {
    return ERR_INIT;
  }
  resetConfig();
  configDWM(&config);
  
//  const uint8 dataA[20] = {19, 18, 17, 16, 15, 14, 13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 11};
//  static uint8 dataB[20];
//  dwt_writetodevice(0x21, 0, 20, &dataA[0]);
//  dwt_readfromdevice(0x21, 0, 20, &dataB[0]);
  
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

int8_t DWM1000::receiveEnable()
{
  /*	
  Activate reception immediately.
  Manual reception activation is performed here but DW1000 offers several features that can be used to handle more complex scenarios or to
  optimise system's overall performance (e.g. timeout after a given time, automatic re-enabling of reception in case of errors, etc.).
  */
  if (dwt_rxenable(DWT_START_RX_IMMEDIATE) != DWT_SUCCESS)
    return DWT_ERROR;
  return DWT_SUCCESS;
}

void DWM1000::receive()
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

