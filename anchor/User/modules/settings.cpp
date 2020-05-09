#include "settings.h"
#include "mx25.h"



DeviceSettings settings;
extern MX25 mx25;


DeviceSettings::DeviceSettings()
{
}

S08 DeviceSettings::init()
{
  S08 sErr = getSettings(NULL, NULL);
  /* No valid settings data found */ 
  if(sErr != RC_ERR_NONE)
  {
    /* Set local settings to default values */
    setDefaultSettings();
  }
  
  net_conf.init(&pb_settings.message);
  
  return RC_ERR_NONE;
}

S08 DeviceSettings::setDefaultSettings()
{ 
  pb_settings.clearMessage();
  
  pb_settings.message.DeviceID = DEFAULT_DEVICE_ID_U32;
  pb_settings.message.NodeID = DEFAULT_DEVICE_ID_U32;
  pb_settings.message.NodeType = Settings_node_type_TYPE_ANCHOR;
  pb_settings.message.RTLSMode = Settings_rtls_mode_MODE_OFF;
  
  pb_settings.message.DeviceIp = this->net_conf.getDefaultDeviceIp32();
  pb_settings.message.SubnetMask = net_conf.getDefaultSubnetMask32();
  pb_settings.message.GatewayIp = net_conf.getDefaultGatewayIp32();
  pb_settings.message.ServerIp = net_conf.getDefaultServerIp32();
  pb_settings.message.ConnectionPort = net_conf.getDefaultServerPort();
  
  pb_settings.message.PositionX = 1.0;
  pb_settings.message.PositionY = 2.0;
  pb_settings.message.PositionZ = 3.0;
  
  pb_settings.message.ConnectedAnchors_count = DEFAULT_TWR_ANCHOR_CNT;
  for(int i=0; i<DEFAULT_TWR_ANCHOR_CNT; i++) 
    pb_settings.message.ConnectedAnchors[i] = i+1;
  
  pb_settings.message.DWRxAntDelay = DEFAULT_RX_ANT_DLY;
  pb_settings.message.DWTxAntDelay = DEFAULT_TX_ANT_DLY;
  
  pb_settings.message.PollPeriod = 1000;	
  pb_settings.message.PollDelay = -1;		// -1 = 0
  
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
  sErr = mx25.read(mx25.partition(PARTNUM_SETTINGS)->addr_first, 
		   pb_settings.temp_buf, 
		   Settings_size + 2);
  if(sErr != RC_ERR_NONE)
    return sErr;
  
  return sErr;
}

S08 DeviceSettings::getSettings(U08 ** buf, U16 * len)
{
  U08 sErr;
  sErr = readSettingsFromFlash();
  if(sErr != RC_ERR_NONE)
    return sErr;
  
  U16 len_buf = (pb_settings.temp_buf[0] << 8) + pb_settings.temp_buf[1];
  
  
  sErr = pb_settings.decode(&pb_settings.temp_buf[2], 
			    len_buf, 
			    &pb_settings.message);
  if(sErr != RC_ERR_NONE)
    return sErr;
  
  if (buf == NULL || len == 0) {
    return sErr;
  }
  
  *buf = &pb_settings.temp_buf[2];
  *len = len_buf;
 
  return RC_ERR_NONE;
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
  S08 sErr = mx25.sector_erase(mx25.partition(PARTNUM_SETTINGS)->addr_first);
  if(sErr != RC_ERR_NONE)
    return sErr;
  
  U08 len_buf[2] = {(U08)((len >> 8) & 0xFF), (U08)(len & 0xFF)};
  sErr = mx25.wr_page(mx25.partition(PARTNUM_SETTINGS)->addr_first, 
		      len_buf, 2);
  if(sErr != RC_ERR_NONE)
    return sErr;
  
  sErr = mx25.wr_page((mx25.partition(PARTNUM_SETTINGS)->addr_first) + 2, 
		      buf, len);
  if(sErr != RC_ERR_NONE)
    return sErr;
  
  /* Check settings after write */
  sErr = mx25.read(mx25.partition(PARTNUM_SETTINGS)->addr_first, 
		   pb_settings.temp_buf, 
		   Settings_size+2);
  U16 tmp_len = (pb_settings.temp_buf[0] << 8) + pb_settings.temp_buf[1];
  
  if(sErr != RC_ERR_NONE)
    return sErr;
  
  sErr = pb_settings.decode(&pb_settings.temp_buf[2], 
			    tmp_len, 
			    &pb_settings.message);
  if(sErr != RC_ERR_NONE)
    return sErr;
  
  return RC_ERR_NONE;
}
