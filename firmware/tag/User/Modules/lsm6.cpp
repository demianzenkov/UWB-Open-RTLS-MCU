#include "lsm6.h"
#include <string.h>
#include <stdio.h>
#include "usbd_cdc_if.h"

extern SPI_HandleTypeDef hspi2;

LSM6::LSM6()
{
  
}

void LSM6::init()
{
  dev_ctx.write_reg = platform_write;
  dev_ctx.read_reg = platform_read;
  dev_ctx.handle = &hspi2;

  TM_AHRSIMU_Init(&AHRSIMU, 0.1f, 10, 0.1f);
}

void LSM6::tx_com( uint8_t *tx_buffer, uint16_t len )
{
  CDC_Transmit_FS(tx_buffer, len);
}

void LSM6::config()
{
  /*
   *  Restore default configuration
   */
  lsm6ds3tr_c_reset_set(&dev_ctx, PROPERTY_ENABLE);
  do {
    lsm6ds3tr_c_reset_get(&dev_ctx, &rst);
  } while (rst);
  /*
   *  Enable Block Data Update
   */
  lsm6ds3tr_c_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);
  /*
   * Set Output Data Rate
   */
  lsm6ds3tr_c_xl_data_rate_set(&dev_ctx, LSM6DS3TR_C_XL_ODR_12Hz5);
  lsm6ds3tr_c_gy_data_rate_set(&dev_ctx, LSM6DS3TR_C_GY_ODR_12Hz5);
  /*
   * Set full scale
   */ 
  lsm6ds3tr_c_xl_full_scale_set(&dev_ctx, LSM6DS3TR_C_2g);
  lsm6ds3tr_c_gy_full_scale_set(&dev_ctx, LSM6DS3TR_C_2000dps);
 
  /*
   * Configure filtering chain(No aux interface)
   */ 
  /* Accelerometer - analog filter */
  lsm6ds3tr_c_xl_filter_analog_set(&dev_ctx, LSM6DS3TR_C_XL_ANA_BW_400Hz);
 
  /* Accelerometer - LPF1 path ( LPF2 not used )*/
  //lsm6ds3tr_c_xl_lp1_bandwidth_set(&dev_ctx, LSM6DS3TR_C_XL_LP1_ODR_DIV_4);
 
  /* Accelerometer - LPF1 + LPF2 path */  
  lsm6ds3tr_c_xl_lp2_bandwidth_set(&dev_ctx, LSM6DS3TR_C_XL_LOW_NOISE_LP_ODR_DIV_100);
 
  /* Accelerometer - High Pass / Slope path */
  //lsm6ds3tr_c_xl_reference_mode_set(&dev_ctx, PROPERTY_DISABLE);
  //lsm6ds3tr_c_xl_hp_bandwidth_set(&dev_ctx, LSM6DS3TR_C_XL_HP_ODR_DIV_100);
 
  /* Gyroscope - filtering chain */
  lsm6ds3tr_c_gy_band_pass_set(&dev_ctx, LSM6DS3TR_C_HP_260mHz_LP1_STRONG);
  
  /* Set free fall duration and threshold */
  lsm6ds3tr_c_ff_dur_set(&dev_ctx, 10);
  lsm6ds3tr_c_ff_threshold_set(&dev_ctx, LSM6DS3TR_C_FF_TSH_250mg);
}

