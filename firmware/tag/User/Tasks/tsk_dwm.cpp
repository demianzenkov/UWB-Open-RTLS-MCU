#include "tsk_dwm.h"
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
  
 if (settings.pb_settings.message.RTLSMode == 
      Settings_rtls_mode_TWR_INITIATOR) {
    UNE_TWR::initDWM();
  }
  
  for(;;)
  {
    
    Settings * msg = &settings.pb_settings.message;
    switch(msg->RTLSMode) {
    case Settings_rtls_mode_TWR_INITIATOR:
      for(int i=0; i<msg->TwrConnectedAnchors_count;i++)
      {
	NVIC_DisableIRQ(USB_IRQn);
	if (tskDWM.une_twr.twrInitiatorLoop(msg->TwrConnectedAnchors[i]) == RC_ERR_NONE){
	  HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
	}
	NVIC_EnableIRQ(USB_IRQn);
	if (msg->TwrPollDelay > 0)
	  osDelay(msg->TwrPollDelay);
      }
      osDelay(msg->TwrPollPeriod ? msg->TwrPollPeriod : DEFAULT_POLL_PERIOD);
      break;
    case Settings_rtls_mode_TWR_RESPONDER:
      if (tskDWM.une_twr.twrResponderLoop() == RC_ERR_NONE)
	HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
      break;
      
    case Settings_rtls_mode_TDOA_TAG:
      tskDWM.une_tdoa.tdoaBlinkNodeRoutine();
      break;
    default:
      break;
    } 
  }
}