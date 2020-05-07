#ifndef __DEV_SETTINGS
#define __DEV_SETTINGS

#include "settings_pb.h"

#define DEFAULT_DEVICE_ID_U32	0xAABBDDCC
#define DEFAULT_TWR_ANCHOR_CNT	4
#define DEFAULT_TWR_TAG_DELAY	1000
#define DEFAULT_TX_ANT_DLY 	16436
#define DEFAULT_RX_ANT_DLY 	16436

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
  
};

#endif /* __DEV_SETTINGS */