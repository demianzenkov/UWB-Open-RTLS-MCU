#include "une_tdoa.h"


UNE_TDOA::UNE_TDOA(DWM1000 * dwm)
{
  this->dwm = dwm;
}

/* TDOA anchor node routine */
err_te UNE_TDOA::tdoaTagRoutine()
{
  dwm->blink();
  return NO_ERR;
}
