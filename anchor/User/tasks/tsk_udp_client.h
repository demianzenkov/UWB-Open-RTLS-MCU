#ifndef __TSK_UDP_CLIENT
#define __TSK_UDP_CLIENT
  
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
  static void udpReceiveThread(void const *);
  static void udpTransmitThread(void const *);
  
  static void udpThread(void const *arg);
  static void udpReceiveCallback(struct netconn* conn, enum netconn_evt evt, u16_t len);
  static void sendThread(void const *);
  
  void transmit(U08 * buf, U16 len);
  void sendHello();
  
private:
  inline S08 lock();
  inline S08 unlock();
  
public:
  ip_addr_t srv_addr;
  unsigned short port;
  struct netconn *conn;
  
//  SemaphoreHandle_t xSemLwipReady;
  SemaphoreHandle_t xSemConnReady;
  
  QueueHandle_t xQueueUdpRx;
  QueueHandle_t xQueueUdpTx;
  
  queue_data_t rx_queue;
  queue_data_t tx_queue;
  
  const U16 _rxQueueSize = 3;
  const U16 _txQueueSize = 3;

  
private:
  OS_SEM sem;
};

extern TskUdpClient tskUdpClient;


#endif /* __TSK_UDP_CLIENT */