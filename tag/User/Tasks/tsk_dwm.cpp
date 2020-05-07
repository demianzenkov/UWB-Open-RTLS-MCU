#include "tsk_dwm.hpp"
#include "cmsis_os.h"
#include "settings.h"
#include "usbd_cdc_if.h"

TskDWM tskDWM;
extern DeviceSettings settings;

TskDWM::TskDWM(): une_tdoa(&dwm), une_twr(&dwm)
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
  if (tskDWM.dwm.init() != RC_ERR_NONE) {
    while(1) {
      HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
      osDelay(200);
      HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
      osDelay(200);
    }
  }
  
  for(;;)
  {
    Settings * msg = &settings.pb_settings.message;
    switch(msg->RTLSMode) {
    case Settings_rtls_mode_MODE_TWR:
      for(int i=0; i<msg->ConnectedAnchors_count;i++)
      {
	NVIC_DisableIRQ(USB_IRQn);
	if (tskDWM.une_twr.twrInitiatorLoop(msg->ConnectedAnchors[i]) == RC_ERR_NONE){
	  HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
	}
	NVIC_EnableIRQ(USB_IRQn);
	if (msg->PollDelay > 0)
	  osDelay(msg->PollDelay);
      }
      osDelay(msg->PollPeriod ? msg->PollPeriod : DEFAULT_TWR_TAG_DELAY);
      break;
    default:
      break;
    }
    osDelay(1);  
  }
}