#include "settings_pb.h"
#include "net_conf.h"

SettingsPB pb_settings;
extern NetConfig net_conf;

SettingsPB::SettingsPB()
{
}

S08 SettingsPB::init()
{
  return RC_ERR_NONE;
}

S08 SettingsPB::setDefaults()
{ 
  clearMessage();
  message.DeviceID = 1;
  message.DeviceIp = net_conf.getDeviceIp32();
  message.GatewayIp = net_conf.getGatewayIp32();
  message.ServerIp = net_conf.getServerIp32();
  message.ConnectionPort = net_conf.getServerPort();
  
  S08 sErr = encode(&message, &temp_buf[2], &buf_len);
  temp_buf[0] = (buf_len >> 8) & 0xFF;
  temp_buf[1] = buf_len & 0xFF;
  
  if (sErr != RC_ERR_NONE)
    return sErr;
  
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
