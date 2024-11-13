/*
 * lcd_rk043fn48h.cpp
 *
 *  Created on: 20 lip 2023
 *      Author: kwarc
 */

#include "lcd_rk043fn48h.hpp"

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


//-----------------------------------------------------------------------------
/* public */

glcd_rk043fn48h::glcd_rk043fn48h(const gpio::io en_io, const std::array<const gpio::io, 28> &ltdc_ios, framebuffer_t &frame_buffer, bool portrait_mode)
{
    this->vsync = false;
    this->vsync_enabled = false;
    this->portrait_mode = portrait_mode;
    this->frame_buffer = frame_buffer.data();
    this->enable_io = en_io;

    /* Initialize LTDC GPIOs */
    for (const auto &pin : ltdc_ios)
        gpio::configure(pin, gpio::mode::af, gpio::af::af14);

    /* Initialize & set LCD display enable GPIO */
    gpio::configure(en_io);
    gpio::write(en_io, true);

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
        use_ltdc_irq
    };

    ltdc::configure(cfg);

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
    dma2d::enable(use_dma2d);
}

glcd_rk043fn48h::~glcd_rk043fn48h()
{
    gpio::write(this->enable_io, false);
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
    if (this->vsync_enabled)
        ltdc::wait_for_vsync();

    if constexpr (use_dma2d)
    {
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
    }
    else
    {
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
    }

}

void glcd_rk043fn48h::set_draw_callback(const draw_cb_t &callback)
{
    this->draw_callback = callback;
}

void glcd_rk043fn48h::enable_vsync(bool value)
{
    this->vsync_enabled = value;
}

void glcd_rk043fn48h::set_vsync_callback(const vsync_cb_t &callback)
{
    ltdc::set_vsync_callback(callback);
}

void glcd_rk043fn48h::set_frame_buffer(pixel_t *addr)
{
    if (this->vsync_enabled)
        ltdc::wait_for_vsync();

    ltdc::layer::set_framebuf_addr(ltdc::layer::id::layer1, addr);
    this->frame_buffer = static_cast<pixel_t*>(addr);
}

glcd_rk043fn48h::pixel_t *glcd_rk043fn48h::get_frame_buffer(void) const
{
    return this->frame_buffer;
}


