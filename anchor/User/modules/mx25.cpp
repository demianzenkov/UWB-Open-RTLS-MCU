#include "mx25.h"
#include "string.h"

MX25 mx25;

static const mem_part mx25_settings = {
  .unit_first = MX25_16MB_SETT_SECTOR_FIRST,
  .unit_last  = MX25_16MB_SETT_SECTOR_LAST,
  .addr_first = MX25_16MB_SETT_ADDR_FIRST,
  .addr_last = MX25_16MB_SETT_ADDR_LAST,
  .mem_size   = MX25_UNIT_SIZE * (MX25_16MB_SETT_SECTOR_LAST - MX25_16MB_SETT_SECTOR_FIRST + 1),
  .hw_type    = FL_TYPE_MX25_16MB,
  .pn         = PARTNUM_SETTINGS,
  .unit_size  = MX25_SECTOR_SIZE,
  .page_size  = MX25_PAGE_SIZE,
  .block_size = MX25_BLOCK_SIZE
};


MX25::MX25()
{
  sem = NULL;
  if (BSP_OS::semCreate(&sem, 1, (char*)"flash_sem") != RC_ERR_NONE)
  {
    sem = NULL;
  }
}

S08 MX25::init (BSP_SPI * spi)
{
  if (spi == NULL)
    return RC_ERR_PARAM;
  
  bsp_spi = spi;

  if (!sem) 
    return RC_ERR_RTOS;
  
  return RC_ERR_NONE;
}

S08 MX25::deinit (void)
{
  S08 sErr = BSP_OS::semDel(&sem);
  return sErr;
}


S08 MX25::lock (void)
{
  if ((sem == 0) || (&sem == 0)) {
    return RC_ERR_PARAM;
  }
  
  if (osSemaphoreWait(sem, osWaitForever) == osOK )
    return RC_ERR_NONE;
  else 
    return RC_ERR_TIMEOUT;
  
}

S08 MX25::unlock (void)
{
  if ((sem == 0) ||  (&sem == 0)) {
    return RC_ERR_PARAM;
  }
  
  osSemaphoreRelease (sem);
  return RC_ERR_NONE;
}

S08 MX25::get_dev_inf(flash_id_ts * p_inf)
{
  S08 sErr;
  lock(); 
  bsp_spi->select();
  sErr = bsp_spi->command(MX25_CMD_RDID, 0, 0, SPI_READ, temp_buf, 3);
  bsp_spi->unselect();
  p_inf->man_id = temp_buf[0];
  p_inf->type = temp_buf[1];
  p_inf->density = temp_buf[2];
  unlock();
  return sErr;
}

S08 MX25::read (U32 addr, U08* p_buf, U16 len)
{
  if (bsp_spi == 0)
    return RC_ERR_ACCESS;
  if (p_buf == 0)
    return RC_ERR_PARAM;
 
  S08 sErr;
  
  lock();
  bsp_spi->select();
  sErr = bsp_spi->command(MX25Q_CMD_PAGE_FAST_READ, addr, 1, SPI_READ, p_buf, len);
  bsp_spi->unselect();
  unlock();
  
  return sErr;
}

S08 MX25::wr_page (U32 addr, U08* p_buf, U16 len)
{
  if (bsp_spi == 0)
    return RC_ERR_ACCESS;
  if (p_buf == 0)
    return RC_ERR_PARAM;
  
  S08 sErr;
  
  lock();
  wr_enable();
  U08 reg = rd_sreg();
  if (!(reg & MX25_SREG_LATCH_EN))
  {
    unlock();
    return RC_ERR_HW;
  }
  
  bsp_spi->select();
  sErr = bsp_spi->command(MX25_CMD_PAGE_PROG, addr, 0, SPI_WRITE, p_buf, len);
  bsp_spi->unselect();
  
  if (sErr != RC_ERR_NONE)
  {
    unlock();
    return RC_ERR_DATA;
  }
  
  sErr = wait_prog();
  
  unlock();
  
  return sErr;
}

S08 MX25::block_erase (U32 addr)
{
  S08 sErr;
  
  lock();
  wr_enable();
  U08 reg = rd_sreg();
  if (!(reg & MX25_SREG_LATCH_EN))
  {
    unlock();
    return RC_ERR_HW;
  }
  bsp_spi->select();
  sErr = bsp_spi->command(MX25Q_CMD_BLOCK_ERASE, addr, 0, SPI_CMD);
  bsp_spi->unselect();
  if (sErr != RC_ERR_NONE)
  {
    unlock();
    return RC_ERR_DATA;
  }
  sErr = wait_prog();  
  unlock();
  
  return sErr;
}

S08 MX25::sector_erase (U32 addr)
{
  S08 sErr;
  
  lock();
  
  
  wr_enable();
  
  U08 reg = rd_sreg();
  if (!(reg & MX25_SREG_LATCH_EN))
  {
    unlock();
    return RC_ERR_HW;
  }
  
  bsp_spi->select();
  sErr = bsp_spi->command(MX25Q_CMD_SECTOR_ERASE, addr, 0, SPI_CMD);
  bsp_spi->unselect();
  
  if (sErr != RC_ERR_NONE)
  {
    unlock();
    return RC_ERR_DATA;
  }
  
  sErr = wait_prog();
  
  unlock();
  
  return sErr;
}

S08 MX25::wr_enable (void)
{
  S08 sErr;
  bsp_spi->select();
  sErr = bsp_spi->command(MX25Q_CMD_WR_ENABLE, 0, 0, SPI_CMD);
  bsp_spi->unselect();
  return sErr;
}

S08 MX25::wait_prog (void)
{
  while (rd_sreg() & MX25_SREG_BUSY)
  {
    osDelay(1);
  }
  
  return RC_ERR_NONE;
}

S08 MX25::wait_erase (void)
{
  while (rd_sreg() & MX25_SREG_BUSY)
  {
    osDelay(1);
  }
  
  return RC_ERR_NONE;
}

U08 MX25::rd_sreg (void)
{
  if (bsp_spi == 0)
    return 0xFF;
  
  bsp_spi->select();
  S08 sErr = bsp_spi->command(MX25_CMD_RD_SREG, 0, 0, SPI_READ, temp_buf, 1);
  bsp_spi->unselect();

  if (sErr != RC_ERR_NONE)
  {
    return 0xFF;
  }
  
  return temp_buf[0];
}

S08 MX25::detect (void) 
{
  S08 sErr;
  flash_id_ts dev_inf;
  sErr = get_dev_inf (&dev_inf);
  if (sErr != RC_ERR_NONE)
  {
    return FALSE;
  }
  
  if (dev_inf.man_id == JEDEC_MACRONIX &&
      dev_inf.type == MX25_MEMORY_TYPE &&
	dev_inf.density == MX25_16MB_DENSITY)
  {  
    return TRUE;
  }
  return FALSE;
}

const mem_part* MX25::partition(mem_part_num_te part)
{
  if (part == PARTNUM_SETTINGS)    
    return &mx25_settings;
  
  return 0;
}