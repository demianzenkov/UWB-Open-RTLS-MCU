#include "bsp_nvm.h"

BSP_NVM bsp_nvm;

static const BSP_NVM::mem_part part_sett = {
   .page_first = FLASH_SETT_PAGE_FIRST,	// first page num
   .page_last = FLASH_SETT_PAGE_LAST,	// last page num
   .addr_first = FLASH_SETT_ADDR_FIRST,	// first section address
   .addr_last = FLASH_SETT_ADDR_LAST,	// last section address
   .mem_size = FLASH_PAGE_SIZE * (FLASH_SETT_PAGE_LAST - FLASH_SETT_PAGE_FIRST + 1), // size of memory for the partition
   .page_size = FLASH_PAGE_SIZE		// page size
};


BSP_NVM::BSP_NVM()
{
  sem = NULL;
  if (BSP_OS::semCreate(&sem, 1, (char*)"flash_sem") != RC_ERR_NONE)
  {
    sem = NULL;
  }
}

S08 BSP_NVM::init ()
{
  if (!sem) 
    return RC_ERR_RTOS;

  p_part = &part_sett;
  
  return RC_ERR_NONE;
}

S08 BSP_NVM::lock (void)
{
  if ((sem == 0) || (&sem == 0)) {
    return RC_ERR_PARAM;
  }
  
  if (osSemaphoreWait(sem, osWaitForever) == osOK )
    return RC_ERR_NONE;
  else 
    return RC_ERR_TIMEOUT;
  
}

S08 BSP_NVM::unlock (void)
{
  if ((sem == 0) ||  (&sem == 0)) {
    return RC_ERR_PARAM;
  }
  
  osSemaphoreRelease (sem);
  return RC_ERR_NONE;
}

void BSP_NVM::read(U32 addr, U08* buf, U16 len)
{
  lock();
  for(int i=0; i<len; i++)
  {
    buf[i] = *((U08 *)addr + i);
  }
  unlock();
}

S08 BSP_NVM::write(U32 addr, U08* buf, U16 len)
{
  if (addr%4)
    return RC_ERR_ADDR;
  
  U08 left_cnt = len%4;
  U16 word_cnt = len/4;
  
  lock();
  HAL_FLASH_Unlock();
  for(int i=0; i<word_cnt-1;i++)
  {
    U32 block = buf[i*4]+(buf[i*4+1]<<8)+(buf[i*4+2]<<16)+(buf[i*4+3]<<24);
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr+i*4, block) != HAL_OK)
    {
      HAL_FLASH_Lock();
      unlock();
      return RC_ERR_HW;
    }
  }
  if(!left_cnt) {
    U32 block = buf[(word_cnt-1)*4]+
		     (buf[(word_cnt-1)*4+1]<<8)+
		       (buf[(word_cnt-1)*4+2]<<16)+
			 (buf[(word_cnt-1)*4+3]<<24);
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr+(word_cnt-1)*4, block) != HAL_OK)
    {
      HAL_FLASH_Lock();
      unlock();
      return RC_ERR_HW;
    }
  }
  else {
    U32 last_block = 0;
    for(int i=0; i<left_cnt; i++){
      last_block |= ((U32)buf[len-1-i] << (3-i)*8);
    }
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr+(word_cnt-1)*4, last_block) != HAL_OK)
    {
      HAL_FLASH_Lock();
      unlock();
      return RC_ERR_HW;
    } 
  }
  HAL_FLASH_Lock();
  unlock();
  return RC_ERR_NONE;
}

S08 BSP_NVM::pagesErase(U32 addr, U16 pages)
{
  U32 errPage;
  lock();
  HAL_FLASH_Unlock();

  BSP_NVM::erase_st.TypeErase   = FLASH_TYPEERASE_PAGES;
  BSP_NVM::erase_st.PageAddress = addr;
  BSP_NVM::erase_st.NbPages     = pages;
  
  if (HAL_FLASHEx_Erase(&bsp_nvm.erase_st, (U32*)&errPage) != HAL_OK)
  {
    HAL_FLASH_Lock();
    unlock();
    return RC_ERR_HW;
  }
  HAL_FLASH_Lock();
  unlock();
  return RC_ERR_NONE;
}

const BSP_NVM::mem_part * BSP_NVM::partition(mem_part_num_te part)
{
  if (part == PARTNUM_SETTINGS)    
    return &part_sett;
  
  return 0;
}