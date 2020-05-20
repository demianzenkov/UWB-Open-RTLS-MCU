#include "cmsis_os.h"
#include "tsk_dwm.h"
#include "tsk_usb.h"
#include "tsk_udp_client.h"
//#include "tsk_tcp_client.h"
#include "usbd_cdc_if.h"
#include "monitoring_pb.h"
#include "settings.h"

TskDWM tskDWM;
extern TskUdpClient tskUdpClient;
//extern TskTcpClient tskTcpClient;
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
  osThreadDef(DWMTask, tskDWM.task, osPriorityAboveNormal, 0, 512);
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
  
  if (settings.pb_settings.message.RTLSMode == 
      Settings_rtls_mode_MODE_TWR_INITIATOR) {
    UNE_TWR::initDWM();
  }
  
  
  for(;;)
  {
    Settings * msg = &settings.pb_settings.message;
    switch(settings.pb_settings.message.RTLSMode) {
    
    case Settings_rtls_mode_MODE_TWR_RESPONDER:
      if (tskDWM.une_twr.twrResponderLoop() == RC_ERR_NONE)
       HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
      break;
    
    case Settings_rtls_mode_MODE_TWR_INITIATOR:
      for(int i=0; i<msg->ConnectedAnchors_count;i++)
      {
	NVIC_DisableIRQ(OTG_FS_IRQn);
	if (tskDWM.une_twr.twrInitiatorLoop(msg->ConnectedAnchors[i]) == RC_ERR_NONE){
	  HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
	}
	NVIC_EnableIRQ(OTG_FS_IRQn);
	if (msg->PollDelay > 0)
	  osDelay(msg->PollDelay);
      }
      osDelay(msg->PollPeriod ? msg->PollPeriod : DEFAULT_POLL_PERIOD);
      break;
    default:
      break;
    }
    osDelay(1);
  }
}