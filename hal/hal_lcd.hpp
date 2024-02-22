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
#include <drivers/touch_ft5336.hpp>

namespace hal
{

//---------------------------------------------------------------------------

    template <typename T>
    class display
    {
    public:
        using pixel_t = T;

        display(hal::interface::glcd<pixel_t> *glcd, hal::interface::led *backlight, hal::interface::touch_panel *touch);
        virtual ~display();

        void backlight(bool state);
        void vsync(bool state);
        void set_vsync_callback(const typename hal::interface::glcd<pixel_t>::vsync_cb_t &callback);

        uint16_t width(void) const;
        uint16_t height(void) const;
        uint8_t bpp(void) const;

        void draw_pixel(int16_t x, int16_t y, pixel_t pixel);
        void draw_data(int16_t x0, int16_t y0, int16_t x1, int16_t y1, pixel_t *data);

        bool get_touch(int16_t &x, int16_t &y);
    protected:
        hal::interface::glcd<pixel_t> *glcd_drv;
        hal::interface::led *backlight_drv;
        hal::interface::touch_panel *touch_drv;
    };

//--------------------------------------------------------------------------

namespace displays
{
    class main : public display<drivers::glcd_rk043fn48h::pixel_t>
    {
    public:
        using pixel_t = drivers::glcd_rk043fn48h::pixel_t;
        using fb_t = drivers::glcd_rk043fn48h::framebuffer_t;

        static constexpr bool use_double_framebuf = false;

        main(hal::interface::i2c_proxy &i2c);

        void set_frame_buffer(pixel_t *addr);
        void set_draw_callback(const drivers::glcd_rk043fn48h::draw_cb_t &callback);
        static const std::pair<fb_t&, fb_t&> & get_frame_buffers(void);
    private:
        drivers::glcd_rk043fn48h lcd_drv;
        drivers::led_gpio backlight_drv;
        drivers::touch_ft5336 touch_drv;
    };
}

//--------------------------------------------------------------------------

}


#endif /* HAL_LCD_HPP_ */
