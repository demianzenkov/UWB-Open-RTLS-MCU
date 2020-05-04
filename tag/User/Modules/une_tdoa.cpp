#include "une_tdoa.h"


UNE_TDOA::UNE_TDOA(DWM1000 * dwm)
{
  this->dwm = dwm;
}

/* TDOA anchor node routine */
S08 UNE_TDOA::tdoaTagRoutine()
{
  dwm->blink();
  return RC_ERR_NONE;
}
