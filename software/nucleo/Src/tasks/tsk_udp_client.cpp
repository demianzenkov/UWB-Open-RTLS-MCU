#include "tsk_udp_client.hpp"

#include <vector>

TskUdpClient tskUdpClient;

TskUdpClient::TskUdpClient() 
{
  
}

TskUdpClient::~TskUdpClient() 
{
  
}

void TskUdpClient::createTask()
{ 
  xQueueUdpRx = xQueueCreate(_rxQueueSize, sizeof(U08));
  xQueueUdpTx = xQueueCreate(_txQueueSize, sizeof(U08));
  
  osThreadId udpClientTaskHandle;
  osThreadDef(UDPClientTask, (&tskUdpClient)->udpClientTsk, osPriorityNormal, 0, 2500);
  udpClientTaskHandle = osThreadCreate(osThread(UDPClientTask), NULL);
  
  
}



void TskUdpClient::udpClientTsk(void const *pvParameters)
{
  MX_LWIP_Init();
  
  err_t err;
  struct netconn *conn;
  ip_addr_t DestIPaddr;
  conn = netconn_new_with_callback(NETCONN_UDP, (&tskUdpClient)->receiveCallback);
 
  IP4_ADDR(&DestIPaddr, tskUdpClient.net_conf.getServerIp()[0],
	   		tskUdpClient.net_conf.getServerIp()[1],
			tskUdpClient.net_conf.getServerIp()[2],
			tskUdpClient.net_conf.getServerIp()[3]);
  
  if (conn!= NULL)
  {
    err = netconn_bind(conn, NULL, tskUdpClient.net_conf.getServerPort());
    if (err == ERR_OK)
    {
//      err = netconn_connect(conn, &DestIPaddr, tskUdpClient.net_conf.getServerPort());
//      if (err == ERR_OK)
//      {
//        tskUdpClient.conn01.conn = conn;
//        sys_thread_new("send_thread1", (&tskUdpClient)->sendThread, (void*)(&tskUdpClient.conn01.conn), DEFAULT_THREAD_STACKSIZE, osPriorityNormal );
//      }
    }
    else
    {
      netconn_delete(conn);
    }
  }
  for(;;)
  {
    osDelay(1);
    
  }
}

void TskUdpClient::receiveCallback(struct netconn* conn, enum netconn_evt evt, u16_t len)
{
  uint32_t syscnt;
  unsigned short port;
  err_t recv_err;
  struct netbuf *buf;
  if(evt==NETCONN_EVT_RCVPLUS)
  {
    recv_err = netconn_recv(conn, &buf);
    if (recv_err == ERR_OK)
    {
      port = netbuf_fromport(buf);
      tskUdpClient.soc_proto.parseBuf((U08 *)buf->p->payload, buf->p->len);
      // echo
      xQueueSend(tskUdpClient.xQueueUdpTx, (void *) buf->p->payload, (TickType_t) 0);
      netbuf_delete(buf);
      HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
    }
  }
  else if (evt == NETCONN_EVT_ERROR)
  {
    __no_operation();
  }
}


void TskUdpClient::sendThread(void *arg)
{
  struct_conn *arg_conn;
  struct netconn *conn;
  struct netbuf *buf;
  arg_conn = (struct_conn*) arg;
  conn = (netconn*)arg_conn->conn; // ! type-casting
  std::vector<U08> tx_vector;
  U08 rx_data[64];
  for(;;)
  {
 
    if( xQueueReceive( tskUdpClient.xQueueUdpTx, &rx_data, (TickType_t) 10))
    {
      buf = netbuf_new();
      netbuf_alloc(buf, 4);
      pbuf_take(buf->p, (void *) &tx_vector, tx_vector.size());
      netconn_send(conn,buf);
      netbuf_delete(buf);
      HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
    }
    
    osDelay(1);
  }
}

