#ifndef _SOC_PROTO
#define _SOC_PROTO

#include <vector>
#include "custom_types.h"


#define SOC_RX_PREF 0x94
#define SOC_TX_PREF 0x9D

extern "C" {

class SocketProtocol {
  
public:
  SocketProtocol();
  ~SocketProtocol();


public:
  typedef enum {
    GET_NETCONF = 0x06,
    SET_NETCONF = 0x07,
    GET_UWBCONF = 0x08,
    SET_UWBCONF = 0x09,
    GET_ANCONF = 0x0A,
    SET_ANCONF = 0x0B,
    GET_TAGCONF = 0x0C,
    SET_TAGCONF = 0x0D,
    GET_DATA = 0x0E,
  } soc_cmd_t;
    
  typedef struct {
    U08 pref;
    const U08 addr=0xFF;
    soc_cmd_t cmd;
    std::vector<U08> data;
    U08 crc;
  } proto_base_t;
  
  proto_base_t proto_base;
  
  typedef struct {
    U08 data[64];
    U08 len;
  } queue_data_t;
  
  
  
public:
  err_te parseBuf(U08 * buf, U16 len, queue_data_t *);
  err_te toProtoBase(std::vector<U08> data, proto_base_t *);
  
};


  
}

#endif