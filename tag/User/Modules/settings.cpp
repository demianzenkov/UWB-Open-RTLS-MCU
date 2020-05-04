#include "settings.h"
#include "bsp_nvm.h"

#define DEFAULT_DEVICE_ID_U32	0xAABBDDCC

DeviceSettings settings;
extern BSP_NVM bsp_nvm;

DeviceSettings::DeviceSettings()
{
}

S08 DeviceSettings::init()
{
  S08 sErr = readSettingsFromFlash();
  /* No valid settings data found */ 
  if(sErr != RC_ERR_NONE)
  {
    /* Set local settings to default values */
    setDefaultSettings();
  }
  
  return RC_ERR_NONE;
}

S08 DeviceSettings::setDefaultSettings()
{ 
  pb_settings.clearMessage();
  
  pb_settings.message.DeviceID = DEFAULT_DEVICE_ID_U32;
  
  pb_settings.message.NodeID = DEFAULT_DEVICE_ID_U32;
  pb_settings.message.NodeType = Settings_node_type_TYPE_TAG;
  pb_settings.message.RTLSMode = Settings_rtls_mode_MODE_TWR;
  
  pb_settings.message.PositionX = 0;
  pb_settings.message.PositionY = 0;
  pb_settings.message.PositionZ = 0;
  
  S08 sErr = pb_settings.encode(&pb_settings.message, 
				pb_settings.temp_buf, 
				&pb_settings.buf_len);
  if (sErr != RC_ERR_NONE)
    return sErr;
  
  sErr = writeSettingsToFlash(pb_settings.temp_buf, pb_settings.buf_len);
  if (sErr != RC_ERR_NONE)
    return sErr;
  
  return RC_ERR_NONE;
}

/**
  * @brief Function read settings from flash and decodes to 
  *	   pb_settings.message local proto struct
  */
S08 DeviceSettings::readSettingsFromFlash()
{
  U08 sErr;
  bsp_nvm.read(bsp_nvm.partition(BSP_NVM::PARTNUM_SETTINGS)->addr_first, 
		   pb_settings.temp_buf, 
		   Settings_size + 4);
  
  U16 tmp_len = (pb_settings.temp_buf[2] << 8) + pb_settings.temp_buf[3];
  
  if (tmp_len) {
    sErr = pb_settings.decode(&pb_settings.temp_buf[4], 
			      tmp_len, 
			      &pb_settings.message);
  }
  else {
    return RC_ERR_DATA;
  }
  return sErr;
}

S08 DeviceSettings::getSettings(U08 ** buf, U16 * len)
{
  readSettingsFromFlash();
  S08 sErr = pb_settings.encode(&pb_settings.message, pb_settings.temp_buf, &pb_settings.buf_len);
  *buf = &pb_settings.temp_buf[0];
  *len = pb_settings.buf_len;
  return sErr;
}

S08 DeviceSettings::setSettingsPb(U08 * buf, U16 len)
{
  S08 sErr;
  
  sErr = pb_settings.decode(buf, len, &pb_settings.message);
  if(sErr != RC_ERR_NONE)
    return sErr;
  
  sErr = writeSettingsToFlash(buf, len);
  if(sErr != RC_ERR_NONE)
    return sErr;
  
  return RC_ERR_NONE;
}

S08 DeviceSettings::writeSettingsToFlash(U08 * buf, U16 len)
{
  S08 sErr = bsp_nvm.pagesErase(bsp_nvm.partition(BSP_NVM::PARTNUM_SETTINGS)->addr_first);
  if(sErr != RC_ERR_NONE)
    return sErr;
  
  U08 len_buf[4] = {0, 0, (U08)((len >> 8) & 0xFF), (U08)(len & 0xFF)};
  sErr = bsp_nvm.write(bsp_nvm.partition(BSP_NVM::PARTNUM_SETTINGS)->addr_first, 
		      len_buf, 4);
  if(sErr != RC_ERR_NONE)
    return sErr;
  
  sErr = bsp_nvm.write((bsp_nvm.partition(BSP_NVM::PARTNUM_SETTINGS)->addr_first) + 4, 
		      buf, len);
  if(sErr != RC_ERR_NONE)
    return sErr;
  
  /* Check settings after write */
  bsp_nvm.read(bsp_nvm.partition(BSP_NVM::PARTNUM_SETTINGS)->addr_first, 
	       pb_settings.temp_buf, 
	       Settings_size+4);
  U16 tmp_len = (pb_settings.temp_buf[2] << 8) + pb_settings.temp_buf[3];
  
  
  sErr = pb_settings.decode(&pb_settings.temp_buf[4], 
			    tmp_len, 
			    &pb_settings.message);
  if(sErr != RC_ERR_NONE)
    return sErr;
  
  return RC_ERR_NONE;
}
