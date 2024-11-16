/*
 * hal_lcd_impl.hpp
 *
 *  Created on: 16 lis 2024
 *      Author: kwarc
 */

#ifndef STM32H7_HAL_LCD_IMPL_HPP_
#define STM32H7_HAL_LCD_IMPL_HPP_

#include <drivers/lcd_rk043fn48h.hpp>
#include <drivers/led_gpio.hpp>
#include <drivers/touch_ft5336.hpp>

#include <utility>

//-----------------------------------------------------------------------------

constexpr inline std::array<const drivers::gpio::io, 28> main_lcd_ios
{{
    /* R [0:7] */
    {drivers::gpio::port::portj, drivers::gpio::pin::pin15},
    {drivers::gpio::port::portj, drivers::gpio::pin::pin0},
    {drivers::gpio::port::portj, drivers::gpio::pin::pin1},
    {drivers::gpio::port::porth, drivers::gpio::pin::pin9},
    {drivers::gpio::port::portj, drivers::gpio::pin::pin3},
    {drivers::gpio::port::portj, drivers::gpio::pin::pin4},
    {drivers::gpio::port::portj, drivers::gpio::pin::pin5},
    {drivers::gpio::port::portj, drivers::gpio::pin::pin6},

    /* G [0:7] */
    {drivers::gpio::port::portj, drivers::gpio::pin::pin7},
    {drivers::gpio::port::portj, drivers::gpio::pin::pin8},
    {drivers::gpio::port::portj, drivers::gpio::pin::pin9},
    {drivers::gpio::port::portj, drivers::gpio::pin::pin10},
    {drivers::gpio::port::portj, drivers::gpio::pin::pin11},
    {drivers::gpio::port::porti, drivers::gpio::pin::pin0},
    {drivers::gpio::port::porti, drivers::gpio::pin::pin1},
    {drivers::gpio::port::portk, drivers::gpio::pin::pin2},

    /* B [0:7] */
    {drivers::gpio::port::portj, drivers::gpio::pin::pin12},
    {drivers::gpio::port::portj, drivers::gpio::pin::pin13},
    {drivers::gpio::port::portj, drivers::gpio::pin::pin14},
    {drivers::gpio::port::portj, drivers::gpio::pin::pin15},
    {drivers::gpio::port::portk, drivers::gpio::pin::pin3},
    {drivers::gpio::port::portk, drivers::gpio::pin::pin4},
    {drivers::gpio::port::portk, drivers::gpio::pin::pin5},
    {drivers::gpio::port::portk, drivers::gpio::pin::pin6},

    /* Control lines */
    {drivers::gpio::port::porti, drivers::gpio::pin::pin12}, // HSYNC
    {drivers::gpio::port::porti, drivers::gpio::pin::pin9},  // VSYNC
    {drivers::gpio::port::porti, drivers::gpio::pin::pin14}, // LCD_CLK
    {drivers::gpio::port::portk, drivers::gpio::pin::pin7},  // LCD_DE
}};

constexpr inline drivers::gpio::io main_lcd_en_io {drivers::gpio::port::portd, drivers::gpio::pin::pin7};

//-----------------------------------------------------------------------------

namespace hal::displays
{
    class main : public display<drivers::glcd_rk043fn48h::pixel_t>
    {
    public:
        using pixel_t = drivers::glcd_rk043fn48h::pixel_t;
        using fb_t = drivers::glcd_rk043fn48h::framebuffer_t;

        static constexpr bool use_double_framebuf = false;

        main(hal::interface::i2c_proxy &i2c) :
        display {&lcd_drv, &backlight_drv , &touch_drv},
        lcd_drv {main_lcd_en_io, main_lcd_ios, get_frame_buffers().first},
        backlight_drv {{drivers::gpio::port::portk, drivers::gpio::pin::pin0}},
        touch_drv {i2c, drivers::touch_ft5336::i2c_address, {drivers::gpio::port::portg, drivers::gpio::pin::pin2}, drivers::touch_ft5336::orientation::mirror_xy}
        {

        }

        void set_frame_buffer(pixel_t *addr)
        {
            this->lcd_drv.set_frame_buffer(addr);
        }

        void set_draw_callback(const drivers::glcd_rk043fn48h::draw_cb_t &callback)
        {
            this->lcd_drv.set_draw_callback(callback);
        }

        static const std::pair<fb_t&, fb_t&> & get_frame_buffers(void)
        {
            __attribute__((section(".sdram"))) static fb_t frame_buffer;

            if constexpr (!use_double_framebuf)
            {
                static constexpr std::pair<fb_t&, fb_t&> fb {frame_buffer, frame_buffer};
                return fb;
            }
            else
            {
                __attribute__((section(".sdram"))) static fb_t frame_buffer2;
                static constexpr std::pair<fb_t&, fb_t&> fb {frame_buffer, frame_buffer2};
                return fb;
            }
        }

    private:
        drivers::glcd_rk043fn48h lcd_drv;
        drivers::led_gpio backlight_drv;
        drivers::touch_ft5336 touch_drv;
    };
}

#endif /* STM32H7_HAL_LCD_IMPL_HPP_ */
