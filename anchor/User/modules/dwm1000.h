#ifndef _DWM1000
#define _DWM1000

#include "prj_defs.h"

#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_spi.h"
#include "deca_port.h"

#define FRAME_LEN_MAX 127

#define DEFAULT_AREA_ID 0x01
#define DEFAULT_NODE_ID 0x01
#define START_SYNC_N	0x00
#define START_BLYNK_N	0x00

#define TX_ANT_DLY 	16436
#define RX_ANT_DLY 	16436

/* Preamble timeout, in multiple of PAC size. See NOTE 6. */
#define PRE_TIMEOUT 8

class DWM1000 
{
public:
  DWM1000();
  ~DWM1000();
  
  typedef enum {
    TX_TS = 0,
    RX_TS
  } ts_type_te;
  
  typedef enum {
    NO_DATA = 0,
    UNKNOWN, 
    SYNC,
    BLYNK,
  } packet_type_te;

  S08 init();
  void resetConfig();
  void configDWM(dwt_config_t * config);
  void testReceive();
  
  S08 receiveEnable();
  static U64 getTimestampU64(ts_type_te ts_type);
  void sendSyncPacket();
  packet_type_te receivePacket(uint8_t * data_len = (uint8_t *)NULL);
  U16 collectSocketBuf(uint8_t * out_buf);

private:
  void checkRW();
    
private:
  dwt_config_t config;

  /* Test frames, last 2 bytes - crc set by DWM */
  U08 sync_msg[9] = {'S', 'Y', 'N', 'C', 
			DEFAULT_AREA_ID, 
			DEFAULT_NODE_ID, 
			START_SYNC_N,
  			0x00, 0x00};
  U08 blynk_msg[9] = {'B', 'L', 'N', 'K', 
			DEFAULT_AREA_ID, 
			DEFAULT_NODE_ID, 
			START_BLYNK_N,
  			0x00, 0x00};
  
public:
  U32 status_reg = 0;
  U16 frame_len = 0;
  U08 rx_buffer[FRAME_LEN_MAX];
  U08 anchor_id = DEFAULT_NODE_ID;
  U08 tag_id = 0;
  U08 is_sync_node = 0;
  U64 sync_ts = 0;
  U64 blynk_ts = 0;
  U08 sync_n = 0;
  U08 blynk_n = 0;
  
};

#endif // _DWM1000