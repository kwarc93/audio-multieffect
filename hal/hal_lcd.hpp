/*
 * hal_lcd.hpp
 *
 *  Created on: 20 lip 2023
 *      Author: kwarc
 */

#ifndef HAL_LCD_HPP_
#define HAL_LCD_HPP_

#include <drivers/lcd_rk043fn48h.hpp>

namespace hal::lcd
{
    class lcd_tft_480x272
    {
    public:
        lcd_tft_480x272();
        ~lcd_tft_480x272();
    private:
        drivers::lcd_rk043fn48h drv;
    };
}


#endif /* HAL_LCD_HPP_ */
