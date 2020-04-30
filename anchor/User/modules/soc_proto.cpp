#include "soc_proto.h"
#include "net_conf.h"
#include "crc.h"

SocketProtocol soc_proto;

SocketProtocol::SocketProtocol()
{
  proto_base.data.reserve(64);
}

SocketProtocol::~SocketProtocol()
{
  
}

S08 SocketProtocol::parseBuf(U08 * buf, U16 len, queue_data_t * resp_queue)
{
  proto_base_t rx_proto_packet;
  std::vector<U08> rx_vect;
  rx_vect.assign(buf, buf + len);
  
  // check if packet is valid and make protocol structure from buf
  if (toProtoBase(rx_vect, &rx_proto_packet) != RC_ERR_NONE)
  {
    return RC_ERR_DATA;
  }
  
  // evaluate cmd and build response to server
  // ...
  
  
  return RC_ERR_NONE;
}

S08 SocketProtocol::toProtoBase(std::vector<U08> packet, proto_base_t * proto_base)
{
  if (packet.size()==0 || packet.size()<5)
    return RC_ERR_DATA;
  
  if (packet[0] != SOC_RX_PREF)
    return RC_ERR_DATA;
  proto_base->pref = packet[0];
  
  if (packet[1] != SocketProtocol::proto_base.addr)
    return RC_ERR_DATA;
  
  proto_base->cmd = (soc_cmd_t)packet[2];
  proto_base->data.clear();
  proto_base->data.shrink_to_fit();
  proto_base->data.assign(packet.begin()+3, packet.end()-1);
  
  U08 comp_crc = CRC_08(0xFF, packet.data(), packet.size()-1);
  if (packet[packet.size()-1] != comp_crc)
    return RC_ERR_CRC;
  
  proto_base->crc = packet[packet.size()-1];
  
  return RC_ERR_NONE;
}