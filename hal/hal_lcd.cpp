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

template class display<drivers::glcd_rk043fn48h::pixel_t>;

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */

template <typename T>
display<T>::display(hal::interface::glcd<T> *glcd, hal::interface::led *backlight, hal::interface::touch_panel *touch) :
glcd_drv {glcd}, backlight_drv {backlight}, touch_drv {touch}
{

}

template <typename T>
display<T>::~display()
{

}

template <typename T>
void display<T>::backlight(bool state)
{
    if (this->backlight_drv)
        this->backlight_drv->set(state);
}

template <typename T>
void display<T>::vsync(bool state)
{
    if (this->glcd_drv)
        this->glcd_drv->enable_vsync(state);
}

template <typename T>
void display<T>::set_vsync_callback(const typename hal::interface::glcd<T>::vsync_cb_t &callback)
{
    if (this->glcd_drv)
        this->glcd_drv->set_vsync_callback(callback);
}


template <typename T>
uint16_t display<T>::width(void) const
{
    if (this->glcd_drv)
        return this->glcd_drv->width();
    else
        return 0;
}

template <typename T>
uint16_t display<T>::height(void) const
{
    if (this->glcd_drv)
        return this->glcd_drv->height();
    else
        return 0;
}

template <typename T>
uint8_t display<T>::bpp(void) const
{
    if (this->glcd_drv)
        return this->glcd_drv->bpp();
    else
        return 0;
}

template <typename T>
void display<T>::draw_pixel(int16_t x, int16_t y, pixel_t pixel)
{
    if (this->glcd_drv)
        this->glcd_drv->draw_pixel(x, y, pixel);
}

template <typename T>
void display<T>::draw_data(int16_t x0, int16_t y0, int16_t x1, int16_t y1, pixel_t *data)
{
    if (this->glcd_drv)
        this->glcd_drv->draw_data(x0, y0, x1, y1, data);
}

template <typename T>
bool display<T>::get_touch(int16_t &x, int16_t &y)
{
    if (this->touch_drv)
        return this->touch_drv->get_touch(x, y);
    else
        return false;
}

//--------------------------------------------------------------------------
/* main display */

using namespace hal::displays;

static constexpr std::array<const drivers::gpio::io, 29> main_lcd_ios =
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

main::main(hal::interface::i2c_proxy &i2c) : display {&lcd_drv, &backlight_drv , &touch_drv},
lcd_drv {main_lcd_ios, get_frame_buffers().first},
backlight_drv {{drivers::gpio::port::portk, drivers::gpio::pin::pin3}},
touch_drv {i2c, drivers::touch_ft5336::i2c_address, {drivers::gpio::port::porti, drivers::gpio::pin::pin13}, drivers::touch_ft5336::orientation::mirror_xy}
{

};

void main::set_frame_buffer(pixel_t *addr)
{
    this->lcd_drv.set_frame_buffer(addr);
}

void main::set_draw_callback(const drivers::glcd_rk043fn48h::draw_cb_t &callback)
{
    this->lcd_drv.set_draw_callback(callback);
}

const std::pair<main::fb_t&, main::fb_t&> & main::get_frame_buffers(void)
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

