#ifndef TSK_UDP_CLIENT
#define TSK_UDP_CLIENT
  
#include "cmsis_os.h"
#include "prj_defs.h"
#include "bsp_os.h"

#include "lwip.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/netbuf.h"
  
class TskUdpClient 
{
public:
  TskUdpClient();
  ~TskUdpClient();
  void createTask();

public:
  static void udpEchoThread(void const *);
  static void udpTransmitThread(void const *);
  
  void transmit(U08 * buf, U16 len);
  
private:
  inline S08 lock();
  inline S08 unlock();
  
public:
  typedef struct struct_conn_t {
    struct netconn *conn;
    struct netbuf *buf;
    struct ip4_addr *addr;
    unsigned short port;
  } struct_conn;
  
  struct_conn udp_recv_conn;
  struct_conn udp_send_conn;
  
  SemaphoreHandle_t xSemLwipReady;
  SemaphoreHandle_t xSemConnReady;
  
  QueueHandle_t xQueueUdpRx;
  QueueHandle_t xQueueUdpTx;
  
  queue_data_t rx_queue;
  queue_data_t tx_queue;
  
  const U16 _rxQueueSize = 1;
  const U16 _txQueueSize = 1;
  
private:
  OS_SEM sem;
};


#endif //TSK_UDP_CLIENT