#ifndef __TSK_EVENT
#define __TSK_EVENT

#include "prj_defs.h"
#include "main.h"
#include "cmsis_os.h"

#define WAKE_CMD_BUF_SIZE	16

#define	EV_CPU_RESET	(1 << 0)

class TskEvent
{
public:
  TskEvent();
  ~TskEvent();
  void createTask();
  static void task(void const *arg);
  
  void setEvent(EventBits_t event_mask);

public:
  U32 ticks_10s;
private:
  EventGroupHandle_t x_ev_group;
  
  U08 wake_cmd_buf[WAKE_CMD_BUF_SIZE];
  U16 wake_cmd_len;
    
};

#endif