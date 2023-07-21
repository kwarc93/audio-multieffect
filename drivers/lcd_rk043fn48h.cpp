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

/**
  * @brief  RK043FN48H Size
  */
#define  RK043FN48H_WIDTH    (480)          /* LCD PIXEL WIDTH            */
#define  RK043FN48H_HEIGHT   (272)          /* LCD PIXEL HEIGHT           */

/**
  * @brief  RK043FN48H Timing
  */
#define  RK043FN48H_HSYNC            (1)    /* Horizontal synchronization */
#define  RK043FN48H_HBP              (43)   /* Horizontal back porch      */
#define  RK043FN48H_HFP              (8)    /* Horizontal front porch     */
#define  RK043FN48H_VSYNC            (10)   /* Vertical synchronization   */
#define  RK043FN48H_VBP              (12)   /* Vertical back porch        */
#define  RK043FN48H_VFP              (4)    /* Vertical front porch       */

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
            RK043FN48H_WIDTH,
            RK043FN48H_HSYNC,
            RK043FN48H_HBP,
            RK043FN48H_HFP
        },

        /* Vertical */
        {
            RK043FN48H_HEIGHT,
            RK043FN48H_VSYNC,
            RK043FN48H_VBP,
            RK043FN48H_VFP
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
        0, RK043FN48H_WIDTH,
        0, RK043FN48H_HEIGHT,
        ltdc::layer::pixel_format::RGB565,
        255,
        framebuf,
        RK043FN48H_WIDTH, RK043FN48H_HEIGHT,

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
    return RK043FN48H_WIDTH;
}

size_t lcd_rk043fn48h::height(void)
{
    return RK043FN48H_HEIGHT;
}

size_t lcd_rk043fn48h::max_bpp(void)
{
    return 24;
}
