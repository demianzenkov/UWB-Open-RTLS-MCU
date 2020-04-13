#include "tsk_dwm.hpp"
#include "cmsis_os.h"

TskDWM tskDWM;

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
      osDelay(200);
      HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
      osDelay(200);
    }
  }
//  tskDWM.dwm.receiveEnable();
  
  for(;;)
  {
    tskDWM.dwm.blink();
    osDelay(100);
    HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
  }
}