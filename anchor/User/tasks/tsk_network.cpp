#include "tsk_network.h"
#include "tsk_event.h"
#include "tsk_udp_client.h"
#include "settings.h"

TskNetwork tskNetwork;
extern TskEvent tskEvent;
extern DeviceSettings settings;
extern TskUdpClient tskUdpClient;

TskNetwork::TskNetwork() 
{
  wake.init();
}

TskNetwork::~TskNetwork() 
{
}

void TskNetwork::createTask()
{ 
  /* Create task queues */
  xQueueNetworkRx = xQueueCreate(_rxQueueSize, sizeof(queue_data_t));
  
  osThreadId NetworkTaskHandle;
  osThreadDef(NetworkTask, tskNetwork.task, osPriorityNormal, 0, 512);
  NetworkTaskHandle = osThreadCreate(osThread(NetworkTask), NULL);
}

/**
  * @brief Process Network received data
  */
void TskNetwork::task(void const *arg) 
{
  S08 sErr;
  
  for(;;) 
  {
    /* Wait data from udp task */
    if (xQueueReceive(tskNetwork.xQueueNetworkRx, &tskNetwork.net_rx_queue, 10)) {
      for (int i=0; i<tskNetwork.net_rx_queue.len; i++) {
	S08 ret;
	ret = tskNetwork.wake.rxHandler(tskNetwork.net_rx_queue.data[i]); 
	if (ret == TRUE) {
	  tskNetwork.net_tx_queue.len = 0;
	  
	  /* If found valid WAKE command */
	  switch(tskNetwork.wake.wake.cmd) {
	  case CMD_I_AM_HERE_RESP:
	    tskEvent.resetHelloReq();
	    break;
	  case CMD_GET_SETTINGS_REQ:
	    U16 msg_len;
	    U08 * settings_buf;
	    sErr = settings.getSettings(&settings_buf, &msg_len);
	    if (sErr != RC_ERR_NONE)
	      break;   
	    tskNetwork.wake.prepareBuf(settings_buf, 
				   msg_len, 
				   CMD_GET_SETTINGS_RESP, 
				   tskNetwork.net_tx_queue.data, 
				   &tskNetwork.net_tx_queue.len);
	    
	    break;
	    
	  case CMD_SET_SETTINGS_REQ:
	    sErr = settings.setSettingsPb(tskNetwork.wake.wake.dbuf, 
					  tskNetwork.wake.wake.len);
	    if (sErr != RC_ERR_NONE)
	      break;
	    tskNetwork.wake.prepareBuf(tskNetwork.wake.wake.dbuf, 
				   tskNetwork.wake.wake.len, 
				   CMD_SET_SETTINGS_RESP, 
				   tskNetwork.net_tx_queue.data, 
				   &tskNetwork.net_tx_queue.len);
	    break;
	  case CMD_SET_DEF_SETTINGS_REQ:
	    sErr = settings.setDefaultSettings();
	    if (sErr != RC_ERR_NONE)
	      break;
	    tskNetwork.wake.prepareBuf(tskNetwork.wake.wake.dbuf, 
				   tskNetwork.wake.wake.len, 
				   CMD_SET_DEF_SETTINGS_RESP, 
				   tskNetwork.net_tx_queue.data, 
				   &tskNetwork.net_tx_queue.len);
	    tskEvent.setEvent(EV_CPU_RESET);
	    break;
	  case CMD_REBOOT_REQ:
	    tskNetwork.wake.prepareBuf(tskNetwork.wake.wake.dbuf, 
				   tskNetwork.wake.wake.len, 
				   CMD_REBOOT_RESP, 
				   tskNetwork.net_tx_queue.data, 
				   &tskNetwork.net_tx_queue.len);
	    tskEvent.setEvent(EV_CPU_RESET);
	  default:
	    break;
	    
	  }
	  if (tskNetwork.net_tx_queue.len) {
	    tskUdpClient.transmit(tskNetwork.net_tx_queue.data, tskNetwork.net_tx_queue.len);
	  }
	}
      }
    }
  }
}