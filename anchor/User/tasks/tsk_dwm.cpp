#include "tsk_dwm.h"
#include "tsk_usb.h"
#include "tsk_udp_client.h"
#include "usbd_cdc_if.h"
#include "monitoring_pb.h"
#include "settings.h"

TskDWM tskDWM;
extern TskUdpClient tskUdpClient;
extern TskUSB tskUSB;
extern MonitoringPB pb_monitoring;
extern DeviceSettings settings;

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
    switch(settings.pb_settings.message.RTLSMode) {
    case Settings_rtls_mode_MODE_TWR:
      if (tskDWM.une_twr.twrResponderLoop() == RC_ERR_NONE)
       HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
      break;
    default:
      break;
    }
    osDelay(1);
  }
}