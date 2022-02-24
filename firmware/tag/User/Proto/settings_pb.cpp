#include "settings_pb.h"

SettingsPB::SettingsPB()
{
}

S08 SettingsPB::init()
{
  return RC_ERR_NONE;
}

void SettingsPB::clearMessage()
{
  message = Settings_init_zero;
}

S08 SettingsPB::encode(void * in_msg, U08 * buffer, U16 * len)
{
  bool status;
  lock();
  pb_out_stream = pb_ostream_from_buffer(buffer, Settings_size);
  status = pb_encode(&pb_out_stream, Settings_fields, in_msg);
  *len = pb_out_stream.bytes_written;
  unlock();
  if (!status)
    return RC_ERR_ACCESS;
  
  return RC_ERR_NONE;
}
