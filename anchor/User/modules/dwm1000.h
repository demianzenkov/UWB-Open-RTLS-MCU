#include "custom_types.h"

#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_spi.h"
#include "deca_port.h"

/* UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) conversion factor.
 * 1 uus = 512 / 499.2 µs and 1 µs = 499.2 * 128 dtu. */
#define UUS_TO_DWT_TIME 65536
#define FRAME_LEN_MAX 127

#define DEFAULT_AREA_ID 0x01
#define DEFAULT_NODE_ID 0x01
#define START_SYNC_N	0x00
#define START_BLYNK_N	0x00

typedef enum {
  SYNC = 0,
  BLYNK,
}packet_type_te;

class DWM1000 
{
public:
  DWM1000();
  ~DWM1000();
  
  err_te init();
  void resetConfig();
  void configDWM(dwt_config_t * config);
  void testReceive();
  
  err_te receiveEnable();
  uint64 getTxTimestampU64(void);
  uint64 getRxTimestampU64(void);
  void sendSyncPacket();
  err_te receivePacket(packet_type_te);
  void collectSocketBuf(uint8_t * out_buf);

private:
  void checkRW();
    
private:
  dwt_config_t config;
  uint32_t RX_ANT_DLY;
  uint32_t TX_ANT_DLY;
  uint32_t status_reg = 0;
  uint16_t frame_len = 0;
  
  uint8 sync_msg[7] = {'S', 'Y', 'N', 'C', 
			DEFAULT_AREA_ID, 
			DEFAULT_NODE_ID, 
			START_SYNC_N};
  uint8 blynk_msg[7] = {'B', 'L', 'N', 'K', 
			DEFAULT_AREA_ID, 
			DEFAULT_NODE_ID, 
			START_BLYNK_N};
  
public:
  uint8_t rx_buffer[FRAME_LEN_MAX];
  uint8_t anchor_id = DEFAULT_NODE_ID;
  uint8_t tag_id = 0;
  uint8_t is_sync_node = 0;
  uint64_t sync_ts = 0;
  uint64_t blynk_ts = 0;
  uint8_t sync_n = 0;
  uint8_t blynk_n = 0;
  
};