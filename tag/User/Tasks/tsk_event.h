#ifndef __TSK_EVENT
#define __TSK_EVENT

#include "prj_defs.h"
#include "main.h"
#include "cmsis_os.h"

#define WAKE_CMD_BUF_SIZE	16

#define	EV_CPU_RESET	(1 << 0)
#define	EV_IMU_IRQ1	(1 << 1)
#define	EV_IMU_IRQ2	(1 << 2)

class TskEvent
{
public:
  TskEvent();
  ~TskEvent();
  void createTask();
  static void task(void const *arg);
  
  void setEvent(EventBits_t event_mask);

private:
  EventGroupHandle_t x_ev_group;
  
  U08 wake_cmd_buf[WAKE_CMD_BUF_SIZE];
  U16 wake_cmd_len;
    
};

#endif