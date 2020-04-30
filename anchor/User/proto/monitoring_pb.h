#ifndef __MONITORING_PB
#define __MONITORING_PB

#include "cmsis_os.h"
#include "prj_defs.h"
#include "virtual_pb.h"
#include "Monitoring.pb.h"

#define PB_TWR_MSGTYPE_NONE	0
#define PB_TWR_MSGTYPE_RANGING	1

class MonitoringPB : VirtualPB
{
public:
  MonitoringPB();
  
  S08 decode(U08 * buffer, U16 len, void * out_msg) {
    return decode_base(buffer, len, out_msg, Monitoring_fields);
  };
  
  S08 encode(void * in_msg, U08 * buffer, U16 * out_len) {
    return encode_base(in_msg, buffer, out_len, Monitoring_fields, Monitoring_size);
  };
  
  S08 init();
  void clearMessage();
  
public:
  Monitoring message = Monitoring_init_zero;
  U08 temp_buf[Monitoring_size];
};  

#endif /* __MONITORING_PB */
