#include "tsk_dwm.hpp"
#include "cmsis_os.h"

TskDWM tskDWM;

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
  if (tskDWM.dwm.init() != NO_ERR) {
    while(1) {
      HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
      osDelay(200);
      HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
      osDelay(200);
    }
  }
  
  for(;;)
  {
    if (tskDWM.une_twr.twrInitiatorLoop() == NO_ERR){
      HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
    }
    osDelay(1000);
    
  }
}