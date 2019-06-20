#ifndef _SOC_PROTO
#define _SOC_PROTO

extern "C" {
 
class SocketProtocol {
public:
  SocketProtocol();
  ~SocketProtocol();
  
  void parseBuf();
};
  
  
}

#endif