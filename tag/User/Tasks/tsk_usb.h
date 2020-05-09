#ifndef __TSK_USB
#define __TSK_USB


#include "cmsis_os.h"
#include "wake.h"

class TskUSB 
{
public:
  TskUSB();
  ~TskUSB();
  void createTask();

public:
  static void task(void const *);
  void transmit(U08 * buf, U16 len);
  void receiveFromISR(U08 * buf, U16 len);
  
private:
  inline S08 lock();
  inline S08 unlock();
  
public:
  SemaphoreHandle_t xSemUSBReady;
  SemaphoreHandle_t xSemConnReady;
  
  const U16 _rxQueueSize = 1;
  const U16 _txQueueSize = 1;
  
  Wake wake;
  
private:
  OS_SEM sem;
  
  QueueHandle_t xQueueUSBRx;
  QueueHandle_t xQueueUSBTx;
  
  queue_data_t rx_queue;
  queue_data_t tx_queue;
};


#endif /* TSK_USB */