#include "soc_proto.hpp"
#include "net_conf.hpp"
#include "crc.h"

SocketProtocol soc_proto;

SocketProtocol::SocketProtocol()
{
  proto_base.data.reserve(64);
}

SocketProtocol::~SocketProtocol()
{
  
}

err_te SocketProtocol::parseBuf(U08 * buf, U16 len)
{
  return ERR_NO;
}

err_te SocketProtocol::toProtoBase(std::vector<U08> packet, proto_base_t * proto_base)
{
  if (packet.size()==0 || packet.size()<5)
    return ERR_PROTO;
  
  if (packet[0] != SOC_RX_PREF)
    return ERR_PROTO;
  proto_base->pref = packet[0];
  
  if (packet[1] != SocketProtocol::proto_base.addr)
    return ERR_PROTO;
  
  proto_base->cmd = (soc_cmd_t)packet[2];
  
  proto_base->data.clear();
  proto_base->data.shrink_to_fit();
  proto_base->data.assign(packet.begin()+3, packet.end()-1);
  
  U08 comp_crc = CRC_08(0xFF, packet.data(), packet.size()-1);
  if (packet[packet.size()-1] != comp_crc)
    return ERR_CRC;
  
  proto_base->crc = packet[packet.size()-1];
  
  return ERR_NO;
}