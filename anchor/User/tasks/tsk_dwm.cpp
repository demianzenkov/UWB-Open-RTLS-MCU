#include "cmsis_os.h"
#include "tsk_dwm.h"
#include "tsk_usb.h"
#include "tsk_udp_client.h"
//#include "tsk_tcp_client.h"
#include "usbd_cdc_if.h"
#include "monitoring_pb.h"
#include "settings.h"

TskDWM tskDWM;

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
//      HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
      osDelay(250);
//      HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
      osDelay(250);
    }
  }
  
  if ((settings.pb_settings.message.RTLSMode == Settings_rtls_mode_TWR_INITIATOR)
      || (settings.pb_settings.message.RTLSMode == Settings_rtls_mode_TDOA_ANCHOR))
  {
    UNE_TWR::initDWM();
  }
  
  
  for(;;)
  {
    Settings * cfg = &settings.pb_settings.message;
    switch(cfg->RTLSMode) {
    
    case Settings_rtls_mode_TWR_RESPONDER:
      if (tskDWM.une_twr.twrResponderLoop() == RC_ERR_NONE)
//       HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
      break;
    
    case Settings_rtls_mode_TWR_INITIATOR:
      for(int i=0; i<cfg->TwrConnectedAnchors_count;i++)
      {
	NVIC_DisableIRQ(OTG_FS_IRQn);
	if (tskDWM.une_twr.twrInitiatorLoop(cfg->TwrConnectedAnchors[i]) == RC_ERR_NONE){
//	  HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
	}
	NVIC_EnableIRQ(OTG_FS_IRQn);
	if (cfg->TwrPollDelay > 0)
	  osDelay(cfg->TwrPollDelay);
      }
      osDelay(cfg->TwrPollPeriod ? cfg->TwrPollPeriod : DEFAULT_POLL_PERIOD);
      break;
    
    case Settings_rtls_mode_TDOA_ANCHOR:
      if (tskDWM.une_tdoa.tdoaAnchorRoutine() == RC_ERR_NONE)
      {
//	HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
      }
      break;
    
    case Settings_rtls_mode_TDOA_SYNC:
      tskDWM.une_tdoa.tdoaSyncNodeRoutine();
      break;
      
    case Settings_rtls_mode_TDOA_TAG:
      tskDWM.une_tdoa.tdoaBlinkNodeRoutine();
      break;
      
    default:
      break;
    }
  }
}