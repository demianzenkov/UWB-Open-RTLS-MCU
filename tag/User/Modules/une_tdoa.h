#ifndef __UNE_TDOA
#define __UNE_TDOA

#include "custom_types.h"
#include "dwm1000.h"

class UNE_TDOA {
public:
  UNE_TDOA(DWM1000 * dwm);
  ~UNE_TDOA();
  
  err_te tdoaTagRoutine();
  
private:
  DWM1000 * dwm;
};

#endif