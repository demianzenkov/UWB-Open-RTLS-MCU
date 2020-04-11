#include "tsk_lsm.hpp"
#include "cmsis_os.h"

TskLSM tskLSM;

TskLSM::TskLSM()
{
  
}


void TskLSM::createTask()
{
  /* Create transmit task */
  osThreadId lsmTaskHandle;
  osThreadDef(LSMTask, tskLSM.task, osPriorityNormal, 0, 512);
  lsmTaskHandle = osThreadCreate(osThread(LSMTask), NULL);
}

void TskLSM::task(void const *arg)
{
  tskLSM.lsm6.init();
  
  for (int i=0; i<5; i++) {
    int8_t sErr = tskLSM.lsm6.checkWhoAmI();
    osDelay(10);
    if (sErr)
      while(1) {
      HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
      osDelay(330);
      HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
      osDelay(330);
    } 
  }
  
  tskLSM.lsm6.config();
  
  for(;;)
  {
    tskLSM.lsm6.poll();
    tskLSM.lsm6.updateAHRS();
    osDelay(100);
  }
}