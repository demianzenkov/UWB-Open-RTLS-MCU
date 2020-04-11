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
  
  if (tskDWM.dwm.receiveEnable() != DWT_SUCCESS) {
    while(1) {
      HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
      osDelay(250);
      HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
      osDelay(250);
    }
  }
  
  for(;;)
  {
    tskDWM.dwm.receive();
//    xQueueSend( tskUdpClient.xQueueUdpTx, (void *)tskDWM.dwm.rx_buffer, (TickType_t)0 );
    osDelay(1);
  }
}