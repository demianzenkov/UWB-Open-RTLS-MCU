#include "tsk_udp_client.h"
#include "tsk_une.h"
#include "settings.h"

TskUdpClient tskUdpClient;
extern TskUNE tskUNE;
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
  xSemLwipReady = xSemaphoreCreateBinary();
  xSemConnReady = xSemaphoreCreateBinary();
  
  /* Create task queues */
  xQueueUdpRx = xQueueCreate(_rxQueueSize, sizeof(queue_data_t));
  xQueueUdpTx = xQueueCreate(_txQueueSize, sizeof(queue_data_t));
  /* Create receive(echo) task */
  osThreadId udpEchoTaskHandle;
  osThreadDef(UDPEchoTask, tskUdpClient.udpEchoThread, osPriorityNormal, 0, 512);
  udpEchoTaskHandle = osThreadCreate(osThread(UDPEchoTask), NULL);
  /* Create transmit task */
  osThreadId udpTxTaskHandle;
  osThreadDef(UDPTransmitTask, tskUdpClient.udpTransmitThread, osPriorityNormal, 0, 512);
  udpTxTaskHandle = osThreadCreate(osThread(UDPTransmitTask), NULL);
}

void TskUdpClient::udpEchoThread(void const *arg)
{
  // wait till lwip is initiated (MX_LWIP_Init())
  xSemaphoreTake(tskUdpClient.xSemLwipReady, portMAX_DELAY);
  vSemaphoreDelete(tskUdpClient.xSemLwipReady);

  err_t err;
  LWIP_UNUSED_ARG(arg);

  tskUdpClient.udp_recv_conn.conn = netconn_new(NETCONN_UDP);
  if (tskUdpClient.udp_recv_conn.conn != NULL)
  {
    xSemaphoreGive(tskUdpClient.xSemConnReady);
    
    err = netconn_bind(tskUdpClient.udp_recv_conn.conn, IP_ADDR_ANY, settings.net_conf.getServerPort());
    if (err == ERR_OK)
    {
      for (;;) 
      {  
        err = netconn_recv(tskUdpClient.udp_recv_conn.conn,  &tskUdpClient.udp_recv_conn.buf);
        if (err == ERR_OK) 
        { 
          /* Echo start */ 
          queue_data_t rx_queue;
          memcpy(rx_queue.data, 
                 tskUdpClient.udp_recv_conn.buf->p->payload,
                 tskUdpClient.udp_recv_conn.buf->p->len);
          rx_queue.len = tskUdpClient.udp_recv_conn.buf->p->len;
          xQueueSend( tskUNE.xQueueNetworkRx, (void *) &rx_queue, (TickType_t)0 );
          /* Echo end */
          
          netbuf_delete(tskUdpClient.udp_recv_conn.buf);
        }
      }
    }
    else {
      printf("can not bind netconn");
    }
  }
  else {
    printf("can create new UDP netconn");
  }
}

void TskUdpClient::udpTransmitThread(void const *arg)
{
  xSemaphoreTake(tskUdpClient.xSemConnReady, portMAX_DELAY);
  vSemaphoreDelete(tskUdpClient.xSemConnReady);
  
  tskUdpClient.udp_send_conn.conn = netconn_new(NETCONN_UDP);
  
  tskUdpClient.udp_send_conn.addr->addr = settings.net_conf.ipArrToHex(settings.net_conf.getServerIp());
  
  tskUdpClient.udp_send_conn.port = settings.net_conf.getServerPort();
  
  // connect to server 
  err_t err = netconn_connect(tskUdpClient.udp_send_conn.conn, 
		  tskUdpClient.udp_send_conn.addr, 
		  tskUdpClient.udp_send_conn.port);
  
  if (err == ERR_OK)
  {
    queue_data_t tx_queue;
    for(;;)
    {
      // wait for data put in tx queue
      if (!xQueueReceive(tskUdpClient.xQueueUdpTx, &tx_queue, portMAX_DELAY))
	continue;
      // allocate netbuf
      tskUdpClient.udp_send_conn.buf = netbuf_new();
      void * buf_p = netbuf_alloc(tskUdpClient.udp_send_conn.buf, tx_queue.len);
      // fill netbuf with data from queue
      memcpy (buf_p, &tx_queue, tx_queue.len);
      // fill netbuf with server ip and port
      tskUdpClient.udp_send_conn.buf->addr.addr = \
    	settings.net_conf.ipArrToHex(settings.net_conf.getServerIp());
      tskUdpClient.udp_send_conn.buf->port = settings.net_conf.getServerPort();
      // send data
      err = netconn_send(tskUdpClient.udp_send_conn.conn, tskUdpClient.udp_send_conn.buf);
      // clean netbuf
      netbuf_delete(tskUdpClient.udp_send_conn.buf);
    }
  }
  else {
    for (;;) {
      osDelay(500);
      HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
      osDelay(500);
      HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
    }
  }
}

S08 TskUdpClient::lock (void)
{
  if (sem)
  {
    S08 sErr = BSP_OS::semWait (&sem, 10.0 * BSP_TICKS_PER_SEC);
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
  unlock();
}