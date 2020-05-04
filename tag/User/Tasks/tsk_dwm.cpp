#include "tsk_dwm.hpp"
#include "cmsis_os.h"
#include "settings.h"

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
    switch(settings.pb_settings.message.RTLSMode) {
    case Settings_rtls_mode_MODE_TWR:
      if (tskDWM.une_twr.twrInitiatorLoop() == RC_ERR_NONE){
	HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
      }
      osDelay(1000);
      break;
    default:
      break;
    }
    osDelay(1);  
  }
}