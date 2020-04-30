#ifndef TSK_USB
#define TSK_USB
  
#include "cmsis_os.h"
#include "soc_proto.h"

class TskUSB 
{
public:
  TskUSB();
  ~TskUSB();
  void createTask();

public:
  static void task(void const *);
  
public:
  SemaphoreHandle_t xSemUSBReady;
  SemaphoreHandle_t xSemConnReady;
  
  QueueHandle_t xQueueUSBRx;
  QueueHandle_t xQueueUSBTx;
  
  const U16 _rxQueueSize = 1;
  const U16 _txQueueSize = 1;
  
  queue_data_t rx_queue;
  queue_data_t tx_queue;
};


#endif //TSK_USB