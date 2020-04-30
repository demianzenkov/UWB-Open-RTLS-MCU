#ifndef __BSP_SPI
#define __BSP_SPI

#include "stm32f2xx_hal.h"
#include "prj_defs.h"
#include "bsp_os.h"

#define MAX_DELAY_SPI  1000

#define SPI_READ          0UL
#define SPI_WRITE         1UL
#define SPI_CMD           2UL


class BSP_SPI
{
  public:
    BSP_SPI(SPI_HandleTypeDef * hspi, U08 nbr);
    
    S08 init ();
    S08 deinit ();
    S08 read (U08* p_buf, U16 len);
    S08 write (U08* p_buf, U16 len);
    S08 transmit_receive (uint8_t *pTxData, uint8_t *pRxData, 
			  U16 len, U16 timeout=1000);
    S08 command (U08 cmd, U32 addr, U08 n_dummy, 
		 U08 rw, U08* p_buf = 0, U16 len = 0);
    void select();
    void unselect();
    S08 lock (void);
    S08 unlock (void);
    
  private:
    SPI_HandleTypeDef * p_hspi;
    U08 nbr;
    OS_SEM sem;
    const U08 dummy_byte = 0x00;
};

#endif /* __BSP_SPI */