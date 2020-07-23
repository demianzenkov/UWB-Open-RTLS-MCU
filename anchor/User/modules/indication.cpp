#include "indication.h"
#include "tsk_event.h"

#define BITSET(reg, bit) ((reg & (1<<bit)) != 0)

Indication indication;

extern TIM_HandleTypeDef htim1;

Indication::Indication() {
  
}

void Indication::init() 
{
  HAL_GPIO_WritePin(LED_VDD_GPIO_Port, LED_VDD_Pin, GPIO_PIN_SET); 
  
  for(int i=DELAY_LEN; i<DMA_BUF_LEN; i++) {
    dma_buf[i] = WS_LOW_ARR;
  }
  light();
}

void Indication::light()
{
  if (tx_complete) {
    HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t*)indication.dma_buf, DMA_BUF_LEN);
    tx_complete = 0;
  }
}

void Indication::DMATransferComplete(DMA_HandleTypeDef *DmaHandle)
{
  HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_1);
  indication.tx_complete = 1;
}

void Indication::DMATransferError(DMA_HandleTypeDef *DmaHandle)
{
  indication.tx_complete = 1;
}

void Indication::pixelToDMA(uint8_t Rpixel , uint8_t Gpixel, uint8_t Bpixel, uint16_t posX)
{
  volatile uint16_t i;
  for(i=0;i<8;i++)
  {
    if (BITSET(Rpixel,(7-i)) == 1) {
      dma_buf[DELAY_LEN+posX*24+i+8] = WS_HIGH_ARR;
    }
    else {
      dma_buf[DELAY_LEN+posX*24+i+8] = WS_LOW_ARR;
    }
    
    if (BITSET(Gpixel,(7-i)) == 1) {
      dma_buf[DELAY_LEN+posX*24+i+0] = WS_HIGH_ARR;
    }
    else {
      dma_buf[DELAY_LEN+posX*24+i+0] = WS_LOW_ARR;
    }
    
    if (BITSET(Bpixel,(7-i)) == 1) {
      dma_buf[DELAY_LEN+posX*24+i+16] = WS_HIGH_ARR;
    }
    else {
      dma_buf[DELAY_LEN+posX*24+i+16] = WS_LOW_ARR;
    }
  }
  while(!tx_complete);
  light();
}