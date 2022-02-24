#include "virtual_pb.h"


VirtualPB::VirtualPB()
{
  if (sem == NULL)
    BSP_OS::semCreate (&sem, 1, (char*)"sem_pb");
}

S08 VirtualPB::decode_base(U08 * buffer, U16 len, void * out_msg,
		      const pb_msgdesc_t * fields)
{
  bool status;
  lock();
  pb_in_stream = pb_istream_from_buffer(buffer, len);
  status = pb_decode(&pb_in_stream, fields, out_msg);
  unlock();
  if (!status)
    return RC_ERR_ACCESS;
  
  return RC_ERR_NONE;
}

S08 VirtualPB::encode_base(void * in_msg, U08 * buffer,
			   U16 * len, const pb_msgdesc_t * fields, U16 msg_size)
{
  bool status;
  lock();
  pb_out_stream = pb_ostream_from_buffer(buffer, msg_size);
  status = pb_encode(&pb_out_stream, fields, in_msg);
  *len = pb_out_stream.bytes_written;
  unlock();
  
  if (!status)
    return RC_ERR_ACCESS;
  
  return RC_ERR_NONE;
}

S08 VirtualPB::lock (void)
{
  if (sem)
  {
    S08 sErr = BSP_OS::semWait (&sem, 10.0 * BSP_TICKS_PER_SEC);
    return sErr;
  }
  else
    return RC_ERR_COMMON;
}


S08 VirtualPB::unlock (void)
{
  if (sem)
  {
    S08 sErr = BSP_OS::semPost (&sem);
    return sErr;
  }
  else
    return RC_ERR_COMMON;
}