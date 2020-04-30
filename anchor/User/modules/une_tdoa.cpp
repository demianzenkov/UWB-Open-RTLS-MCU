#include "une_tdoa.h"
#include "tsk_usb.hpp"
#include "tsk_udp_client.hpp"


extern TskUdpClient tskUdpClient;
extern TskUSB tskUSB;


UNE_TDOA::UNE_TDOA(DWM1000 * dwm)
{
  this->dwm = dwm;
}

/* TDOA anchor node routine */
S08 UNE_TDOA::tdoaAnchorRoutine()
{
  U08 packet_len = 0;
  U08 data_len=0;
  DWM1000::packet_type_te packet_type = dwm->receivePacket(&packet_len);
  if (packet_type == DWM1000::SYNC){
    dwm->sync_ts = dwm->getTimestampU64(DWM1000::RX_TS);
    data_len = 8;
    memcpy(&tskUSB.tx_queue.data[packet_len], &dwm->sync_ts, sizeof(dwm->sync_ts));
    HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
  }
  else if (packet_type == DWM1000::BLYNK){
    dwm->blynk_ts = dwm->getTimestampU64(DWM1000::RX_TS);
    data_len = 8;
    memcpy(&tskUSB.tx_queue.data[packet_len], &dwm->blynk_ts, sizeof(dwm->blynk_ts));
    HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
  }
  
  if(packet_type != DWM1000::NO_DATA)
  {
    memcpy(tskUSB.tx_queue.data, dwm->rx_buffer, packet_len);
    tskUSB.tx_queue.len = packet_len + data_len;
    xQueueSend(tskUSB.xQueueUSBTx, (void *)&tskUSB.tx_queue, (TickType_t)0);
    xQueueSend(tskUdpClient.xQueueUdpTx, (void *)&tskUSB.tx_queue, (TickType_t)0);
  }
  return RC_ERR_NONE;
}

/* Synchronisational node node routine */
S08 UNE_TDOA::tdoaSyncNodeRoutine()
{
  HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
  dwm->sendSyncPacket();
  return RC_ERR_NONE;
}