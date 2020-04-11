#include "custom_types.h"

#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_spi.h"
#include "deca_port.h"


/* UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) conversion factor.
 * 1 uus = 512 / 499.2 µs and 1 µs = 499.2 * 128 dtu. */
#define UUS_TO_DWT_TIME 65536

class DWM1000 
{
public:
  DWM1000();
  ~DWM1000();
  
  err_te init();
  void resetConfig(dwt_config_t * config);
  void configDWM(dwt_config_t * config);
  void receiveEnable();
  void blink();
  
private:
  dwt_config_t config;
  uint32_t RX_ANT_DLY;
  uint32_t TX_ANT_DLY;
  
  uint8 tx_msg[12] = {0xC5, 0, 'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E', 0, 0};
  uint8_t BLINK_FRAME_SN_IDX = 1;
};