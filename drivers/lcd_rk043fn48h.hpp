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

class glcd_rk043fn48h : public hal::interface::glcd<uint16_t>
{
    static constexpr size_t width_px = 480;
    static constexpr size_t height_px = 272;

public:
    using pixel_t = pixel_t;
    using framebuffer_t = std::array<pixel_t, width_px * height_px>;

    glcd_rk043fn48h(const std::array<const gpio::io, 29> &gpios, framebuffer_t &frame_buffer);
    ~glcd_rk043fn48h();

    size_t width(void) override { return width_px; }
    size_t height(void) override { return height_px; }
    size_t bpp(void) override { return 5 + 6 + 5; }

    void draw_pixel(int16_t x, int16_t y, pixel_t pixel) override;
    void draw_data(int16_t x0, int16_t y0, int16_t x1, int16_t y1, pixel_t *data) override;

    void set_vsync_callback(const vsync_cb_t &callback) override;
    void wait_for_vsync(void) const;

    void set_frame_buffer(void *addr);
    void *get_frame_buffer(void) const;


private:
    pixel_t *frame_buffer;
};

}

#endif /* LCD_RK043FN48H_HPP_ */
