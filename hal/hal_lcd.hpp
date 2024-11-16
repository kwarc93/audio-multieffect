/*
 * hal_lcd.hpp
 *
 *  Created on: 20 lip 2023
 *      Author: kwarc
 */

#ifndef HAL_LCD_HPP_
#define HAL_LCD_HPP_

#include <hal_interface.hpp>

namespace hal
{
    template <typename T>
    class display
    {
    public:
        using pixel_t = T;

        display(hal::interface::glcd<pixel_t> *glcd, hal::interface::led *backlight, hal::interface::touch_panel *touch) :
        glcd_drv {glcd}, backlight_drv {backlight}, touch_drv {touch}
        {

        }

        virtual ~display()
        {

        }

        void backlight(bool state)
        {
            if (this->backlight_drv)
                this->backlight_drv->set(state);
        }

        void vsync(bool state)
        {
            if (this->glcd_drv)
                this->glcd_drv->enable_vsync(state);
        }

        void set_vsync_callback(const typename hal::interface::glcd<pixel_t>::vsync_cb_t &callback)
        {
            if (this->glcd_drv)
                this->glcd_drv->set_vsync_callback(callback);
        }

        uint16_t width(void) const
        {
            if (this->glcd_drv)
                return this->glcd_drv->width();
            else
                return 0;
        }

        uint16_t height(void) const
        {
            if (this->glcd_drv)
                return this->glcd_drv->height();
            else
                return 0;
        }

        uint8_t bpp(void) const
        {
            if (this->glcd_drv)
                return this->glcd_drv->bpp();
            else
                return 0;
        }

        void draw_pixel(int16_t x, int16_t y, pixel_t pixel)
        {
            if (this->glcd_drv)
                this->glcd_drv->draw_pixel(x, y, pixel);
        }

        void draw_data(int16_t x0, int16_t y0, int16_t x1, int16_t y1, pixel_t *data)
        {
            if (this->glcd_drv)
                this->glcd_drv->draw_data(x0, y0, x1, y1, data);
        }

        bool get_touch(int16_t &x, int16_t &y)
        {
            if (this->touch_drv)
                return this->touch_drv->get_touch(x, y);
            else
                return false;
        }

    protected:
        hal::interface::glcd<pixel_t> *glcd_drv;
        hal::interface::led *backlight_drv;
        hal::interface::touch_panel *touch_drv;
    };
}

#include <hal_lcd_impl.hpp>

#endif /* HAL_LCD_HPP_ */
