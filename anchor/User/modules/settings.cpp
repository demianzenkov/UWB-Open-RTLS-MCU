#include "settings.h"
#include "mx25.h"

DeviceSettings settings;
extern SettingsPB pb_settings;
extern MX25 mx25;


DeviceSettings::DeviceSettings()
{
}

S08 DeviceSettings::init()
{
  U08 sErr;
  sErr = mx25.read(mx25.partition(PARTNUM_SETTINGS)->addr_first, 
		   pb_settings.temp_buf, 
		   Settings_size + 2);
  if(sErr != RC_ERR_NONE)
    return sErr;
  
  U16 tmp_len = (pb_settings.temp_buf[0] << 8) + pb_settings.temp_buf[1];
  
  sErr = pb_settings.decode(&pb_settings.temp_buf[2], 
			    tmp_len, 
			    &pb_settings.message);
  
  /* No valid settings data found */ 
  if(sErr != RC_ERR_NONE)
  {
    /* Erase subsector */
    sErr = mx25.sector_erase(mx25.partition(PARTNUM_SETTINGS)->addr_first);
    if(sErr != RC_ERR_NONE)
      return sErr;
    
    /* Set local settings to default values */
    pb_settings.setDefaults();
    
    /* Write default settings to memory */
    
    sErr = mx25.wr_page(mx25.partition(PARTNUM_SETTINGS)->addr_first, 
		   pb_settings.temp_buf, 
		   pb_settings.buf_len+2);
    
    if(sErr != RC_ERR_NONE)
      return sErr;
    
    /* Read again after first set */
    sErr = mx25.read(mx25.partition(PARTNUM_SETTINGS)->addr_first, 
		     pb_settings.temp_buf, 
		     Settings_size+2);
    
    tmp_len = (pb_settings.temp_buf[0] << 8) + pb_settings.temp_buf[1];
    
    if(sErr != RC_ERR_NONE)
      return sErr;
  
    sErr = pb_settings.decode(&pb_settings.temp_buf[2], 
			      tmp_len, 
			      &pb_settings.message);
    
    if(sErr != RC_ERR_NONE)
      return sErr;
  }
  
  return RC_ERR_NONE;
}

