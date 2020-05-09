#ifndef __WAKE
#define __WAKE

#include "prj_defs.h"
#include "bsp_os.h"

#define FEND  			0xC0    //Frame END
#define FESC  			0xDB    //Frame ESCape
#define TFEND 			0xDC    //Transposed Frame END
#define TFESC 			0xDD    //Transposed Frame ESCape

#define SYN			0xC0
#define ESC			0xDB
#define ESC_SYN			0xDC
#define ESC_ESC			0xDD

#define WAKE_FRAME		1536     // max packet len

#define CMD_I_AM_HERE_REQ		0x31
#define CMD_I_AM_HERE_RESP		0x32
#define CMD_REBOOT_REQ			0x33
#define CMD_REBOOT_RESP			0x34

#define CMD_GET_SETTINGS_REQ		0x11
#define CMD_GET_SETTINGS_RESP		0x12
#define CMD_SET_SETTINGS_REQ		0x13
#define CMD_SET_SETTINGS_RESP		0x14
#define CMD_SET_DEF_SETTINGS_REQ	0x15
#define CMD_SET_DEF_SETTINGS_RESP	0x16
#define CMD_TWR_RANGING			0x21


class Wake {
public:
  Wake();
  
  /*
  * @brief RX process states
  */
  enum rx_state
  {
    WAIT_FEND,     // wait FEND
    WAIT_ADDR,     // wait address
    WAIT_CMD,      // wait cmd
    WAIT_NBT1,     // wait bytes in packet L
    WAIT_NBT2,     // wait bytes in packet H
    WAIT_DATA,     // wait data 
    WAIT_CRC1,     // wait CRC L
    WAIT_CRC2,     // wait CRC H
    WAIT_FIN
  };
  
  /*
  * @brief TX process states
  */
  enum tx_state
  { 
    SEND_IDLE,	// idle state
    SEND_ADDR,	// send address
    SEND_CMD,	// send cmd
    SEND_NBT,	// send number of bytes
    SEND_DATA,	// send data
    SEND_CRC,	// send CRC
    SEND_END  	// end packet transmit
  };
  
  typedef struct
  {
    U08 addr;			// device address
    U16 crc;			// computed while receiving CRC
    enum rx_state rx_st;	// rx sdtate machine
    U08 pre;			// prev received byte
    U08 cmd;			// cmd 
    U16 len;			// length
    U16 ptr;			// received bytes in block counter
    U16 err_cnt;		// error packets counter
    U08 dbuf[WAKE_FRAME]; 	// buf for packet
    U16 crc_pack;
  } wake_ts;
  
  wake_ts wake;
  
public:
  S08 init();
  inline S08 lock (void);
  inline S08 unlock (void);
  S08 rxHandler(U08 byte);
  S08 prepareBuf(U08 * in_buf, U16 in_len, U08 cmd, U08* out_buf, U16* out_len);
  
private: 
   U08 _buf_add(U08** pp, U08 byte);
   U16 crc16(U16 init, U08 byte);
   
private:
  OS_SEM sem;
};


#endif /* __WAKE*/