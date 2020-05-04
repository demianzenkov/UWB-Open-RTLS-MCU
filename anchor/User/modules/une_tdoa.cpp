#include "une_tdoa.h"
#include "tsk_usb.h"
#include "tsk_udp_client.h"


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
    HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
  }
  else if (packet_type == DWM1000::BLYNK){
    dwm->blynk_ts = dwm->getTimestampU64(DWM1000::RX_TS);
    data_len = 8;
    HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
  }
  
  if(packet_type != DWM1000::NO_DATA)
  {
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