#include "net_conf.hpp"
#include <string>


NetConfig::NetConfig()
{
  setDeviceIp(192, 168, 1, 225); 
  setGatewayIp(192, 168, 1, 1);
  setServerIp(192, 168, 1, 8);
  setServerPort(30001);
  setSubnetMask(255, 255, 255, 0);
}

NetConfig::~NetConfig()
{
  
}

U32 NetConfig::ipArrToHex(U08 * ip)
{
  return ( (U32) ((ip[3]) & 0xff) << 24) | \
         ( (U32) ((ip[2]) & 0xff) << 16) | \
	 ( (U32) ((ip[1]) & 0xff) << 8)  | \
	 ( (U32) ((ip[0]) & 0xff) << 0);
}
U08 * NetConfig::getDeviceIp(void)
{
  return ip_conf.device_ip;
}


void NetConfig::setDeviceIp(U08 a, U08 b, U08 c, U08 d) 
{
  ip_conf.device_ip[0] = a;
  ip_conf.device_ip[1] = b;
  ip_conf.device_ip[2] = c;
  ip_conf.device_ip[3] = d;
}


U08 * NetConfig::getGatewayIp(void)
{
  return ip_conf.gw_ip;
}
void NetConfig::setGatewayIp(U08 a, U08 b, U08 c, U08 d) 
{
  ip_conf.gw_ip[0] = a;
  ip_conf.gw_ip[1] = b;
  ip_conf.gw_ip[2] = c;
  ip_conf.gw_ip[3] = d;
}


U08 * NetConfig::getServerIp(void)
{
  return ip_conf.server_ip;
}
void NetConfig::setServerIp(U08 a, U08 b, U08 c, U08 d) 
{
  ip_conf.server_ip[0] = a;
  ip_conf.server_ip[1] = b;
  ip_conf.server_ip[2] = c;
  ip_conf.server_ip[3] = d;
}

U08 * NetConfig::getSubnetMask(void)
{
  return ip_conf.subnet_mask;
}
void NetConfig::setSubnetMask(U08 a, U08 b, U08 c, U08 d) 
{
  ip_conf.subnet_mask[0] = a;
  ip_conf.subnet_mask[1] = b;
  ip_conf.subnet_mask[2] = c;
  ip_conf.subnet_mask[3] = d;
}

U16 NetConfig::getServerPort(void)
{
  return ip_conf.server_port;
}

void NetConfig::setServerPort(U16 port) 
{
  ip_conf.server_port = port;
}
