/*! ----------------------------------------------------------------------------
 * @file    deca_spi.c
 * @brief   SPI access functions
 *
 * @attention
 *
 * Copyright 2015 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */


#include "deca_spi.h"
#include "deca_device_api.h"
#include "deca_port.h"
#include "stm32f2xx_hal_def.h"
#include "dwm1000.hpp"
   
extern DWM1000 dwm1000;

/****************************************************************************//**
 *
 *                              DW1000 SPI section
 *
 *******************************************************************************/
/*! ------------------------------------------------------------------------------------------------------------------
 * Function: openspi()
 *
 * Low level abstract function to open and initialise access to the SPI device.
 * returns 0 for success, or -1 for error
 */
int openspi(/*SPI_TypeDef* SPIx*/)
{
    return 0;
} // end openspi()

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: closespi()
 *
 * Low level abstract function to close the the SPI device.
 * returns 0 for success, or -1 for error
 */
int closespi(void)
{
    return 0;
} // end closespi()

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: writetospi()
 *
 * Low level abstract function to write to the SPI
 * Takes two separate byte buffers for write header and write data
 * returns 0 for success
 */

int writetospi(uint16 headerLength, 
	       const uint8 *headerBuffer,
	       uint32 bodyLength, 
	       const uint8 *bodyBuffer)
{
    decaIrqStatus_t  stat ;
    stat = decamutexon() ;

    while (HAL_SPI_GetState(&dwm1000.hspi) != HAL_SPI_STATE_READY);

    HAL_GPIO_WritePin(DW_NSS_GPIO_Port, DW_NSS_Pin, GPIO_PIN_RESET); /**< Put chip select line low */

    HAL_SPI_Transmit(&dwm1000.hspi, (uint8_t *)&headerBuffer[0], headerLength, HAL_MAX_DELAY);    /* Send header in polling mode */
    HAL_SPI_Transmit(&dwm1000.hspi, (uint8_t *)&bodyBuffer[0], bodyLength, HAL_MAX_DELAY);        /* Send data in polling mode */

    HAL_GPIO_WritePin(DW_NSS_GPIO_Port, DW_NSS_Pin, GPIO_PIN_SET); /**< Put chip select line high */

    decamutexoff(stat);

    return 0;
} // end writetospi()


/*! ------------------------------------------------------------------------------------------------------------------
 * Function: readfromspi()
 *
 * Low level abstract function to read from the SPI
 * Takes two separate byte buffers for write header and read data
 * returns the offset into read buffer where first byte of read data may be found,
 * or returns 0
 */

int readfromspi(uint16 headerLength,
                const uint8 *headerBuffer,
                uint32 readlength,
                uint8 *readBuffer)
{
    int i;
    decaIrqStatus_t  stat ;
    stat = decamutexon() ;

    /* Blocking: Check whether previous transfer has been finished */
    while (HAL_SPI_GetState(&dwm1000.hspi) != HAL_SPI_STATE_READY);

    HAL_GPIO_WritePin(DW_NSS_GPIO_Port, DW_NSS_Pin, GPIO_PIN_RESET); /**< Put chip select line low */

    /* Send header */
    for(i=0; i<headerLength; i++)
    {
        HAL_SPI_Transmit(&dwm1000.hspi, (uint8_t*)&headerBuffer[i], 1, 1); //No timeout
    }

    /* for the data buffer use LL functions directly as the HAL SPI read function
     * has issue reading single bytes */
    while(readlength-- > 0)
    {
        /* Wait until TXE flag is set to send data */
        while(__HAL_SPI_GET_FLAG(&dwm1000.hspi, SPI_FLAG_TXE) == RESET)
        {
        }

        dwm1000.hspi.Instance->DR = 0; /* set output to 0 (MOSI), this is necessary for
        e.g. when waking up DW1000 from DEEPSLEEP via dwt_spicswakeup() function.
        */

        /* Wait until RXNE flag is set to read data */
        while(__HAL_SPI_GET_FLAG(&dwm1000.hspi, SPI_FLAG_RXNE) == RESET)
        {
        }

        (*readBuffer++) = dwm1000.hspi.Instance->DR;  //copy data read form (MISO)
    }

    HAL_GPIO_WritePin(DW_NSS_GPIO_Port, DW_NSS_Pin, GPIO_PIN_SET); /**< Put chip select line high */

    decamutexoff(stat);

    return 0;
} // end readfromspi()

/****************************************************************************//**
 *
 *                              END OF DW1000 SPI section
 *
 *******************************************************************************/
