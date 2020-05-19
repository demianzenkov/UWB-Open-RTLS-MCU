#ifndef _NET_CONFIG
#define _NET_CONFIG

#include "prj_defs.h"
#include "Settings.pb.h"

class NetConfig {

public:
  NetConfig();
  ~NetConfig();
  
  typedef struct {
    U08 device_mac[6];
    U08 device_ip[4];
    U08 gw_ip[4];
    U08 server_ip[4];
    U16 server_port;
    U08 subnet_mask[4];
  } ip_conf_t;
  
  
public:
  void init(Settings * settings);
  U32 ipArrToHex(U08 * ip);
  void setDefaultSettings();
  
  ip_conf_t getIpSettings(void);
  void  setIpSettings(ip_conf_t ip_conf);
  
  void  setDeviceIp(U08 a, U08 b, U08 c, U08 d);
  void  setGatewayIp(U08 a, U08 b, U08 c, U08 d);
  void  setServerIp(U08 a, U08 b, U08 c, U08 d);
  void  setServerPort(U16 port);
  void  setSubnetMask(U08 a, U08 b, U08 c, U08 d);
  
  void setDeviceIp32(U32 ip);
  void setGatewayIp32(U32 ip);
  void setServerIp32(U32 ip);
  void setSubnetMask32(U32 ip);
  
  U08 * getDeviceMac(void);
  U08 * getDeviceIp(void);
  U08 * getSubnetMask(void);
  U08 * getGatewayIp(void);
  U08 * getServerIp(void);
  U16   getServerPort(void);
  
  U32 getDeviceIp32(void);
  U32 getSubnetMask32(void);
  U32 getGatewayIp32(void);
  U32 getServerIp32(void);
  
  const U08 * getDefaultMac(void);
  const U08 * getDefaultDeviceIp(void);
  const U08 * getDefaultSubnetMask(void);
  const U08 * getDefaultGatewayIp(void);
  const U08 * getDefaultServerIp(void);
  U16 getDefaultServerPort(void);
  
  U32 getDefaultDeviceIp32(void);
  U32 getDefaultSubnetMask32(void);
  U32 getDefaultGatewayIp32(void);
  U32 getDefaultServerIp32(void);
  
private:
  static void setIp32 (U08 * buf, U32 ip);
    
private:
  ip_conf_t ip_conf;
  
  ip_conf_t def_config = {
    .device_mac = {0x00,0x80,0xE1,0x12,0x45,0x79},
    .device_ip = {10, 90, 0, 2},
    .gw_ip = {10, 90, 90, 90},
    .server_ip = {10, 90, 90, 99},
    .server_port = 30005,
    .subnet_mask = {255, 0, 0, 0},
  };
  
};

#endif