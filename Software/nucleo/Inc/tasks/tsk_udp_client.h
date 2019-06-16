#ifndef TSK_UDP_CLIENT
#define TSK_UDP_CLIENT

//#ifdef __cplusplus
//extern "C" {
//#endif
#include <FreeRTOS.h>
#include <task.h>
//#ifdef __cplusplus    
//}
//#endif

#include "cmsis_os.h"

#include "lwip.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/netbuf.h"

class TskUdpClient {

public:
  TskUdpClient();
  ~TskUdpClient();
  void createTask();

public:
  static void UdpClientTsk(void const *);
  static void UdpReceiveCallback(struct netconn* conn, enum netconn_evt evt, u16_t len);
  static void SendThread(void *arg);
  
public:
  typedef struct struct_conn_t {
    void * conn;
    void * buf;
  } struct_conn;
  struct_conn conn01;
};

#endif //TSK_UDP_CLIENT