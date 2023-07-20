/*
 * hal_lcd.hpp
 *
 *  Created on: 20 lip 2023
 *      Author: kwarc
 */

#ifndef HAL_LCD_HPP_
#define HAL_LCD_HPP_

#include <drivers/lcd_rk043fn48h.hpp>
#include <drivers/led_gpio.hpp>

namespace hal
{
    class lcd_tft_480x272
    {
    public:
        lcd_tft_480x272(void *framebuff);
        ~lcd_tft_480x272();

        void backlight(bool state);

        size_t width(void);
        size_t height(void);
        size_t max_bpp(void);
    private:
        drivers::lcd_rk043fn48h lcd_drv;
        const drivers::gpio::io bkl_io = { drivers::gpio::port::portk, drivers::gpio::pin::pin3 };
        drivers::led_gpio bkl_drv { bkl_io };
        /* TODO: Add touch controller driver? */
    };
}


#endif /* HAL_LCD_HPP_ */
