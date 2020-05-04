#include "tsk_une.h"
#include "tsk_udp_client.h"
#include "settings.h"

TskUNE tskUNE;
extern DeviceSettings settings;
extern TskUdpClient tskUdpClient;


TskUNE::TskUNE() 
{
  wake.init();
}

TskUNE::~TskUNE() 
{
}

void TskUNE::createTask()
{ 
  /* Create task queues */
  xQueueNetworkRx = xQueueCreate(_rxQueueSize, sizeof(queue_data_t));
  
  osThreadId UNETaskHandle;
  osThreadDef(UNETask, tskUNE.task, osPriorityNormal, 0, 256);
  UNETaskHandle = osThreadCreate(osThread(UNETask), NULL);
}

/**
  * @brief Process Network received data
  */
void TskUNE::task(void const *arg) 
{
  S08 sErr;
  
  for(;;) 
  {
    /* Wait data from udp task */
    if (xQueueReceive(tskUNE.xQueueNetworkRx, &tskUNE.net_rx_queue, 5)) {
      for (int i=0; i<tskUNE.net_rx_queue.len; i++) {
	S08 ret;
	ret = tskUNE.wake.rxHandler(tskUNE.net_rx_queue.data[i]); 
	if (ret == TRUE) {
	  tskUNE.net_tx_queue.len = 0;
	  
	  /* If found valid WAKE command */
	  switch(tskUNE.wake.wake.cmd) {
	  case CMD_GET_SETTINGS_REQ:
	    U16 msg_len;
	    U08 * settings_buf;
	    sErr = settings.getSettings(&settings_buf, &msg_len);
	    if (sErr != RC_ERR_NONE)
	      break;   
	    tskUNE.wake.prepareBuf(settings_buf, 
				   msg_len, 
				   CMD_GET_SETTINGS_RESP, 
				   tskUNE.net_tx_queue.data, 
				   &tskUNE.net_tx_queue.len);
	    
	    break;
	    
	  case CMD_SET_SETTINGS_REQ:
	    sErr = settings.setSettingsPb(tskUNE.wake.wake.dbuf, 
					  tskUNE.wake.wake.len);
	    if (sErr != RC_ERR_NONE)
	      break;
	    tskUNE.wake.prepareBuf(tskUNE.wake.wake.dbuf, 
				   tskUNE.wake.wake.len, 
				   CMD_SET_SETTINGS_RESP, 
				   tskUNE.net_tx_queue.data, 
				   &tskUNE.net_tx_queue.len);
	    break;
	  case CMD_SET_DEF_SETTINGS_REQ:
	    sErr = settings.setDefaultSettings();
	    if (sErr != RC_ERR_NONE)
	      break;
	    tskUNE.wake.prepareBuf(tskUNE.wake.wake.dbuf, 
				   tskUNE.wake.wake.len, 
				   CMD_SET_DEF_SETTINGS_RESP, 
				   tskUNE.net_tx_queue.data, 
				   &tskUNE.net_tx_queue.len);
	    break;
	  default:
	    break;
	    
	  }
	  if (tskUNE.net_tx_queue.len) {
	    tskUdpClient.transmit(tskUNE.net_tx_queue.data, tskUNE.net_tx_queue.len);
	  }
	}
      }
    }
  }
}