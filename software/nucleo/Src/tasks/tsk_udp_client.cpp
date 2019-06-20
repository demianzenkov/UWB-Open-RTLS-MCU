#include "tsk_udp_client.hpp"

TskUdpClient tskUdpClient;

TskUdpClient::TskUdpClient() 
{
  
}

TskUdpClient::~TskUdpClient() 
{
  
}

void TskUdpClient::createTask()
{ 
  osThreadId udpClientTaskHandle;
  osThreadDef(UDPClientTask, (&tskUdpClient)->udpClientTsk, osPriorityNormal, 0, 2500);
  udpClientTaskHandle = osThreadCreate(osThread(UDPClientTask), NULL);
  
//  sys_thread_new("udp_thread1", udp_thread, NULL, DEFAULT_THREAD_STACKSIZE, osPriorityNormal );
}



void TskUdpClient::udpClientTsk(void const *pvParameters)
{
  MX_LWIP_Init();
  
  err_t err;
  struct netconn *conn;
  ip_addr_t DestIPaddr;
  conn = netconn_new_with_callback(NETCONN_UDP, (&tskUdpClient)->udpReceiveCallback);
  IP4_ADDR(&DestIPaddr, 192, 168, 1, 8);
  if (conn!= NULL)
  {
  	err = netconn_bind(conn, NULL, 30001);
    if (err == ERR_OK)
    {
      err = netconn_connect(conn, &DestIPaddr, 7);
      if (err == ERR_OK)
      {
        tskUdpClient.conn01.conn = conn;
        sys_thread_new("send_thread1", (&tskUdpClient)->sendThread, (void*)(&tskUdpClient.conn01.conn), DEFAULT_THREAD_STACKSIZE, osPriorityNormal );
      }
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

void TskUdpClient::udpReceiveCallback(struct netconn* conn, enum netconn_evt evt, u16_t len)
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
      syscnt = *(uint32_t*) buf->p->payload;
      netbuf_delete(buf);
      HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
    }
  }
}


void TskUdpClient::sendThread(void *arg)
{
  struct_conn *arg_conn;
  struct netconn *conn;
  struct netbuf *buf;
  uint32_t syscnt = 0;
  arg_conn = (struct_conn*) arg;
  conn = (netconn*)arg_conn->conn; // ! type-casting
  for(;;)
  {
    syscnt = osKernelSysTick();
    buf = netbuf_new();
    netbuf_alloc(buf, 4);
    pbuf_take(buf->p, (void *) &syscnt, 4);
    netconn_send(conn,buf);
    netbuf_delete(buf);
    HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
    osDelay(500);
  }
}
