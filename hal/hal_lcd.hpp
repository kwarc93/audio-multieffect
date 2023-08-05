/*
 * hal_lcd.hpp
 *
 *  Created on: 20 lip 2023
 *      Author: kwarc
 */

#ifndef HAL_LCD_HPP_
#define HAL_LCD_HPP_

#include <utility>
#include <drivers/lcd_rk043fn48h.hpp>
#include <drivers/led_gpio.hpp>

namespace hal
{

//---------------------------------------------------------------------------

    template <typename T>
    class glcd
    {
    public:
        using pixel_t = T;

        glcd(hal::interface::glcd<pixel_t> *glcd, hal::interface::led *backlight);
        virtual ~glcd();

        void backlight(bool state);

        uint16_t width(void) const;
        uint16_t height(void) const;
        uint8_t bpp(void) const;

        void draw_pixel(int16_t x, int16_t y, pixel_t pixel);
        void draw_data(int16_t x0, int16_t y0, int16_t x1, int16_t y1, pixel_t *data);

        void enable_vsync(bool state);

    protected:
        hal::interface::glcd<pixel_t> *glcd_drv;
        hal::interface::led *backlight_drv;
        /* TODO: Add touch controller driver? */
    };

//--------------------------------------------------------------------------

namespace displays
{
    class main : public glcd<drivers::glcd_rk043fn48h::pixel_t>
    {
    public:
        using pixel_t = drivers::glcd_rk043fn48h::pixel_t;
        using fb_t = drivers::glcd_rk043fn48h::framebuffer_t;

        static constexpr bool use_double_framebuf = false;

        main() : glcd{ &lcd_drv, &backlight_drv } {};

        void wait_for_vsync(void) { this->lcd_drv.wait_for_vsync(); };
        void set_frame_buffer(pixel_t *addr) { this->lcd_drv.set_frame_buffer(addr); };
        void set_draw_callback(const drivers::glcd_rk043fn48h::draw_cb_t &callback) { this->lcd_drv.set_draw_callback(callback); };

        static inline const std::pair<fb_t&, fb_t&> & get_frame_buffers(void)
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
        static constexpr std::array<const drivers::gpio::io, 29> lcd_ios =
        {{
            /* R [0:7] */
            {drivers::gpio::port::portj, drivers::gpio::pin::pin15},
            {drivers::gpio::port::portj, drivers::gpio::pin::pin0},
            {drivers::gpio::port::portj, drivers::gpio::pin::pin1},
            {drivers::gpio::port::portj, drivers::gpio::pin::pin2},
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
            {drivers::gpio::port::portk, drivers::gpio::pin::pin0},
            {drivers::gpio::port::portk, drivers::gpio::pin::pin1},
            {drivers::gpio::port::portk, drivers::gpio::pin::pin2},

            /* B [0:7] */
            {drivers::gpio::port::porte, drivers::gpio::pin::pin4},
            {drivers::gpio::port::portj, drivers::gpio::pin::pin13},
            {drivers::gpio::port::portj, drivers::gpio::pin::pin14},
            {drivers::gpio::port::portj, drivers::gpio::pin::pin15},
            {drivers::gpio::port::portg, drivers::gpio::pin::pin12},
            {drivers::gpio::port::portk, drivers::gpio::pin::pin4},
            {drivers::gpio::port::portk, drivers::gpio::pin::pin5},
            {drivers::gpio::port::portk, drivers::gpio::pin::pin6},

            /* Control lines */
            {drivers::gpio::port::porti, drivers::gpio::pin::pin10}, // HSYNC
            {drivers::gpio::port::porti, drivers::gpio::pin::pin9},  // VSYNC
            {drivers::gpio::port::porti, drivers::gpio::pin::pin14}, // LCD_CLK
            {drivers::gpio::port::portk, drivers::gpio::pin::pin7},  // LCD_DE
            {drivers::gpio::port::porti, drivers::gpio::pin::pin12}, // LCD_EN
        }};

        drivers::glcd_rk043fn48h lcd_drv { lcd_ios, get_frame_buffers().first };

        const drivers::gpio::io backlight_io = { drivers::gpio::port::portk, drivers::gpio::pin::pin3 };
        drivers::led_gpio backlight_drv { backlight_io };
    };
}

//--------------------------------------------------------------------------

}


#endif /* HAL_LCD_HPP_ */
