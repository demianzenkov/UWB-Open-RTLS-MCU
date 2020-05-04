#include "tsk_usb.hpp"
#include "usbd_cdc_if.h"
#include "settings.h"

TskUSB tskUSB;
extern DeviceSettings settings;

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
  osThreadDef(USBTask, tskUSB.task, osPriorityNormal, 0, 256);
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
	    break;
	  default:
	    break;
	    
	  }
	  if (tskUSB.tx_queue.len) {
	    tskUSB.tx_queue.data[tskUSB.tx_queue.len++] = '\n';
	    xQueueSend(tskUSB.xQueueUSBTx, (void *)&tskUSB.tx_queue, (TickType_t)0);
	  }
	}
      }
    }
  }
}