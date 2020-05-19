#ifndef __TSK_EVENT
#define __TSK_EVENT

#include "prj_defs.h"
#include "main.h"
#include "cmsis_os.h"

#define	EV_CPU_RESET	(1 << 0)
#define	EV_TIMER_10S	(1 << 1)

#define HELLO_REQ_ATTEMPTS	3

class TskEvent
{
public:
  TskEvent();
  ~TskEvent();
  void createTask();
  static void task(void const *arg);
  
  void setEvent(EventBits_t event_mask);
  void resetHelloReq();

private:
  static void timer10sCallback(TimerHandle_t xTimer);
  
private:
  EventGroupHandle_t x_ev_group;
  TimerHandle_t timer_10s_handle;
  
  U08 hello_req = 0;
};

#endif