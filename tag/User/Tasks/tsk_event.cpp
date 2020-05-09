#include "tsk_event.h"
#include "bsp_os.h"

TskEvent tskEvent;

TskEvent::TskEvent()
{
}

void TskEvent::createTask()
{ 
  tskEvent.x_ev_group = xEventGroupCreate();
  
  osThreadId EventTaskHandle;
  osThreadDef(EventTask, tskEvent.task, osPriorityNormal, 0, 256);
  EventTaskHandle = osThreadCreate(osThread(EventTask), NULL);
}

void TskEvent::task(void const *arg) {
  S08 sErr;
  
  for(;;) 
  {
    EventBits_t uxBits;
    const TickType_t xTicksToWait = 100;
    /* Wait a maximum of 100ms  0 or bit 4 to be set within
    the event group.  Clear the bits before exiting. */
    uxBits = xEventGroupWaitBits(
				 tskEvent.x_ev_group,   /* The event group being tested. */
				 0x00FFFFFF, 	/* The bits within the event group to wait for. */
				 pdTRUE,        /* BITs should be cleared before returning. */
				 pdFALSE,       /* Don't wait for all bits, either bit will do. */
				 xTicksToWait );/* Wait a maximum of 100ms for either bit to be set. */
    
    if (uxBits & EV_CPU_RESET) 
    {
      BSP_OS::restartCPU(100);	// 100 ms - timeout before restart
    }
    
    osDelay(10);
  }
}

void TskEvent::setEvent(EventBits_t event_mask)
{
  /* Set bit 0 and bit 4 in xEventGroup. */
   xEventGroupSetBits( x_ev_group, event_mask );
}
