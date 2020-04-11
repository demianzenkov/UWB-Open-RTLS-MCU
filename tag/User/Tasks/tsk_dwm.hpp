#ifndef __TSK_DWM
#define __TSK_DWM

#include "dwm1000.h"

class TskDWM 
{
public:
  TskDWM();
  ~TskDWM();
  void createTask();
  static void task(void const *arg);
  
private:
  DWM1000 dwm;
};

#endif