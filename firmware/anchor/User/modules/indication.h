#ifndef _INDICATION
#define _INDICATION

#include "main.h"
#include "bsp_os.h"

#define DELAY_LEN   48
#define LED_COUNT   6
#define DMA_BUF_LEN (DELAY_LEN + LED_COUNT*24)

#define WS_LOW_ARR (48-1)
#define WS_HIGH_ARR (150-48-1)


class Indication {

public:
  Indication();
  
  void init();
  
  static void DMATransferComplete(DMA_HandleTypeDef * dma_handle);
  static void DMATransferError(DMA_HandleTypeDef * dma_handle);
  
  void light();
  void pixelToDMA(uint8_t Rpixel , uint8_t Gpixel, uint8_t Bpixel, uint16_t posX);
  
  U08 isTxComplete();
  void setTxComplete(U08 val);
  
private:
  U16 dma_buf[DMA_BUF_LEN] = {0};
  U08 tx_complete = 1;
};

extern Indication indication;

#endif