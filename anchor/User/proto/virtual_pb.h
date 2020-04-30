#ifndef __VIRTUAL_PB
#define __VIRTUAL_PB

#include "cmsis_os.h"
#include "prj_defs.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "bsp_os.h"


class VirtualPB
{
public:
  VirtualPB();
  
  S08 decode_base(U08 * buffer, U16 len, void * out_msg, 
		     const pb_msgdesc_t *fields);
  S08 encode_base(void * in_msg, U08 * buffer, 
		  U16 * len, const pb_msgdesc_t *fields, U16 msg_size=0);
  S08 lock (void);
  S08 unlock (void);
  
public:
  pb_ostream_t pb_out_stream;	/* a stream that will write to buffer */
  pb_istream_t pb_in_stream;	/* a stream that reads from the buffer */

private:
  OS_SEM sem;
  
};  

#endif /* __VIRTUAL_PB */
