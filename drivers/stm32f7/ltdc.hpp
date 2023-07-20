/*
 * ltdc.hpp
 *
 *  Created on: 20 lip 2023
 *      Author: kwarc
 */

#ifndef STM32F7_LTDC_HPP_
#define STM32F7_LTDC_HPP_

#include <cstdint>
#include <cstddef>
#include <unordered_map>

namespace drivers
{

class ltdc final
{
public:
    ltdc() = delete;

    struct cfg
    {
        struct horizontal
        {
            uint32_t width;
            uint32_t sync;
            uint32_t back_porch;
            uint32_t front_porch;
        } h;

        struct vertical
        {
            uint32_t height;
            uint32_t sync;
            uint32_t back_porch;
            uint32_t front_porch;
        } v;

        /* Background color (RGB) */
        uint8_t r, g, b;

        struct polarity
        {
            bool hsync;
            bool vsync;
            bool de;
            bool pixel_clk;
        } pol;

        bool err_irq_enable;
    };

    static void configure(const cfg &cfg);
    static void enable(bool state);

    class layer
    {
    public:
        layer() = delete;

        enum class id { layer1, layer2 };

        enum class pixel_format
        {
            ARGB8888, RGB888, RGB565, ARGB1555, ARGB4444, L8, AL44, AL88
        };

        struct cfg
        {
            uint16_t h_start, h_stop;
            uint16_t v_start, v_stop;
            pixel_format pix_fmt;
            uint8_t const_alpha_blend;
            void *frame_buf_addr;
            uint16_t frame_buf_width, frame_buf_height;

            /* Background color (ARGB) */
            uint8_t a, r, g, b;
        };

        static void configure(id layer, const cfg &cfg);
        static void enable(id layer, bool layer_enable, bool color_keying_enable, bool clut_enable);
        static void set_framebuf_addr(id layer, void *addr);
    private:
        static const inline std::unordered_map<pixel_format, size_t> pixel_size
        {
            { pixel_format::ARGB8888, 4 }, { pixel_format::RGB888, 3 },
            { pixel_format::RGB565, 2 },   { pixel_format::ARGB1555, 2 },
            { pixel_format::ARGB4444, 2 }, { pixel_format::L8, 1 },
            { pixel_format::AL44, 1 },     { pixel_format::AL88, 2 },
        };

    };

    static void irq_handler(void);
private:
    static void global_toggle(bool state);
};

}

#endif /* STM32F7_LTDC_HPP_ */
