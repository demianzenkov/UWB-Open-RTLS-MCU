#include "bsp_spi.h"
#include "main.h"

BSP_SPI::BSP_SPI(SPI_HandleTypeDef * hspi, U08 num)
{
  p_hspi = hspi;
  nbr = num;
}

S08 BSP_SPI::init ()
{
  
  if (sem == NULL)
    BSP_OS::semCreate (&sem, 1, (char*)"sem_spi");
  
  return RC_ERR_NONE;
}


S08 BSP_SPI::deinit ()
{ 
  BSP_OS::semDel(&sem);
  return RC_ERR_NONE;
}

S08 BSP_SPI::read (U08* p_buf, U16 len)
{
  S08 sErr = RC_ERR_NONE;

  if (HAL_SPI_Receive(p_hspi, p_buf, len, MAX_DELAY_SPI) != HAL_OK)
    return RC_ERR_HW;
  
  return sErr;
}


S08 BSP_SPI::write (U08* p_buf, U16 len)
{
  if (HAL_SPI_Transmit(p_hspi, p_buf, len, MAX_DELAY_SPI) != HAL_OK)
    return RC_ERR_HW;

  return RC_ERR_NONE;
}


S08 BSP_SPI::transmit_receive (uint8_t *pTxData, uint8_t *pRxData, U16 len, U16 timeout)
{
  if (HAL_SPI_TransmitReceive(p_hspi, pTxData, pRxData, len, MAX_DELAY_SPI) != HAL_OK)
    return RC_ERR_HW;

  return RC_ERR_NONE;
}


void BSP_SPI::select()
{
  if (this->nbr == 2)
  {
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
  }
}


void BSP_SPI::unselect()
{
  if (this->nbr == 2)
  {
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);
  }
}


S08 BSP_SPI::lock (void)
{
  if (sem)
  {
    S08 sErr = BSP_OS::semWait (&sem, 10.0 * BSP_TICKS_PER_SEC);
    return sErr;
  }
  else
    return RC_ERR_COMMON;
}


S08 BSP_SPI::unlock (void)
{
  if (sem)
  {
    S08 sErr = BSP_OS::semPost (&sem);
    return sErr;
  }
  else
    return RC_ERR_COMMON;
}

S08 BSP_SPI::command (U08 cmd, U32 addr, U08 n_dummy, U08 rw, U08* p_buf, U16 len)
{
  if (cmd)
    if (HAL_SPI_Transmit(p_hspi, (U08 *)&cmd, 1, MAX_DELAY_SPI) != HAL_OK)
      return RC_ERR_HW;
  
  if(addr)
  {
    U08 tmp_adr[3];
    tmp_adr[0] = (addr >> 16) & 0xFF;
    tmp_adr[1] = (addr >> 8) & 0xFF;
    tmp_adr[2] = addr & 0xFF;
     
    if (HAL_SPI_Transmit(p_hspi, tmp_adr, 3, MAX_DELAY_SPI) != HAL_OK)
      return RC_ERR_HW;
  }
  
  if(n_dummy)
    for (int i=0; i<n_dummy; i++)
      if (HAL_SPI_Transmit(p_hspi, (U08 *)&dummy_byte, 1, MAX_DELAY_SPI) != HAL_OK)
	return RC_ERR_HW;
  
  if (p_buf && len)
  {
    if (rw == SPI_READ)
    {
      if (HAL_SPI_Receive(p_hspi, p_buf, len, MAX_DELAY_SPI) != HAL_OK)
        return RC_ERR_HW;
    }
    else if (rw == SPI_WRITE)
    {
      if (HAL_SPI_Transmit(p_hspi, p_buf, len, MAX_DELAY_SPI) != HAL_OK)
        return RC_ERR_HW;
    }
  }
  return RC_ERR_NONE;
}
