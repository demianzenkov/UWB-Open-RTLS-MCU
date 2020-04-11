#include "custom_types.h"

#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_spi.h"
#include "deca_port.h"

/* UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) conversion factor.
 * 1 uus = 512 / 499.2 µs and 1 µs = 499.2 * 128 dtu. */
#define UUS_TO_DWT_TIME 65536

#define FRAME_LEN_MAX 127

class DWM1000 
{
public:
  DWM1000();
  ~DWM1000();
  
  err_te init();
  void resetConfig();
  void configDWM(dwt_config_t * config);
  int8_t receiveEnable();
  void receive();
  
private:
  dwt_config_t config;
  uint32_t RX_ANT_DLY;
  uint32_t TX_ANT_DLY;
  uint32_t status_reg = 0;
  uint16_t frame_len = 0;
  
public:
  uint8_t rx_buffer[FRAME_LEN_MAX];
};