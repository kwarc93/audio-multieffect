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
#include <functional>

#include <hal_interface.hpp>

#include <drivers/stm32.hpp>

namespace drivers
{

class glcd_rk043fn48h : public hal::interface::glcd<uint16_t>
{
private:
    static constexpr bool use_dma2d = true;
    static constexpr bool use_ltdc_irq = true;
    static constexpr uint16_t width_px = 480;
    static constexpr uint16_t height_px = 272;
    static constexpr uint8_t bits_per_px = 5 + 6 + 5; // RGB565

public:
    using pixel_t = pixel_t;
    using framebuffer_t = std::array<pixel_t, width_px * height_px>;
    using draw_cb_t = std::function<void(void)>;

    glcd_rk043fn48h(const gpio::io en_io, const std::array<const gpio::io, 28> &ltdc_ios, framebuffer_t &frame_buffer, bool portrait_mode = false);
    ~glcd_rk043fn48h();

    uint16_t width(void) override { return this->portrait_mode ? height_px : width_px; }
    uint16_t height(void) override { return this->portrait_mode ? width_px : height_px; }
    uint8_t bpp(void) override { return bits_per_px; }

    void draw_pixel(int16_t x, int16_t y, pixel_t pixel) override;
    void draw_data(int16_t x0, int16_t y0, int16_t x1, int16_t y1, pixel_t *data) override;
    void set_draw_callback(const draw_cb_t &callback);

    void enable_vsync(bool value) override;
    void set_vsync_callback(const vsync_cb_t &callback) override;

    void set_frame_buffer(pixel_t *addr);
    pixel_t *get_frame_buffer(void) const;

private:
    gpio::io enable_io;
    pixel_t *frame_buffer;
    draw_cb_t draw_callback;
    volatile bool vsync;
    bool vsync_enabled;
    bool portrait_mode;
};

}

#endif /* LCD_RK043FN48H_HPP_ */
