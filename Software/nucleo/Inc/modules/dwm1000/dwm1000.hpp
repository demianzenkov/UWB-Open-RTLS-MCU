#ifndef DWM1000_H_
#define DWM1000_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "cmsis_os.h"
#include "deca_port.h"
#include "deca_device_api.h"


class DWM1000
{
public:
  DWM1000();
  ~DWM1000();
  
  void periphInit();
  void createTask();
  static void dwmTsk(void const *pvParameters);
  
public:
  SPI_HandleTypeDef hspi;
  dwt_config_t config;
  
};
  
  
#ifdef __cplusplus
}
#endif

#endif
