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

namespace
{

constexpr size_t framebuf_size = drivers::lcd_rk043fn48h::width()
                               * drivers::lcd_rk043fn48h::height()
                               * drivers::lcd_rk043fn48h::bpp() / 8;

__attribute__((section(".sdram"))) uint32_t framebuf_1[framebuf_size / 4];
__attribute__((section(".sdram"))) uint32_t framebuf_2[framebuf_size / 4];

}

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

lcd_tft::lcd_tft() : lcd_drv { gpios , framebuf_1 }
{
    /* Initialize & set LCD display enable pin */
    const drivers::gpio::io lcd_displ {drivers::gpio::port::porti, drivers::gpio::pin::pin12};
    drivers::gpio::configure(lcd_displ);
    drivers::gpio::write(lcd_displ, true);

    /* Enable backlight */
    this->bkl_drv.set(true);
}

lcd_tft::~lcd_tft()
{

}

void lcd_tft::backlight(bool state)
{
    this->bkl_drv.set(state);
}

void lcd_tft::set_framebuf(void *addr)
{
    this->lcd_drv.set_framebuf(addr);
}

bool lcd_tft::is_double_framebuf(void) const
{
    return true;
}

void *lcd_tft::get_framebuf_1(void) const
{
    return framebuf_1;
}

void *lcd_tft::get_framebuf_2(void) const
{
    return framebuf_2;
}

size_t lcd_tft::width(void) const
{
    return this->lcd_drv.width();
}

size_t lcd_tft::height(void) const
{
    return this->lcd_drv.height();
}

size_t lcd_tft::bpp(void) const
{
    return this->lcd_drv.bpp();
}

