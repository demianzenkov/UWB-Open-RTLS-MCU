#include "tsk_usb.hpp"
#include "usbd_cdc_if.h"

TskUSB tskUSB;

TskUSB::TskUSB() 
{
  
}

TskUSB::~TskUSB() 
{
}

void TskUSB::createTask()
{ 
  /* Create task semaphores */
  xSemUSBReady = xSemaphoreCreateBinary();
  
  /* Create task queues */
  xQueueUSBRx = xQueueCreate(_rxQueueSize, sizeof(queue_data_t));
  xQueueUSBTx = xQueueCreate(_txQueueSize, sizeof(queue_data_t));
  
  osThreadId USBTaskHandle;
  osThreadDef(USBTask, tskUSB.task, osPriorityNormal, 0, 256);
  USBTaskHandle = osThreadCreate(osThread(USBTask), NULL);
}

void TskUSB::task(void const *arg) {
  for(;;) 
  {
    /* Wait data in xQueueUSBTx and put it to serial port */
    if (xQueueReceive(tskUSB.xQueueUSBTx, &tskUSB.tx_queue, 5)) {
      CDC_Transmit_FS(tskUSB.tx_queue.data, tskUSB.tx_queue.len);
    }
  }
}