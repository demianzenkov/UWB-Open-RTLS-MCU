#ifndef TSK_UDP_CLIENT
#define TSK_UDP_CLIENT
  
#include "cmsis_os.h"

#include "lwip.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/netbuf.h"

#include "net_conf.hpp"
#include "soc_proto.hpp"
  
class TskUdpClient 
{
public:
  TskUdpClient();
  ~TskUdpClient();
  void createTask();

public:
  static void udpEchoThread(void const *);
  static void udpTransmitThread(void const *);
  
public:
  typedef struct struct_conn_t {
    struct netconn *conn;
    struct netbuf *buf;
    struct ip4_addr *addr;
    unsigned short port;
  } struct_conn;
  
  struct_conn udp_recv_conn;
  struct_conn udp_send_conn;
  
  NetConfig net_conf;
  SocketProtocol soc_proto;
  
  SemaphoreHandle_t xSemLwipReady;
  SemaphoreHandle_t xSemConnReady;
  
  QueueHandle_t xQueueUdpRx;
  QueueHandle_t xQueueUdpTx;
  
  
  
  const U16 _rxQueueSize = 1;
  const U16 _txQueueSize = 1;
  
};


#endif //TSK_UDP_CLIENT