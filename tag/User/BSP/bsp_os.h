#ifndef __BSP_OS
#define __BSP_OS

#include "prj_defs.h"
#include "cmsis_os.h"

#define	OS_SEM  		osSemaphoreId
#define	BSP_TICKS_PER_SEC 	(1000 / portTICK_PERIOD_MS )

class BSP_OS {

private:
  BSP_OS(){};
  
public:
  static S08  semCreate (OS_SEM* p_sem, U08 sem_val, char*  p_sem_name);
  static S08  semWait   (OS_SEM* p_sem, U32 dly_tck);
  static S08  semPost   (OS_SEM* p_sem);
  static S08  semClear  (OS_SEM* p_sem);
  static S08  semDel    (OS_SEM* p_sem);
  
  static U32  getTime (void);
};


#endif