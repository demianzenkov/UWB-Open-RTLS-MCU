#ifndef _NET_CONFIG
#define _NET_CONFIG

#include "custom_types.h"


extern "C"
{

class NetConfig {

public:
  NetConfig();
  ~NetConfig();
  
  typedef struct {
    U08 device_ip[4];
    U08 gw_ip[4];
    U08 server_ip[4];
    U16 server_port;
    U08 subnet_mask[4];
  } ip_conf_t;
  
public:
  ip_conf_t getIpSettings(void);
  void  setIpSettings(ip_conf_t ip_conf);
  U08 * getDeviceIp(void);
  void  setDeviceIp(U08 a, U08 b, U08 c, U08 d);
  U08 * getGatewayIp(void);
  void  setGatewayIp(U08 a, U08 b, U08 c, U08 d);
  U08 * getServerIp(void);
  void  setServerIp(U08 a, U08 b, U08 c, U08 d);
  U16   getServerPort(void);
  void  setServerPort(U16 port);
  U08 * getSubnetMask(void);
  void  setSubnetMask(U08 a, U08 b, U08 c, U08 d);
  
  U32 ipArrToHex(U08 * ip);
  
private:
  ip_conf_t ip_conf;
};

}
#endif