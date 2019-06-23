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
  static void udpClientTsk(void const *);
  static void receiveCallback(struct netconn* conn, enum netconn_evt evt, u16_t len);
  static void sendThread(void *arg);
  
public:
  typedef struct struct_conn_t {
    void * conn;
    void * buf;
  } struct_conn;
  struct_conn conn01;
  
  NetConfig net_conf;
  SocketProtocol soc_proto;
  
  QueueHandle_t xQueueUdpRx;
  QueueHandle_t xQueueUdpTx;
  const U16 _rxQueueSize = 256;
  const U16 _txQueueSize = 256;
};


#endif //TSK_UDP_CLIENT