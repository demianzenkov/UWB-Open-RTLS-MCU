#include "bsp_os.h"


S08 BSP_OS::semWait (OS_SEM* p_sem, U32 dly_tck)
{
  if ((p_sem == 0) || (*p_sem == 0))
  {
//    APP_PRINTF(APP_DBG_ON, "-I- BSP_OS_ERR: Sem Wait Not Define Error\r\n");
    return RC_ERR_PARAM;
  }
  
  if (xTaskGetSchedulerState() != taskSCHEDULER_RUNNING)
  {
    if ( uxSemaphoreGetCount(*p_sem) == 0 )
      return RC_ERR_RTOS;
  }
  
  if (dly_tck == 0)
    dly_tck = osWaitForever;
  if (osSemaphoreWait (*p_sem, dly_tck) == osOK )
    return RC_ERR_NONE;
  else 
    return RC_ERR_TIMEOUT;
}


S08 BSP_OS::semPost (OS_SEM* p_sem)
{
  if ((p_sem == 0) ||  (*p_sem == 0))
  {
    return RC_ERR_PARAM;
  }
  osSemaphoreRelease (*p_sem);
  return RC_ERR_NONE;
}

S08 BSP_OS::semCreate (OS_SEM * p_sem, U08 sem_val, char * p_sem_name)
{
  if (p_sem == NULL)
    return RC_ERR_PARAM;
  
  *p_sem = xSemaphoreCreateCounting (10, sem_val);
  if (*p_sem == 0)
    return RC_ERR_RTOS;
  
  return RC_ERR_NONE;
}


S08 BSP_OS::semDel (OS_SEM * p_sem)
{
  if (p_sem != NULL)
    vSemaphoreDelete(*p_sem);
  else
  {
    return RC_ERR_RTOS;
  }
  *p_sem = 0;
  return RC_ERR_NONE;
}
