#ifndef __LSM6
#define __LSM6

#include "main.h"
#include "lsm6ds3tr_c_reg.h"
#include "tm_stm32_ahrs_imu.h"

#define TX_BUF_DIM          1000


typedef union{
  int16_t i16bit[3];
  uint8_t u8bit[6];
} axis3bit16_t;

typedef union{
  int16_t i16bit;
  uint8_t u8bit[2];
} axis1bit16_t;

#define CS_DEV_GPIO_Port	GPIOB   		
#define CS_DEV_Pin  		SPI2_NSS_IMU_Pin	


class LSM6 
{
public:
  LSM6();
 
  void init();
  
  void tx_com( uint8_t *tx_buffer, uint16_t len );
  static int32_t platform_write(void *handle, uint8_t Reg, uint8_t *Bufp, uint16_t len);
  static int32_t platform_read(void *handle, uint8_t Reg, uint8_t *Bufp, uint16_t len);
  
  int8_t checkWhoAmI();
  void config();
  void poll();
  void updateAHRS();
  
  void processEvent();
private:
  stmdev_ctx_t dev_ctx;
  
  axis3bit16_t data_raw_acceleration;
  axis3bit16_t data_raw_angular_rate;
  axis1bit16_t data_raw_temperature;
  float acceleration_mg[3];
  float angular_rate_mdps[3];
  float temperature_degC;
  uint8_t whoamI, rst;
  uint8_t tx_buffer[TX_BUF_DIM];
  
  TM_AHRSIMU_t AHRSIMU;
};

#endif //__LSM6