#include "tsk_dwm.hpp"
#include "tsk_usb.hpp"
#include "tsk_udp_client.hpp"
#include "usbd_cdc_if.h"

TskDWM tskDWM;
extern TskUdpClient tskUdpClient;
extern TskUSB tskUSB;

TskDWM::TskDWM()
{
  
}

TskDWM::~TskDWM()
{
  
}

void TskDWM::createTask()
{
  xSemUSBReady = xSemaphoreCreateBinary();
  
  /* Create transmit task */
  osThreadId dwmTaskHandle;
  osThreadDef(DWMTask, tskDWM.task, osPriorityNormal, 0, 512);
  dwmTaskHandle = osThreadCreate(osThread(DWMTask), NULL);
}

void TskDWM::task(void const *arg)
{
  xSemaphoreTake(tskDWM.xSemUSBReady, portMAX_DELAY);
  vSemaphoreDelete(tskDWM.xSemUSBReady);
  
  if (tskDWM.dwm.init() != NO_ERR) {
    while(1) {
      HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
      osDelay(250);
      HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
      osDelay(250);
    }
  }
#ifdef SYNC_NODE  
  tskDWM.sync_period_ms = 1000;
#endif
  
  for(;;)
  {
#ifdef SYNC_NODE  
    /* Synchronisational node node routine */
    HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
    tskDWM.dwm.sendSyncPacket();
    osDelay(tskDWM.sync_period_ms);
#else
    /* TDOA anchor node routine */
    U08 packet_len = 0;
    U08 data_len=0;
    DWM1000::packet_type_te packet_type = tskDWM.dwm.receivePacket(&packet_len);
    if (packet_type == DWM1000::SYNC){
      tskDWM.dwm.sync_ts = tskDWM.dwm.getRxTimestampU64();
      data_len = 8;
      memcpy(&tskUSB.tx_queue.data[packet_len], &tskDWM.dwm.sync_ts, sizeof(tskDWM.dwm.sync_ts));
      HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
    }
    else if (packet_type == DWM1000::BLYNK){
      tskDWM.dwm.blynk_ts = tskDWM.dwm.getRxTimestampU64();
      data_len = 8;
      memcpy(&tskUSB.tx_queue.data[packet_len], &tskDWM.dwm.blynk_ts, sizeof(tskDWM.dwm.blynk_ts));
      HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
    }
    
    if(packet_type != DWM1000::NO_DATA)
    {
      memcpy(tskUSB.tx_queue.data, tskDWM.dwm.rx_buffer, packet_len);
      tskUSB.tx_queue.len = packet_len + data_len;
      xQueueSend(tskUSB.xQueueUSBTx, (void *)&tskUSB.tx_queue, (TickType_t)0);
      xQueueSend(tskUdpClient.xQueueUdpTx, (void *)&tskUSB.tx_queue, (TickType_t)0);
    }
#endif	// SYNC_NODE
    
    
  }
}