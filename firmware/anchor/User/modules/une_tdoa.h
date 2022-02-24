#ifndef _UNE_TDOA
#define _UNE_TDOA

#include "prj_defs.h"
#include "dwm1000.h"

#define TX_SYNC_TS 0x00

#define TDOA_MSG_SYNC_ID	4
#define TDOA_MSG_SYNC_L		5
#define TDOA_MSG_SYNC_H		6
#define TDOA_SYNC_TS		7

#define TDOA_MSG_BLINK_ID	4
#define TDOA_MSG_BLINK_L	5
#define TDOA_MSG_BLINK_H	6
#define TDOA_BLINK_TS		7

#define SYNC_TX_DELAY 		3000

class UNE_TDOA {

public:
  UNE_TDOA(DWM1000 * dwm);
  ~UNE_TDOA();
  
  typedef enum {
    NO_DATA = 0,
    UNKNOWN,
    SYNC,
    BLINK,
  } packet_type_te;
  
  S08 tdoaAnchorRoutine();
  S08 tdoaSyncNodeRoutine();
  S08 tdoaBlinkNodeRoutine();
  
  void sendImmediate(U08 * packet, U08 size);
  S08 sendSyncWithDelay();
  S08 sendBlinkWithDelay();
  packet_type_te receivePacket(uint8_t * data_len = (uint8_t *)NULL);
  
private:
  DWM1000 * dwm;
  
  /* Test frames, last 2 bytes - crc set by DWM */
  U08 sync_msg[14] = {'S', 'Y', 'N', 'C', 
			DEFAULT_NODE_ID, 
			START_SYNC_N,
			START_SYNC_N,
			TX_SYNC_TS,
			TX_SYNC_TS,
			TX_SYNC_TS,
			TX_SYNC_TS,
  			0x00, 0x00};
  
  U08 blink_msg[14] = {'B', 'L', 'N', 'K', 
			DEFAULT_NODE_ID, 
			START_BLINK_N,
			START_BLINK_N,
			TDOA_BLINK_TS,
			TDOA_BLINK_TS,
			TDOA_BLINK_TS,
			TDOA_BLINK_TS,
  			0x00, 0x00};
  
  U32 tx_time;
  U64 tx_time64;
  U16 blink_id = 0;
  U16 blink_n = 0;
  U64 blink_ts = 0;
    
  U16 sync_id = 0;
  U16 sync_n = 0;
  U32 sync_tx_ts = 0;
  U64 sync_rx_ts = 0;
  
  U32 status_reg = 0;
  U16 frame_len = 0;
  
  U08 sync_started;
  U08 blink_started;
};

#endif