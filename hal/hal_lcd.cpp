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

//-----------------------------------------------------------------------------
/* private */

static constexpr std::array<const drivers::gpio::io, 28> gpios =
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
}};

//-----------------------------------------------------------------------------
/* public */

lcd_tft_480x272::lcd_tft_480x272(void *framebuff) : lcd_drv { gpios , framebuff}
{
    /* Initialize & set LCD display enable pin */
    const drivers::gpio::io lcd_displ {drivers::gpio::port::porti, drivers::gpio::pin::pin12};
    drivers::gpio::configure(lcd_displ);
    drivers::gpio::write(lcd_displ, true);

    /* Enable backlight */
    this->bkl_drv.set(true);
}

lcd_tft_480x272::~lcd_tft_480x272()
{

}

void lcd_tft_480x272::backlight(bool state)
{
    this->bkl_drv.set(state);
}

size_t lcd_tft_480x272::width(void) const
{
    return this->lcd_drv.width();
}

size_t lcd_tft_480x272::height(void) const
{
    return this->lcd_drv.height();
}

size_t lcd_tft_480x272::max_bpp(void) const
{
    return this->lcd_drv.max_bpp();
}
