#ifndef __BSP_NVM
#define __BSP_NVM

#include "stm32l0xx_hal.h"

#include "prj_defs.h"
#include "bsp_os.h"

#define FLASH_PAGE_FIRST 	(0)
#define FLASH_PAGE_LAST 	(511)

#define FLASH_SETT_PAGE_FIRST 	(FLASH_PAGE_LAST-2)
#define FLASH_SETT_PAGE_LAST	(FLASH_PAGE_LAST)
#define FLASH_SETT_ADDR_FIRST 	(FLASH_BASE + FLASH_SETT_PAGE_FIRST*FLASH_PAGE_SIZE)
#define FLASH_SETT_ADDR_LAST	(FLASH_BASE + FLASH_SETT_PAGE_LAST*FLASH_PAGE_SIZE)

class BSP_NVM
{
public:
  BSP_NVM();
  
  typedef struct
  {
    U16 page_first;	// first page num
    U16 page_last;	// last page num
    U32 addr_first;	// first section address
    U32 addr_last;	// last section address
    U32 mem_size;	// size of memory for the partition
    U32 page_size;	// page size
  } mem_part;
  
  typedef enum
  {
    PARTNUM_EMPTY = -1,
    PARTNUM_SETTINGS = 0,
  } mem_part_num_te;
  
  S08 init ();
  void read (U32 addr, U08* p_buf, U16 len);
  S08 write (U32 addr, U08* p_buf, U16 len);
  S08 pagesErase (U32 addr, U16 pages=1);
  
  inline S08 lock (void);
  inline S08 unlock (void);
  const mem_part * partition(mem_part_num_te part);
  
private:
  static const mem_part * p_part;
  FLASH_EraseInitTypeDef erase_st;
  U08 temp_buf[FLASH_PAGE_SIZE];
  OS_SEM sem;
  
};

#endif