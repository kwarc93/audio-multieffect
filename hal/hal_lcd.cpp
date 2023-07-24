/*
 * hal_lcd.cpp
 *
 *  Created on: 20 lip 2023
 *      Author: kwarc
 */

#include "hal_lcd.hpp"

#include <drivers/stm32f7/gpio.hpp>

#include <array>

using namespace hal;


//-----------------------------------------------------------------------------
/* helpers */

template class glcd<drivers::lcd::pixel_t>;

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */

template <typename pixel_t>
glcd<pixel_t>::glcd(hal::interface::glcd<pixel_t> *glcd, hal::interface::led *backlight)
{
    this->glcd_drv = glcd;
    this->backlight_drv = backlight;
}

template <typename pixel_t>
glcd<pixel_t>::~glcd()
{

}

template <typename pixel_t>
void glcd<pixel_t>::backlight(bool state)
{
    this->backlight_drv->set(state);
}

template <typename pixel_t>
size_t glcd<pixel_t>::width(void) const
{
    return this->glcd_drv->width();
}

template <typename pixel_t>
size_t glcd<pixel_t>::height(void) const
{
    return this->glcd_drv->height();
}

template <typename pixel_t>
size_t glcd<pixel_t>::bpp(void) const
{
    return this->glcd_drv->bpp();
}

template <typename pixel_t>
void glcd<pixel_t>::draw_pixel(int16_t x, int16_t y, pixel_t pixel)
{
    this->glcd_drv->draw_pixel(x, y, pixel);
}

template <typename pixel_t>
void glcd<pixel_t>::draw_data(int16_t x0, int16_t y0, int16_t x1, int16_t y1, pixel_t *data)
{
    this->glcd_drv->draw_data(x0, y0, x1, y1, data);
}

