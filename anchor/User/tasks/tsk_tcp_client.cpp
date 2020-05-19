#include "tsk_tcp_client.h"
#include "tsk_une.h"
#include "settings.h"

TskTcpClient tskTcpClient;
extern TskUNE tskUNE;
extern DeviceSettings settings;


TskTcpClient::TskTcpClient() 
{
  
}

TskTcpClient::~TskTcpClient() 
{
}

void TskTcpClient::createTask()
{ 
  /* Create task semaphores */ 
  if (sem == NULL)
    BSP_OS::semCreate (&sem, 1, (char*)"sem_tcp");
  
  /* Create task semaphores */
//  xSemLwipReady = xSemaphoreCreateBinary();
  xSemConnReady = xSemaphoreCreateBinary();
  
  /* Create task queues */
  xQueueTcpRx = xQueueCreate(_rxQueueSize, sizeof(queue_data_t));
  xQueueTcpTx = xQueueCreate(_txQueueSize, sizeof(queue_data_t));
  /* Create receive task */
  osThreadId tcpRxTaskHandle;
  osThreadDef(TCPReceiveTask, tskTcpClient.tcpReceiveThread, osPriorityNormal, 0, 512);
  tcpRxTaskHandle = osThreadCreate(osThread(TCPReceiveTask), NULL);
  /* Create transmit task */
  osThreadId tcpTxTaskHandle;
  osThreadDef(TCPTransmitTask, tskTcpClient.tcpTransmitThread, osPriorityNormal, 0, 512);
  tcpTxTaskHandle = osThreadCreate(osThread(TCPTransmitTask), NULL);
}

void TskTcpClient::tcpReceiveThread(void const *arg)
{
  // wait till lwip is initiated (MX_LWIP_Init())
//  xSemaphoreTake(tskTcpClient.xSemLwipReady, portMAX_DELAY);
//  vSemaphoreDelete(tskTcpClient.xSemLwipReady);

  err_t err;
  LWIP_UNUSED_ARG(arg);

  tskTcpClient.tcp_recv_conn.conn = netconn_new(NETCONN_TCP);
  if (tskTcpClient.tcp_recv_conn.conn != NULL)
  {
    xSemaphoreGive(tskTcpClient.xSemConnReady);
    ip4_addr_t server_ip;
    U08 * srv_ip_p = settings.net_conf.getServerIp();
//    IP4_ADDR(&server_ip, srv_ip_p[0], srv_ip_p[1], srv_ip_p[2], srv_ip_p[3]);
    err = netconn_bind(tskTcpClient.tcp_recv_conn.conn, 
		       IP_ADDR_ANY, //&server_ip, 
		       settings.net_conf.getServerPort());
    
    if (err == ERR_OK)
    {
      for (;;) 
      {  
        err = netconn_recv(tskTcpClient.tcp_recv_conn.conn,  &tskTcpClient.tcp_recv_conn.buf);
        if (err == ERR_OK) 
        { 
          /* Echo start */ 
          queue_data_t rx_queue;
          memcpy(rx_queue.data, 
                 tskTcpClient.tcp_recv_conn.buf->p->payload,
                 tskTcpClient.tcp_recv_conn.buf->p->len);
          rx_queue.len = tskTcpClient.tcp_recv_conn.buf->p->len;
          xQueueSend( tskUNE.xQueueNetworkRx, (void *) &rx_queue, (TickType_t)0 );
          /* Echo end */
          
          netbuf_delete(tskTcpClient.tcp_recv_conn.buf);
        }
      }
    }
    else {
      netconn_delete(tskTcpClient.tcp_recv_conn.conn);
      printf("can not bind netconn");
      for(;;) {
	osDelay(1000);
      }
    }
  }
  else {
    printf("can create new TCP netconn");
    for(;;) {
	osDelay(1000);
      }
  }
}

void TskTcpClient::tcpTransmitThread(void const *arg)
{
  xSemaphoreTake(tskTcpClient.xSemConnReady, portMAX_DELAY);
  vSemaphoreDelete(tskTcpClient.xSemConnReady);
  
  tskTcpClient.tcp_send_conn.conn = netconn_new(NETCONN_TCP);
  
  tskTcpClient.tcp_send_conn.addr.addr = settings.net_conf.getServerIp32();
  
  tskTcpClient.tcp_send_conn.port = settings.net_conf.getServerPort();
  
  // connect to server 
  err_t err = netconn_connect(tskTcpClient.tcp_send_conn.conn, 
		  &tskTcpClient.tcp_send_conn.addr, 
		  tskTcpClient.tcp_send_conn.port);
  
  if (err == ERR_OK)
  {
    queue_data_t tx_queue;
    for(;;)
    {
      // wait for data put in tx queue
      if (!xQueueReceive(tskTcpClient.xQueueTcpTx, &tx_queue, portMAX_DELAY))
	continue;
      // allocate netbuf
      tskTcpClient.tcp_send_conn.buf = netbuf_new();
      void * buf_p = netbuf_alloc(tskTcpClient.tcp_send_conn.buf, tx_queue.len);
      // fill netbuf with data from queue
      memcpy (buf_p, &tx_queue, tx_queue.len);
      // fill netbuf with server ip and port
      tskTcpClient.tcp_send_conn.buf->addr.addr = \
    	settings.net_conf.ipArrToHex(settings.net_conf.getServerIp());
      tskTcpClient.tcp_send_conn.buf->port = settings.net_conf.getServerPort();
      // send data
      err = netconn_send(tskTcpClient.tcp_send_conn.conn, tskTcpClient.tcp_send_conn.buf);
      // clean netbuf
      netbuf_delete(tskTcpClient.tcp_send_conn.buf);
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

S08 TskTcpClient::lock (void)
{
  if (sem)
  {
    S08 sErr = BSP_OS::semWait (&sem, 10.0 * BSP_TICKS_PER_SEC);
    return sErr;
  }
  else
    return RC_ERR_COMMON;
}


S08 TskTcpClient::unlock (void)
{
  if (sem)
  {
    S08 sErr = BSP_OS::semPost (&sem);
    return sErr;
  }
  else
    return RC_ERR_COMMON;
}


void TskTcpClient::transmit(U08 * buf, U16 len)
{
  lock();
  tx_queue.len = len;
  memcpy(tx_queue.data, buf, len);
  xQueueSend(xQueueTcpTx, (void *)&tx_queue, (TickType_t)0);
  unlock();
}