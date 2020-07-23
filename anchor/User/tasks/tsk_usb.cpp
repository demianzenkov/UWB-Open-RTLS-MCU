#include "tsk_usb.h"
#include "tsk_event.h"
#include "usbd_cdc_if.h"
#include "settings.h"
#include "main.h"

TskUSB tskUSB;

TskUSB::TskUSB() 
{
  wake.init();
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
  osThreadDef(USBTask, tskUSB.task, osPriorityNormal, 0, 512);
  USBTaskHandle = osThreadCreate(osThread(USBTask), NULL);
}

void TskUSB::task(void const *arg) {
  S08 sErr;
  
  for(;;) 
  {
    /* Wait data in xQueueUSBTx and put it to serial port */
    if (xQueueReceive(tskUSB.xQueueUSBTx, &tskUSB.tx_queue, 5)) {
      CDC_Transmit_FS(tskUSB.tx_queue.data, tskUSB.tx_queue.len);
    }
    
    /* Check if data received in xQueueUSBRx and process data */
    if (xQueueReceive(tskUSB.xQueueUSBRx, &tskUSB.rx_queue, 5)) {
      for (int i=0; i<tskUSB.rx_queue.len; i++) {
	S08 ret;
	ret = tskUSB.wake.rxHandler(tskUSB.rx_queue.data[i]); 
	if (ret == TRUE) {
	  tskUSB.tx_queue.len = 0;
	  bool reset = false;
	  
	  /* If found valid WAKE command */
	  switch(tskUSB.wake.wake.cmd) {
	  case CMD_GET_SETTINGS_REQ:
	    U16 msg_len;
	    U08 * settings_buf;
	    sErr = settings.getSettings(&settings_buf, &msg_len);
	    if (sErr != RC_ERR_NONE)
	      break;   
	    tskUSB.wake.prepareBuf(settings_buf, 
				   msg_len, 
				   CMD_GET_SETTINGS_RESP, 
				   tskUSB.tx_queue.data, 
				   &tskUSB.tx_queue.len);
	    
	    break;
	    
	  case CMD_SET_SETTINGS_REQ:
	    sErr = settings.setSettingsPb(tskUSB.wake.wake.dbuf, 
					  tskUSB.wake.wake.len);
	    if (sErr != RC_ERR_NONE)
	      break;
	    tskUSB.wake.prepareBuf(tskUSB.wake.wake.dbuf, 
				   tskUSB.wake.wake.len, 
				   CMD_SET_SETTINGS_RESP, 
				   tskUSB.tx_queue.data, 
				   &tskUSB.tx_queue.len);
	    break;
	  case CMD_SET_DEF_SETTINGS_REQ:
	    sErr = settings.setDefaultSettings();
	    if (sErr != RC_ERR_NONE)
	      break;
	    tskUSB.wake.prepareBuf(tskUSB.wake.wake.dbuf, 
				   tskUSB.wake.wake.len, 
				   CMD_SET_DEF_SETTINGS_RESP, 
				   tskUSB.tx_queue.data, 
				   &tskUSB.tx_queue.len);
	    tskEvent.setEvent(EV_CPU_RESET);
	    break;
	  case CMD_REBOOT_REQ:
	    tskUSB.wake.prepareBuf(tskUSB.wake.wake.dbuf, 
				   tskUSB.wake.wake.len, 
				   CMD_REBOOT_RESP, 
				   tskUSB.tx_queue.data, 
				   &tskUSB.tx_queue.len);
	    tskEvent.setEvent(EV_CPU_RESET);
	  default:
	    break;
	    
	  }
	  if (tskUSB.tx_queue.len) {
	    tskUSB.lock();
	    tskUSB.tx_queue.data[tskUSB.tx_queue.len++] = '\n';
	    xQueueSend(tskUSB.xQueueUSBTx, (void *)&tskUSB.tx_queue, (TickType_t)0);
	    tskUSB.unlock();
	  }
	}
      }
    }
  }
}

S08 TskUSB::lock (void)
{
  if (sem)
  {
    S08 sErr = BSP_OS::semWait (&sem, 10.0 * BSP_TICKS_PER_SEC);
    return sErr;
  }
  else
    return RC_ERR_COMMON;
}


S08 TskUSB::unlock (void)
{
  if (sem)
  {
    S08 sErr = BSP_OS::semPost (&sem);
    return sErr;
  }
  else
    return RC_ERR_COMMON;
}


void TskUSB::transmit(U08 * buf, U16 len)
{
  lock();
  tx_queue.len = len;
  memcpy(tx_queue.data, buf, len);
  xQueueSend(xQueueUSBTx, (void *)&tx_queue, (TickType_t)0);
  unlock();
}

void TskUSB::receiveFromISR(U08 * buf, U16 len)
{
  rx_queue.len = len;
  memcpy(rx_queue.data, buf, len);
  xQueueSendFromISR(xQueueUSBRx, (void *)&rx_queue, (TickType_t)0);
}