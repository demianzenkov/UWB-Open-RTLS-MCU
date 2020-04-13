#ifndef __TSK_DWM
#define __TSK_DWM

#include "main.h"
#include "cmsis_os.h"
#include "dwm1000.h"

typedef enum {
  WAIT_SYNC = 0,
  WAIT_BLYNK,
}tdoa_an_sm_te;	// TDOA Anchor state machine enum

class TskDWM 
{
public:
  TskDWM();
  ~TskDWM();
  void createTask();
  static void task(void const *arg);
  
private:
  DWM1000 dwm;
  
#ifdef SYNC_NODE 
  uint32_t sync_period_ms;
#else
  tdoa_an_sm_te an_sm = WAIT_SYNC;
#endif
  
  uint8_t udp_data_buf[21];
  
};

#endif