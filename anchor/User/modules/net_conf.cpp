#include "net_conf.h"
#include <string.h>

NetConfig::NetConfig()
{
  setDefaultSettings();
}

NetConfig::~NetConfig()
{
  
}

void NetConfig::init(Settings * settings)
{
  setDeviceIp32(settings->DeviceIp);
  setGatewayIp32(settings->GatewayIp);
  setServerIp32(settings->ServerIp);
  setSubnetMask32(settings->SubnetMask);
  setServerPort(settings->ConnectionPort);
}

U32 NetConfig::ipArrToHex(U08 * ip)
{
  return ( (U32) ((ip[3]) & 0xff) << 24) | \
         ( (U32) ((ip[2]) & 0xff) << 16) | \
	 ( (U32) ((ip[1]) & 0xff) << 8)  | \
	 ( (U32) ((ip[0]) & 0xff) << 0);
}

void NetConfig::setDefaultSettings()
{
  memcpy(ip_conf.device_mac, def_config.device_mac, 4);
  memcpy(ip_conf.device_ip, def_config.device_ip, 4);
  memcpy(ip_conf.gw_ip, def_config.gw_ip, 4);
  memcpy(ip_conf.server_ip, def_config.server_ip, 4);
  memcpy(ip_conf.subnet_mask, def_config.subnet_mask, 4);
  ip_conf.server_port = def_config.server_port;
}


const U08 * NetConfig::getDefaultMac(void)
{
  return def_config.device_mac;
}

const U08 * NetConfig::getDefaultDeviceIp(void) 
{
  return def_config.device_ip;
}
const U08 * NetConfig::getDefaultSubnetMask(void) 
{
  return def_config.subnet_mask;
}
const U08 * NetConfig::getDefaultGatewayIp(void) 
{
  return def_config.gw_ip;
}
const U08 * NetConfig::getDefaultServerIp(void)
{
  return def_config.server_ip;
}
U16 NetConfig::getDefaultServerPort(void) 
{
  return def_config.server_port;
}


U32 NetConfig::getDefaultDeviceIp32(void) 
{
  return ipArrToHex((U08*)def_config.device_ip);
}
U32 NetConfig::getDefaultSubnetMask32(void) 
{
  return ipArrToHex((U08*)def_config.subnet_mask);
}
U32 NetConfig::getDefaultGatewayIp32(void) 
{
  return ipArrToHex((U08*)def_config.gw_ip);
}
U32 NetConfig::getDefaultServerIp32(void)
{
  return ipArrToHex((U08*)def_config.server_ip);
}


void NetConfig::setDeviceIp(U08 a, U08 b, U08 c, U08 d) 
{
  ip_conf.device_ip[0] = a;
  ip_conf.device_ip[1] = b;
  ip_conf.device_ip[2] = c;
  ip_conf.device_ip[3] = d;
}
void NetConfig::setGatewayIp(U08 a, U08 b, U08 c, U08 d) 
{
  ip_conf.gw_ip[0] = a;
  ip_conf.gw_ip[1] = b;
  ip_conf.gw_ip[2] = c;
  ip_conf.gw_ip[3] = d;
}
void NetConfig::setServerIp(U08 a, U08 b, U08 c, U08 d) 
{
  ip_conf.server_ip[0] = a;
  ip_conf.server_ip[1] = b;
  ip_conf.server_ip[2] = c;
  ip_conf.server_ip[3] = d;
}
void NetConfig::setSubnetMask(U08 a, U08 b, U08 c, U08 d) 
{
  ip_conf.subnet_mask[0] = a;
  ip_conf.subnet_mask[1] = b;
  ip_conf.subnet_mask[2] = c;
  ip_conf.subnet_mask[3] = d;
}
void NetConfig::setServerPort(U16 port) 
{
  ip_conf.server_port = port;
}


void NetConfig::setDeviceIp32(U32 ip) 
{
  setIp32(ip_conf.device_ip, ip);
}
void NetConfig::setGatewayIp32(U32 ip) 
{
  setIp32(ip_conf.gw_ip, ip);
}
void NetConfig::setServerIp32(U32 ip)
{
  setIp32(ip_conf.server_ip, ip);
}
void NetConfig::setSubnetMask32(U32 ip)
{
  setIp32(ip_conf.subnet_mask, ip);
}

void NetConfig::setIp32(U08 * buf, U32 ip)
{
  buf[0] = ip & 0xFF;
  buf[1] = (ip >> 8) & 0xFF;
  buf[2] = (ip >> 16) & 0xFF;
  buf[3] = (ip >>24) & 0xFF;
}


U08 * NetConfig::getDeviceMac(void)
{
  return ip_conf.device_mac;
}
U08 * NetConfig::getDeviceIp(void)
{
  return ip_conf.device_ip;
}
U08 * NetConfig::getGatewayIp(void)
{
  return ip_conf.gw_ip;
}
U08 * NetConfig::getSubnetMask(void)
{
  return ip_conf.subnet_mask;
}
U08 * NetConfig::getServerIp(void)
{
  return ip_conf.server_ip;
}
U16 NetConfig::getServerPort(void)
{
  return ip_conf.server_port;
}


U32 NetConfig::getDeviceIp32(void)
{
  return ipArrToHex(ip_conf.device_ip);
}
U32 NetConfig::getSubnetMask32(void)
{
  return ipArrToHex(ip_conf.subnet_mask);
}
U32 NetConfig::getGatewayIp32(void)
{
  return ipArrToHex(ip_conf.gw_ip);
}
U32 NetConfig::getServerIp32(void)
{
  return ipArrToHex(ip_conf.server_ip);
}






