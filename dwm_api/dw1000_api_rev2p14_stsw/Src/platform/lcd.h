/*! ----------------------------------------------------------------------------
 * @file    lcd.h
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

#ifndef _LCD_H_
#define _LCD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "deca_types.h"

#define EVB1000_LCD_SUPPORT             (1)

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
void writetoLCD
(
    uint32       bodylength,
    uint8        rs_enable,
    const uint8 *bodyBuffer
);
#else
#define writetoLCD(a, b, c)
#endif

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
#if (EVB1000_LCD_SUPPORT == 1)
void lcd_display_str(const char *string1);
void lcd_display_str2(const char *string1, const char *string2);
#else
#define lcd_display_str(x)
#define lcd_display_str2(x, y)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _LCD_H_ */
