/*
 * lcd_rk043fn48h.cpp
 *
 *  Created on: 20 lip 2023
 *      Author: kwarc
 */

#include "lcd_rk043fn48h.hpp"

#include <cmsis/stm32f7xx.h>

#include <drivers/stm32f7/ltdc.hpp>
#include <drivers/stm32f7/rcc.hpp>

using namespace drivers;

//-----------------------------------------------------------------------------
/* helpers */

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */

lcd_rk043fn48h::lcd_rk043fn48h(const std::array<const drivers::gpio::io, 28> &gpios, void *framebuf)
{
    /* Initialize LTDC GPIOs */
    for (const auto &pin : gpios)
        drivers::gpio::configure(pin, drivers::gpio::mode::af, drivers::gpio::af::af14);

    /*
     * Configure pixel clock for LCD & LTDC
     * @note 1. Pixel clock = LCD total width x LCD total height x refresh rate
     *       2. Pixel clock source is PLLSAI output R
     */
    static const rcc::sai_pll sai_cfg
    {
        228,
        8,
        2,
        5,
        1,
        8 // 9.5 MHz
    };

    rcc::set_sai_pll(sai_cfg);

    static const ltdc::cfg cfg
    {
        /* Horizontal */
        {
            480,
            1,
            43,
            8
        },

        /* Vertical */
        {
            272,
            10,
            12,
            4
        },

        /* Background color (RGB) */
        0, 0, 0,

        /* Signal polarities */
        {
            false,
            false,
            false,
            false
        },

        /* Error IRQ enable */
        true
    };

    ltdc::configure(cfg);

    const auto ltdc_layer = ltdc::layer::id::layer1;

    static const ltdc::layer::cfg layer_cfg
    {
        0, 480,
        0, 272,
        ltdc::layer::pixel_format::RGB565,
        255,
        framebuf,
        480, 272,

        /* Background color (ARGB) */
        0, 0, 0, 0
    };

    ltdc::layer::configure(ltdc_layer, layer_cfg);
    ltdc::layer::enable(ltdc_layer, true, false, false);
    ltdc::enable(true);
}

lcd_rk043fn48h::~lcd_rk043fn48h()
{
    ltdc::enable(false);
}

size_t lcd_rk043fn48h::width(void)
{
    return 480;
}

size_t lcd_rk043fn48h::height(void)
{
    return 272;
}

size_t lcd_rk043fn48h::max_bpp(void)
{
    return 24;
}
