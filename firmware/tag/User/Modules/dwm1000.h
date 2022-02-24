#ifndef __DWM1000
#define __DWM1000

#include "prj_defs.h"

#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_spi.h"
#include "deca_port.h"

#define FRAME_LEN_MAX 127


#define DEFAULT_AREA_ID 0x01
#define DEFAULT_NODE_ID 0x01
#define START_BLYNK_N	0x00

class DWM1000 
{
public:
  DWM1000();
  ~DWM1000();
  
  typedef enum {
    TX_TS = 0,
    RX_TS
  } ts_type_te;
  
  
  S08 init();
  void resetConfig(dwt_config_t * config);
  void configDWM(dwt_config_t * config);
  void receiveEnable();
  void blink();
  
  static U64 getTimestampU64(ts_type_te ts_type);
  
private:
  dwt_config_t config;
  
public:
  U32 status_reg = 0;
  U16 frame_len = 0;
  U08 rx_buffer[FRAME_LEN_MAX];

};

#endif