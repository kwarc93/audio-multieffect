/*
 * lcd_rk043fn48h.hpp
 *
 *  Created on: 20 lip 2023
 *      Author: kwarc
 */

#ifndef LCD_RK043FN48H_HPP_
#define LCD_RK043FN48H_HPP_

#include <array>
#include <cstddef>

#include <hal/hal_interface.hpp>
#include <drivers/stm32f7/gpio.hpp>

namespace drivers
{

namespace lcd
{
    using pixel_t = uint16_t; // To hold RGB565 format
}

class glcd_rk043fn48h : public hal::interface::glcd<lcd::pixel_t>
{
    static constexpr size_t width_px = 480;
    static constexpr size_t height_px = 272;

public:
    using framebuffer_t = std::array<lcd::pixel_t, width_px * height_px>;

    glcd_rk043fn48h(const std::array<const gpio::io, 29> &gpios, framebuffer_t &frame_buffer);
    ~glcd_rk043fn48h();

    void draw_pixel(int16_t x, int16_t y, lcd::pixel_t pixel) override;
    void draw_data(int16_t x0, int16_t y0, int16_t x1, int16_t y1, lcd::pixel_t *data) override;

    size_t width(void) override { return width_px; }
    size_t height(void) override { return height_px; }
    size_t bpp(void) override { return 5 + 6 + 5; }

private:
    lcd::pixel_t *active_framebuffer;
};

}

#endif /* LCD_RK043FN48H_HPP_ */
