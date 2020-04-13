#include "tsk_dwm.hpp"
#include "tsk_udp_client.hpp"

TskDWM tskDWM;
extern TskUdpClient tskUdpClient;

TskDWM::TskDWM()
{
  
}

TskDWM::~TskDWM()
{
  
}

void TskDWM::createTask()
{
  /* Create transmit task */
  osThreadId dwmTaskHandle;
  osThreadDef(DWMTask, tskDWM.task, osPriorityNormal, 0, 512);
  dwmTaskHandle = osThreadCreate(osThread(DWMTask), NULL);
}

void TskDWM::task(void const *arg)
{
  if (tskDWM.dwm.init() != NO_ERR) {
    while(1) {
      HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
      osDelay(250);
      HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
      osDelay(250);
    }
  }
#ifdef SYNC_NODE  
  tskDWM.sync_period_ms = 100;
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
    switch(tskDWM.an_sm) {
    case WAIT_SYNC:
      if (tskDWM.dwm.receivePacket(SYNC) == NO_ERR){
	tskDWM.dwm.sync_ts = tskDWM.dwm.getRxTimestampU64();
	tskDWM.an_sm = WAIT_BLYNK;
	HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
      }
      break;
    case WAIT_BLYNK:
      if (tskDWM.dwm.receivePacket(BLYNK) == NO_ERR){
	tskDWM.dwm.blynk_ts = tskDWM.dwm.getRxTimestampU64();
	tskDWM.dwm.collectSocketBuf(tskDWM.udp_data_buf);
	xQueueSend( tskUdpClient.xQueueUdpTx, (void *) tskDWM.udp_data_buf, (TickType_t)0 );
	tskDWM.an_sm = WAIT_SYNC;
	HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
      }
      break;
    default:
      break;
    }
    osDelay(1);
#endif	// SYNC_NODE
    
    
  }
}