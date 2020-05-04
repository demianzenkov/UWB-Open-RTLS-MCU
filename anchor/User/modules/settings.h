#ifndef __DEV_SETTINGS
#define __DEV_SETTINGS

#include "settings_pb.h"
#include "net_conf.h"

class DeviceSettings
{
public:
  DeviceSettings();
  S08 init();
  
  S08 setDefaultSettings();
  S08 getSettings(U08 ** buf, U16 * len);
  S08 setSettingsPb(U08 * buf, U16 len);
  
private:
  S08 readSettingsFromFlash();
  S08 writeSettingsToFlash(U08 * buf, U16 len);
  
public:
  SettingsPB pb_settings;
  NetConfig net_conf;
  

  
};

#endif /* __DEV_SETTINGS */