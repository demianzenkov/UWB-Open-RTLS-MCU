#include "tsk_udp_client.hpp"
#include "net_conf.h"
#include <vector>

TskUdpClient tskUdpClient;
extern NetConfig net_conf;

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
    
    err = netconn_bind(tskUdpClient.udp_recv_conn.conn, IP_ADDR_ANY, net_conf.getServerPort());
    if (err == ERR_OK)
    {
      for (;;) 
      {  
        err = netconn_recv(tskUdpClient.udp_recv_conn.conn,  &tskUdpClient.udp_recv_conn.buf);
        if (err == ERR_OK) 
        {
//          tskUdpClient.udp_recv_conn.addr = \
//            netbuf_fromaddr(tskUdpClient.udp_recv_conn.buf);
//          tskUdpClient.udp_recv_conn.port = \
//            netbuf_fromport(tskUdpClient.udp_recv_conn.buf);
//          
//          SocketProtocol::queue_data_t resp_queue;
//          
//          tskUdpClient.soc_proto.parseBuf((U08 *)tskUdpClient.udp_recv_conn.buf->p->payload, 
//                                          tskUdpClient.udp_recv_conn.buf->p->len,
//                                          &resp_queue);
//          if(resp_queue.len > 0)
//            xQueueSend( tskUdpClient.xQueueUdpTx, (void *) &resp_queue, (TickType_t)0 );
          
          /* Echo start */ 
          queue_data_t rx_queue;
          memcpy(rx_queue.data, 
                 tskUdpClient.udp_recv_conn.buf->p->payload,
                 tskUdpClient.udp_recv_conn.buf->p->len);
          rx_queue.len = tskUdpClient.udp_recv_conn.buf->p->len;
          xQueueSend( tskUdpClient.xQueueUdpTx, (void *) &rx_queue, (TickType_t)0 );
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
  
  tskUdpClient.udp_send_conn.addr->addr = net_conf.ipArrToHex(net_conf.getServerIp());
  
  tskUdpClient.udp_send_conn.port = net_conf.getServerPort();
  
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
    	net_conf.ipArrToHex(net_conf.getServerIp());
      tskUdpClient.udp_send_conn.buf->port = net_conf.getServerPort();
      // send data
      err = netconn_send(tskUdpClient.udp_send_conn.conn, tskUdpClient.udp_send_conn.buf);
      // clean netbuf
      netbuf_delete(tskUdpClient.udp_send_conn.buf);
    }
  }
  else {
    for (;;) {
      osDelay(500);
      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
      osDelay(500);
      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    }
  }
}
