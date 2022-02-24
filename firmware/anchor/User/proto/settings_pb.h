#ifndef __SETTINGS_PB
#define __SETTINGS_PB

#include "cmsis_os.h"
#include "prj_defs.h"
#include "virtual_pb.h"
#include "Settings.pb.h"


class SettingsPB : VirtualPB
{
public:
  SettingsPB();
  
  S08 decode(U08 * buffer, U16 len, void * out_msg) {
    return decode_base(buffer, len, out_msg, Settings_fields);
  };
  
  S08 encode(void * in_msg, U08 * buffer, U16 * len);
  
  S08 init();
  void clearMessage();
  
public:
  Settings message = Settings_init_zero;
  U08 temp_buf[Settings_size + 2];	// + U16 len
  U16 buf_len;
};  

#endif /* __SETTINGS_PB */
