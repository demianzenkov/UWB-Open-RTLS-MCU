#ifndef __PRJ_DEFS
#define __PRJ_DEFS

#include "stdint.h"

#define MAX_QUEUE_SIZE	128

/* S08 Return Codes */
#define RC_ERR_NONE	(0)
#define RC_ERR_PARAM	(-1)
#define RC_ERR_RANGE	(-2)
#define RC_ERR_VALUE	(-3)
#define RC_ERR_SEQ	(-4)
#define RC_ERR_MEM	(-5)
#define RC_ERR_ADDR	(-6)
#define RC_ERR_CMD	(-7)
#define RC_ERR_DATA	(-8)
#define RC_ERR_CRC	(-9)
#define RC_ERR_IO	(-10)
#define RC_ERR_HW	(-11)
#define RC_ERR_RTOS	(-12)
#define RC_ERR_ACCESS	(-13)
#define RC_ERR_TIMEOUT	(-14)
#define RC_ERR_RESP	(-15)
#define RC_ERR_PWR_OFF	(-16)
#define RC_ERR_COMMON	(-30)
#define RC_ERR_UNKNOWN	(-31)
#define RC_ERR_DISABLE	(-32)
#define RC_ERR_NOTSPTD	(-33)

typedef uint8_t		U08;
typedef int8_t		S08;
typedef int16_t		U16;
typedef int16_t		S16;
typedef uint32_t	U32;
typedef int32_t		S32;
typedef uint64_t	U64;
typedef int64_t		S64;

typedef volatile U08	VU08;
typedef volatile S08	VS08;
typedef volatile U16	VU16;
typedef volatile S16	VS16;
typedef volatile U32	VU32;
typedef volatile S32	VS32;
typedef volatile U64	VU64;
typedef volatile S64	VS64;

#define FALSE			0
#define TRUE			1

#define S16_MIN_VAL		-32767
#define S16_MAX_VAL		32767
#define S32_MIN_VAL		-2147483647
#define S32_MAX_VAL		2147483647
#define S64_MIN_VAL		-9223372036854775807
#define S64_MAX_VAL		9223372036854775807

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef struct {
  U08 data[MAX_QUEUE_SIZE];
  U16 len;
} queue_data_t;

#endif /* __PRJ_DEFS */