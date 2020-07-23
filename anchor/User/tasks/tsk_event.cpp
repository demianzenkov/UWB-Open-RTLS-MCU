#include "tsk_event.h"
#include "cmsis_os.h"
#include "timers.h"
#include "bsp_os.h"
#include "tsk_une.h"
#include "tsk_udp_client.h"
//#include "tsk_tcp_client.h"

TskEvent tskEvent;
extern TskUNE tskUNE;
extern TskUdpClient tskUdpClient;
//extern TskTcpClient tskTcpClient;

TskEvent::TskEvent()
{
}

void TskEvent::createTask()
{ 
  x_ev_group = xEventGroupCreate();
  
  
  timer_10s_handle = xTimerCreate("Timer10s", pdMS_TO_TICKS(10000), pdTRUE, (void *) 0, timer10sCallback);
  
  xTimerStart(timer_10s_handle, 0);
  
  osThreadId EventTaskHandle;
  osThreadDef(EventTask, task, osPriorityNormal, 0, 512);
  EventTaskHandle = osThreadCreate(osThread(EventTask), NULL);
}

void TskEvent::task(void const *arg) {
  S08 sErr;
  
  tskUdpClient.sendHello();
  
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
				 portMAX_DELAY );/* Wait a maximum of 100ms for either bit to be set. */
    
    if (uxBits & EV_CPU_RESET) 
    {
      BSP_OS::restartCPU(500);	// 500 ms - timeout before restart
    }
    if (uxBits & EV_TIMER_10S) 
    {
      if (tskEvent.hello_req >= HELLO_REQ_ATTEMPTS) {
	BSP_OS::restartCPU(500);	// 500 ms - timeout before restart
      }
      tskUdpClient.sendHello();
      tskEvent.hello_req++;
    }
    osDelay(1);
  }
}

void TskEvent::setEvent(EventBits_t event_mask)
{
  /* Set bit 0 and bit 4 in xEventGroup. */
   xEventGroupSetBits( x_ev_group, event_mask );
}

void TskEvent::timer10sCallback(TimerHandle_t xTimer)
{
  tskEvent.setEvent(EV_TIMER_10S);
}

void TskEvent::resetHelloReq()
{
  hello_req = 0;
}

