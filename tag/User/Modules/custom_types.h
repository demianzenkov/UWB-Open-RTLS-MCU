#ifndef _CUSTOM_TYPES
#define _CUSTOM_TYPES

typedef unsigned char		U08;
typedef signed char		S08;
typedef unsigned short int	U16;
typedef signed short int	S16;
typedef unsigned long		U32;
typedef signed long		S32;
typedef unsigned long long	U64;
typedef signed long long	S64;

typedef volatile U08		VU08;
typedef volatile S08		VS08;
typedef volatile U16		VU16;
typedef volatile S16		VS16;
typedef volatile U32		VU32;
typedef volatile S32		VS32;
typedef volatile U64		VU64;
typedef volatile S64		VS64;

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

typedef enum {
  NO_ERR = 0,
  ERR_CRC = -1,
  ERR_INIT = -2,
  ERR_DATA = -3,
  ERR_CNT = -4,
  ERR_VAR = -5,
  ERR_PROTO = -6,
  ERR_SOC = -7,
  ERR_TYPE = -8,
} err_te;

#endif