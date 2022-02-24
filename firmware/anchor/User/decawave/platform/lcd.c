/*! ----------------------------------------------------------------------------
 * @file    lcd.c
 * @brief   EVB1000 LCD screen access functions
 *
 * @attention
 *
 * Copyright 2015 (c) Decawave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author Decawave
 */
#include <string.h>

#include "sleep.h"
#include "port.h"
#include "lcd.h"

extern SPI_HandleTypeDef hspi2; /*clocked from 36MHz*/

#if (EVB1000_LCD_SUPPORT == 1)
/*! ------------------------------------------------------------------------------------------------------------------
 * Function: writetoLCD()
 *
 * Low level abstract function to write data to the LCD display via SPI2 peripheral
 * Takes byte buffer and rs_enable signals
 * or returns -1 if there was an error
 */
void writetoLCD
(
    uint32       bodylength,
    uint8        rs_enable,
    const uint8 *bodyBuffer
);
#else
#define writetoLCD(x)
#endif


#if (EVB1000_LCD_SUPPORT == 1)
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn writetoLCD()
 *
 * @brief this is the low-level function to write data to the LCD display over SPI
 *
 * @param  bodylength  this is the length of the bodyBuffer array in bytes
 * @param  rs_enable   when this is set to 0 the bodyBuffer should contain a command byte - just a single byte,
 *                     when this is 1, the data in the bodyBuffer should contain string to display on the LCD
 * @param  bodyBuffer  array of the bytes of the string to display
 *
 * @return none
 */
#pragma GCC optimize ("O3")
void writetoLCD
(
    uint32       bodylength,
    uint8        rs_enable,
    const uint8 *bodyBuffer
)
{

    while (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY);

    int sleep = 0;

    if(rs_enable)
    {
        port_LCD_RS_set();
    }
    else
    {
        if(bodylength == 1)
        {
            if(bodyBuffer[0] & 0x3) //if this is command = 1 or 2 - execution time is > 1ms
                sleep = 1 ;
        }
        port_LCD_RS_clear();
    }

    port_SPIy_clear_chip_select();  //CS low for SW controllable SPI_NSS

    HAL_SPI_Transmit(&hspi2, (uint8_t*)bodyBuffer , bodylength, HAL_MAX_DELAY);

    port_LCD_RS_clear();
    port_SPIy_set_chip_select();  //CS high for SW controllable SPI_NSS

    if(sleep)
        Sleep(2);
} // end writetoLCD()

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn lcd_display_str()
 *
 * @brief Display a string on the LCD screen.
 * /!\ The string must be 16 chars long maximum!
 *
 * @param  string  the string to display
 *
 * @return none
 */
void lcd_display_str(const char *string1)
{
    uint8 command;
    /* Return cursor home and clear screen. */
    command = 0x2;
    writetoLCD(1, 0, &command);
    command = 0x1;
    writetoLCD(1, 0, &command);
    /* Write the string to display. */
    //writetoLCD(strlen(string), 1, (const uint8 *)string);
    writetoLCD(40, 1, (const uint8 *)string1);
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn lcd_display_str2()
 *
 * @brief Display a string on the LCD screen.
 * /!\ Both of the strings must be 16 chars long maximum!
 *
 * @param  string  the string to display on the 1st line
 * @param  string  the string to display on the second line
 *
 * @return none
 */
void lcd_display_str2(const char *string1, const char *string2)
{
    uint8 command;
    /* Return cursor home and clear screen. */
    command = 0x2;
    writetoLCD(1, 0, &command);
    command = 0x1;
    writetoLCD(1, 0, &command);
    /* Write the strings to display. */
    writetoLCD(40, 1, (const uint8 *)string1); //sending 40 bytes to align the second line
    writetoLCD(16, 1, (const uint8 *)string2); //send the data for second line of the display
}

#endif
