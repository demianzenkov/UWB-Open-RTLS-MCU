#include "tsk_udp_client.h"
#include "tsk_une.h"
#include "settings.h"
#include "tsk_event.h"

TskUdpClient tskUdpClient;
extern TskUNE tskUNE;
extern TskEvent tskEvent;
extern DeviceSettings settings;


TskUdpClient::TskUdpClient() 
{
  
}

TskUdpClient::~TskUdpClient() 
{
}

void TskUdpClient::createTask()
{ 
  /* Create task semaphores */ 
  if (sem == NULL)
    BSP_OS::semCreate (&sem, 1, (char*)"sem_udp");
  
  /* Create task semaphores */
//  xSemLwipReady = xSemaphoreCreateBinary();
  xSemConnReady = xSemaphoreCreateBinary();
  
  /* Create task queues */
  xQueueUdpRx = xQueueCreate(_rxQueueSize, sizeof(queue_data_t));
  xQueueUdpTx = xQueueCreate(_txQueueSize, sizeof(queue_data_t));
  
/* Create receive task */
  osThreadId udpReceiveTaskHandle;
  osThreadDef(UDPReceiveTask, tskUdpClient.udpThread, osPriorityNormal, 0, 512);
  udpReceiveTaskHandle = osThreadCreate(osThread(UDPReceiveTask), NULL);
}


void TskUdpClient::udpThread(void const *arg) 
{ 
  err_t err;
  bool connected = false;
  
  tskUdpClient.srv_addr.addr = settings.net_conf.getServerIp32();
  tskUdpClient.port = settings.net_conf.getServerPort();
    
  
  tskUdpClient.conn = netconn_new_with_callback(NETCONN_UDP, tskUdpClient.udpReceiveCallback);
  
  if (tskUdpClient.conn != NULL)
  {
    if (netconn_bind(tskUdpClient.conn, 
		     NULL, 
		     tskUdpClient.port) == ERR_OK)
    {
      err = netconn_connect(tskUdpClient.conn, 
			    &tskUdpClient.srv_addr, 
			    tskUdpClient.port);
      if (err == ERR_OK)
      {
	osThreadId udpTransmitTaskHandle;
	osThreadDef(UDPTransmitTask, 
		    tskUdpClient.sendThread, 
		    osPriorityNormal, 
		    0, 
		    512);
	udpTransmitTaskHandle = osThreadCreate(osThread(UDPTransmitTask), NULL);
	connected = true;
      }
    }
  }
   
  if (connected != true) {
    tskEvent.setEvent(EV_CPU_RESET);
  }
  
  for(;;)
  {
    osDelay(1);
  }
  
}

void TskUdpClient::sendThread(void const *arg)
{
  struct netbuf *tx_buf;
  uint32_t syscnt = 0;
  
  for(;;)
    {
      if (!xQueueReceive(tskUdpClient.xQueueUdpTx, &tskUdpClient.tx_queue, pdMS_TO_TICKS(5000)))
      {
	tskUdpClient.unlock();
	continue;
      }
   
      tx_buf = netbuf_new();
      netbuf_alloc(tx_buf, tskUdpClient.tx_queue.len);
      pbuf_take(tx_buf->p, (void *) tskUdpClient.tx_queue.data, tskUdpClient.tx_queue.len);
      netconn_sendto(tskUdpClient.conn,tx_buf,&tskUdpClient.srv_addr,tskUdpClient.port);
      netbuf_delete(tx_buf);
      tskUdpClient.unlock();
    }
}


void TskUdpClient::udpReceiveCallback(struct netconn* conn, enum netconn_evt evt, u16_t len)
{
  unsigned short port;
  err_t recv_err;
  struct netbuf *rx_buf;
  if(evt==NETCONN_EVT_RCVPLUS)
  {
    recv_err = netconn_recv(conn, &rx_buf);
    if (recv_err == ERR_OK)
    {
      memcpy(tskUdpClient.rx_queue.data, 
	     rx_buf->p->payload,
	     rx_buf->p->len);
      tskUdpClient.rx_queue.len = rx_buf->p->len;
      
      xQueueSend(tskUNE.xQueueNetworkRx, (void *) &tskUdpClient.rx_queue, (TickType_t)0 );
      netbuf_delete(rx_buf);
    }
  }
}

S08 TskUdpClient::lock (void)
{
  if (sem)
  {
    S08 sErr = BSP_OS::semWait (&sem, 100.0 * BSP_TICKS_PER_SEC);
    return sErr;
  }
  else
    return RC_ERR_COMMON;
}


S08 TskUdpClient::unlock (void)
{
  if (sem)
  {
    S08 sErr = BSP_OS::semPost (&sem);
    return sErr;
  }
  else
    return RC_ERR_COMMON;
}


void TskUdpClient::transmit(U08 * buf, U16 len)
{
  lock();
  tx_queue.len = len;
  memcpy(tx_queue.data, buf, len);
  xQueueSend(xQueueUdpTx, (void *)&tx_queue, (TickType_t)0);
}

void TskUdpClient::sendHello()
{
  lock();
  tskUNE.wake.prepareBuf((U08 *)NULL, 0, CMD_I_AM_HERE_REQ, 
				   tx_queue.data, 
				   &tx_queue.len);
  xQueueSend(xQueueUdpTx, (void *)&tx_queue, (TickType_t)0);
}
