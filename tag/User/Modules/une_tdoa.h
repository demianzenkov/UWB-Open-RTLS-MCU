#ifndef __UNE_TDOA
#define __UNE_TDOA

#include "prj_defs.h"
#include "dwm1000.h"

#define TDOA_MSG_BLINK_ID	4
#define TDOA_MSG_BLINK_L	5
#define TDOA_MSG_BLINK_H	6
#define TDOA_BLINK_TS		7

#define DEFAULT_NODE_ID 0x01
#define START_BLINK_N	0x00

class UNE_TDOA {
public:
  UNE_TDOA(DWM1000 * dwm);
  ~UNE_TDOA();
  
  S08 tdoaBlinkNodeRoutine();
  S08 sendBlinkWithDelay();
  
private:
  DWM1000 * dwm;
  
  void sendImmediate(U08 * packet, U08 size);
  void tagBlink();
  
  U08 blink_msg[14] = {'B', 'L', 'N', 'K', 
			DEFAULT_NODE_ID, 
			START_BLINK_N,
			START_BLINK_N,
			TDOA_BLINK_TS,
			TDOA_BLINK_TS,
			TDOA_BLINK_TS,
			TDOA_BLINK_TS,
  			0x00, 0x00};
  U16 blink_id = 0;
  U16 blink_n;
  U64 blink_ts = 0;
  
  U32 tx_time;
  U64 tx_time64;
  U08 blink_started;
};

#endif