void LSM6::poll()
{
  /*
  * Read output only if new value is available
  */
  lsm6ds3tr_c_reg_t reg;
  lsm6ds3tr_c_status_reg_get(&dev_ctx, &reg.status_reg);
  
  if (reg.status_reg.xlda)
  {
    /* Read magnetic field data */
    memset(data_raw_acceleration.u8bit, 0x00, 3*sizeof(int16_t));
    lsm6ds3tr_c_acceleration_raw_get(&dev_ctx, data_raw_acceleration.u8bit);
    acceleration_mg[0] = lsm6ds3tr_c_from_fs2g_to_mg( data_raw_acceleration.i16bit[0]);
    acceleration_mg[1] = lsm6ds3tr_c_from_fs2g_to_mg( data_raw_acceleration.i16bit[1]);
    acceleration_mg[2] = lsm6ds3tr_c_from_fs2g_to_mg( data_raw_acceleration.i16bit[2]);
    
//    sprintf((char*)tx_buffer, "ACC [ mg ]:\t%4.2f\t%4.2f\t%4.2f\r\n",
//	    acceleration_mg[0], acceleration_mg[1], acceleration_mg[2]);
//    tx_com( tx_buffer, strlen( (char const*)tx_buffer ) );
  }
  if (reg.status_reg.gda)
  {
    /* Read magnetic field data */
    memset(data_raw_angular_rate.u8bit, 0x00, 3*sizeof(int16_t));
    lsm6ds3tr_c_angular_rate_raw_get(&dev_ctx, data_raw_angular_rate.u8bit);
    angular_rate_mdps[0] = lsm6ds3tr_c_from_fs2000dps_to_mdps(data_raw_angular_rate.i16bit[0]);
    angular_rate_mdps[1] = lsm6ds3tr_c_from_fs2000dps_to_mdps(data_raw_angular_rate.i16bit[1]);
    angular_rate_mdps[2] = lsm6ds3tr_c_from_fs2000dps_to_mdps(data_raw_angular_rate.i16bit[2]);
    
//    sprintf((char*)tx_buffer, "GYR [mdps]:%4.2f\t%4.2f\t%4.2f\r\n",
//	    angular_rate_mdps[0], angular_rate_mdps[1], angular_rate_mdps[2]);
//    tx_com( tx_buffer, strlen( (char const*)tx_buffer ) );
  }   
  if (reg.status_reg.tda)
  {  
    /* Read temperature data */
    memset(data_raw_temperature.u8bit, 0x00, sizeof(int16_t));
    lsm6ds3tr_c_temperature_raw_get(&dev_ctx, data_raw_temperature.u8bit);
    temperature_degC = lsm6ds3tr_c_from_lsb_to_celsius( data_raw_temperature.i16bit );
    
//    sprintf((char*)tx_buffer, "TMP [degC]:%4.2f\r\n", temperature_degC );
//    tx_com( tx_buffer, strlen( (char const*)tx_buffer ) );
  }
}

int32_t LSM6::platform_write(void *handle, uint8_t Reg, uint8_t *Bufp, uint16_t len)
{
  
  HAL_GPIO_WritePin(CS_DEV_GPIO_Port, CS_DEV_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit((SPI_HandleTypeDef*)handle, &Reg, 1, 1000);
  HAL_SPI_Transmit((SPI_HandleTypeDef*)handle, Bufp, len, 1000);
  HAL_GPIO_WritePin(CS_DEV_GPIO_Port, CS_DEV_Pin, GPIO_PIN_SET);
  
  return 0;
}

int32_t LSM6::platform_read(void *handle, uint8_t Reg, uint8_t *Bufp, uint16_t len)
{  
  Reg |= 0x80;
  HAL_GPIO_WritePin(CS_DEV_GPIO_Port, CS_DEV_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit((SPI_HandleTypeDef*)handle, &Reg, 1, 1000);
  HAL_SPI_Receive((SPI_HandleTypeDef*)handle, Bufp, len, 1000);
  HAL_GPIO_WritePin(CS_DEV_GPIO_Port, CS_DEV_Pin, GPIO_PIN_SET);
  
  return 0;
}

int8_t LSM6::checkWhoAmI()
{
  whoamI = 0;
  lsm6ds3tr_c_device_id_get(&dev_ctx, &whoamI);
  if ( whoamI != LSM6DS3TR_C_ID )
    return -1;
  return 0;
}

void LSM6::updateAHRS() 
{
  
  TM_AHRSIMU_UpdateIMU(&AHRSIMU, 
			AHRSIMU_DEG2RAD(angular_rate_mdps[0]/1000), 
			AHRSIMU_DEG2RAD(angular_rate_mdps[1]/1000), 
			AHRSIMU_DEG2RAD(angular_rate_mdps[2]/1000), 
			acceleration_mg[0]/1000, 
			acceleration_mg[1]/1000, 
			acceleration_mg[2]/1000);
  /* Read new roll, pitch and yaw values */
  sprintf((char*)tx_buffer, 
	  "R: %3.3f\tP: %3.3f\tY: %3.3f\r\n", 
	  AHRSIMU.Roll, 
	  AHRSIMU.Pitch, 
	  AHRSIMU.Yaw);
  tx_com( tx_buffer, strlen( (char const*)tx_buffer ) );
}

void LSM6::processEvent()
{
//  lsm6ds3tr_c_reg_t reg;
//  lsm6ds3tr_c_status_reg_get(&dev_ctx, &reg.free_fall);
}
