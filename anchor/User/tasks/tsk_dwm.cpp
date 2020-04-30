#include "tsk_dwm.hpp"
#include "tsk_usb.hpp"
#include "tsk_udp_client.hpp"
#include "usbd_cdc_if.h"
#include "monitoring_pb.h"

TskDWM tskDWM;
extern TskUdpClient tskUdpClient;
extern TskUSB tskUSB;

extern MonitoringPB pb_monitoring;

TskDWM::TskDWM() : une_tdoa(&dwm), une_twr(&dwm)
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
  
  if (tskDWM.dwm.init() != RC_ERR_NONE) {
    while(1) {
      HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
      osDelay(250);
      HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
      osDelay(250);
    }
  }  
  for(;;)
  {
    tskDWM.une_twr.twrResponderLoop();
    osDelay(1);
#ifndef SYNC_NODE 
    
#endif	// SYNC_NODE
    
    
  }
}