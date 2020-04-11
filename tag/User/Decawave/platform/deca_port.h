/*! ----------------------------------------------------------------------------
 * @file    port.h
 * @brief   HW specific definitions and functions for portability
 *
 * @attention
 *
 * Copyright 2015 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */


#ifndef PORT_H_
#define PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>
//#include "compiler.h"
#include "main.h"
  
#include "stm32l0xx.h"
#include "stm32l0xx_hal.h"

/* DW1000 IRQ (EXTI9_5_IRQ) handler type. */
typedef void (*port_deca_isr_t)(void);

/* DW1000 IRQ handler declaration. */
extern port_deca_isr_t port_deca_isr;

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn port_set_deca_isr()
 *
 * @brief This function is used to install the handling function for DW1000 IRQ.
 *
 * NOTE:
 *   - As EXTI9_5_IRQHandler does not check that port_deca_isr is not null, the user application must ensure that a
 *     proper handler is set by calling this function before any DW1000 IRQ occurs!
 *   - This function makes sure the DW1000 IRQ line is deactivated while the handler is installed.
 *
 * @param deca_isr function pointer to DW1000 interrupt handler to install
 *
 * @return none
 */
void port_set_deca_isr(port_deca_isr_t deca_isr);


/*****************************************************************************************************************//*
**/

 /****************************************************************************//**
  *
  *                                 Types definitions
  *
  *******************************************************************************/
typedef uint64_t        uint64 ;

typedef int64_t         int64 ;


#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

typedef enum
{
    LED_PC6, //LED5
    LED_PC7, //LED6
    LED_PC8, //LED7
    LED_PC9, //LED8
    LED_ALL,
    LEDn
} led_t;

/* ***** MACRO ***** */


#define DECAIRQ_EXTI_IRQn       (EXTI2_3_IRQn)

#define DW1000_RSTn                 DW_RESET_Pin
#define DW1000_RSTn_GPIO            DW_RESET_GPIO_Port


#define DECAIRQ                     DW_IRQn_Pin
#define DECAIRQ_GPIO                DW_IRQn_GPIO_Port

/* ***** MACRO Function ***** */

#define GPIO_ResetBits(x,y)             HAL_GPIO_WritePin(x,y, RESET)
#define GPIO_SetBits(x,y)               HAL_GPIO_WritePin(x,y, SET)
#define GPIO_ReadInputDataBit(x,y)      HAL_GPIO_ReadPin (x,y)


/* NSS pin is SW controllable */
#define port_SPIx_set_chip_select()     HAL_GPIO_WritePin(SPI1_NSS_DWM_GPIO_Port, SPI1_NSS_DWM_Pin, GPIO_PIN_SET)
#define port_SPIx_clear_chip_select()   HAL_GPIO_WritePin(SPI1_NSS_DWM_GPIO_Port, SPI1_NSS_DWM_Pin, GPIO_PIN_RESET)

/* ***** Port Function Prototypes ***** */

void Sleep(uint32_t Delay);
uint32_t portGetTickCnt(void);


void port_wakeup_dw1000(void);
void port_wakeup_dw1000_fast(void);

void port_set_dw1000_slowrate(void);
void port_set_dw1000_fastrate(void);

void process_dwRSTn_irq(void);
void process_deca_irq(void);

void led_on(led_t led);
void led_off(led_t led);

int  peripherals_init(void);
void spi_peripheral_init(void);

void setup_DW1000RSTnIRQ(int enable);

void reset_DW1000(void);

ITStatus EXTI_GetITEnStatus(uint32_t x);

uint32_t port_GetEXT_IRQStatus(void);
uint32_t port_CheckEXT_IRQ(void);
void port_DisableEXT_IRQ(void);
void port_EnableEXT_IRQ(void);
extern uint32_t     HAL_GetTick(void);

#ifdef __cplusplus
}
#endif

#endif /* PORT_H_ */

