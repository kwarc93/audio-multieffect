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
    class lcd_tft
    {
    public:
        lcd_tft();
        ~lcd_tft();

        void backlight(bool state);

        void set_framebuf(void *addr);
        bool is_double_framebuf(void) const;
        void *get_framebuf_1(void) const;
        void *get_framebuf_2(void) const;

        size_t width(void) const;
        size_t height(void) const;
        size_t bpp(void) const;
    private:
        drivers::lcd_rk043fn48h lcd_drv;
        const drivers::gpio::io bkl_io = { drivers::gpio::port::portk, drivers::gpio::pin::pin3 };
        drivers::led_gpio bkl_drv { bkl_io };
        /* TODO: Add touch controller driver? */
    };
}


#endif /* HAL_LCD_HPP_ */
