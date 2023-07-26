/*
 * lcd_rk043fn48h.cpp
 *
 *  Created on: 20 lip 2023
 *      Author: kwarc
 */

#include "lcd_rk043fn48h.hpp"

#include <cmsis/stm32f7xx.h>

#include <drivers/stm32f7/ltdc.hpp>
#include <drivers/stm32f7/dma2d.hpp>
#include <drivers/stm32f7/rcc.hpp>

#include <cstring>

using namespace drivers;

//-----------------------------------------------------------------------------
/* helpers */

/**
  * @brief  RK043FN48H Timings
  */
#define  LCD_RK043FN48H_HSYNC            (1)    /* Horizontal synchronization */
#define  LCD_RK043FN48H_HBP              (43)   /* Horizontal back porch      */
#define  LCD_RK043FN48H_HFP              (8)    /* Horizontal front porch     */
#define  LCD_RK043FN48H_VSYNC            (10)   /* Vertical synchronization   */
#define  LCD_RK043FN48H_VBP              (12)   /* Vertical back porch        */
#define  LCD_RK043FN48H_VFP              (4)    /* Vertical front porch       */

//-----------------------------------------------------------------------------
/* private */

#define LCD_RK043FN48H_USE_DMA2D        (1)
#define LCD_RK043FN48H_USE_VSYNC_IRQ    (0)

//-----------------------------------------------------------------------------
/* public */

glcd_rk043fn48h::glcd_rk043fn48h(const std::array<const drivers::gpio::io, 29> &ios, framebuffer_t &frame_buffer, bool portrait_mode)
{
    this->vsync = false;
    this->vsync_enabled = false;
    this->portrait_mode = portrait_mode;
    this->frame_buffer = frame_buffer.data();

    /* Initialize LTDC GPIOs */
    for (const auto &pin : ios)
        drivers::gpio::configure(pin, drivers::gpio::mode::af, drivers::gpio::af::af14);

    /* Initialize & set LCD display enable GPIO */
    const drivers::gpio::io lcd_displ {ios.back()};
    drivers::gpio::configure(ios.back());
    drivers::gpio::write(lcd_displ, true);

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
            this->width_px,
            LCD_RK043FN48H_HSYNC,
            LCD_RK043FN48H_HBP,
            LCD_RK043FN48H_HFP
        },

        /* Vertical */
        {
            this->height_px,
            LCD_RK043FN48H_VSYNC,
            LCD_RK043FN48H_VBP,
            LCD_RK043FN48H_VFP
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

        /* IRQ enable */
        LCD_RK043FN48H_USE_VSYNC_IRQ
    };

    ltdc::configure(cfg);
    ltdc::set_vsync_callback([this](void){ this->vsync = true; });

    const auto ltdc_layer = ltdc::layer::id::layer1;

    static const ltdc::layer::cfg layer_cfg
    {
        0, this->width_px,
        0, this->height_px,
        ltdc::layer::pixel_format::RGB565,
        255,
        frame_buffer.data(),
        this->width_px, this->height_px,

        /* Background color (ARGB) */
        0, 0, 0, 0
    };

    ltdc::layer::configure(ltdc_layer, layer_cfg);
    ltdc::layer::enable(ltdc_layer, true, false, false);
    ltdc::enable(true);
#if LCD_RK043FN48H_USE_DMA2D
    dma2d::enable(true);
#endif
}

glcd_rk043fn48h::~glcd_rk043fn48h()
{
    dma2d::enable(false);
    ltdc::enable(false);
}

void glcd_rk043fn48h::draw_pixel(int16_t x, int16_t y, pixel_t pixel)
{
    if (this->portrait_mode)
    {
        int16_t tmp = y;
        y = this->height_px - x - 1;
        x = tmp;
    }

    this->frame_buffer[y * this->width_px + x] = pixel;
}

void glcd_rk043fn48h::draw_data(int16_t x0, int16_t y0, int16_t x1, int16_t y1, pixel_t *data)
{
    this->wait_for_vsync();

#if LCD_RK043FN48H_USE_DMA2D
    const dma2d::transfer_cfg cfg
    {
        this->draw_callback,
        dma2d::mode::mem_to_mem,
        dma2d::color::RGB565,
        255,
        data,
        this->frame_buffer,
        this->width(), this->height(),
        x0, y0, x1, y1,
        this->portrait_mode
    };

    dma2d::transfer(cfg);
#else
    if (!this->portrait_mode)
    {
        const int16_t w = x1 - x0 + 1;
        for (int16_t y = y0; y <= y1 && y < static_cast<int16_t>(this->height()); y++)
        {
            memcpy(&this->frame_buffer[y * this->width() + x0], data, w * sizeof(pixel_t));
            data += w;
        }
    }
    else
    {
        for (int16_t y = y0; y <= y1 && y < static_cast<int16_t>(this->height()); y++)
            for (int16_t x = x0; x <= x1 && x < static_cast<int16_t>(this->width()); x++)
                this->draw_pixel(x, y, *data++);
    }

    if (this->draw_callback)
        this->draw_callback();
#endif
}

void glcd_rk043fn48h::set_draw_callback(const draw_cb_t &callback)
{
    this->draw_callback = callback;
}

void glcd_rk043fn48h::enable_vsync(bool state)
{
    this->vsync_enabled = state;
}

void glcd_rk043fn48h::wait_for_vsync(void)
{
    if (this->vsync_enabled)
    {
#if LCD_RK043FN48H_USE_VSYNC_IRQ
        while(!this->vsync);
        this->vsync = false;
#else
        ltdc::wait_for_vsync();
#endif
    }
}

void glcd_rk043fn48h::set_frame_buffer(pixel_t *addr)
{
    ltdc::layer::set_framebuf_addr(ltdc::layer::id::layer1, addr);
    this->frame_buffer = static_cast<pixel_t*>(addr);
}

glcd_rk043fn48h::pixel_t *glcd_rk043fn48h::get_frame_buffer(void) const
{
    return this->frame_buffer;
}


