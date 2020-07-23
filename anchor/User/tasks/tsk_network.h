#ifndef __TSK_UNE
#define __TSK_UNE

#include "main.h"
#include "cmsis_os.h"
#include "wake.h"

class TskNetwork 
{
public:
  TskNetwork();
  ~TskNetwork();
  void createTask();
  static void task(void const *arg);
  
public:
  Wake wake;
  const U16 _rxQueueSize = 1;
  
  QueueHandle_t xQueueNetworkRx;
private:
  queue_data_t net_rx_queue;
  queue_data_t net_tx_queue;
};

extern TskNetwork tskNetwork;

#endif