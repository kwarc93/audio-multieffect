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

template class glcd<drivers::glcd_rk043fn48h::pixel_t>;

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */

template <typename T>
glcd<T>::glcd(hal::interface::glcd<T> *glcd, hal::interface::led *backlight)
{
    this->glcd_drv = glcd;
    this->backlight_drv = backlight;
}

template <typename T>
glcd<T>::~glcd()
{

}

template <typename T>
void glcd<T>::backlight(bool state)
{
    this->backlight_drv->set(state);
}

template <typename T>
uint16_t glcd<T>::width(void) const
{
    return this->glcd_drv->width();
}

template <typename T>
uint16_t glcd<T>::height(void) const
{
    return this->glcd_drv->height();
}

template <typename T>
uint8_t glcd<T>::bpp(void) const
{
    return this->glcd_drv->bpp();
}

template <typename T>
void glcd<T>::draw_pixel(int16_t x, int16_t y, pixel_t pixel)
{
    this->glcd_drv->draw_pixel(x, y, pixel);
}

template <typename T>
void glcd<T>::draw_data(int16_t x0, int16_t y0, int16_t x1, int16_t y1, pixel_t *data)
{
    this->glcd_drv->draw_data(x0, y0, x1, y1, data);
}

template <typename T>
void glcd<T>::enable_vsync(bool state)
{
    this->glcd_drv->enable_vsync(state);
}

