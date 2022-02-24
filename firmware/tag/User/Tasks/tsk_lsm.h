#ifndef __TSK_LSM
#define __TSK_LSM

#include "lsm6.h"
#include "prj_defs.h"

class TskLSM 
{
public:
  TskLSM();
  void createTask();
  static void task(void const *arg);
    
private:
  LSM6 lsm6;
  
};

#endif