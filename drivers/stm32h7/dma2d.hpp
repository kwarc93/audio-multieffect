/*
 * dma2d.hpp
 *
 *  Created on: 25 lip 2023
 *      Author: kwarc
 */

#ifndef STM32F7_DMA2D_HPP_
#define STM32F7_DMA2D_HPP_

#include <cstdint>
#include <cstddef>
#include <functional>
#include <unordered_map>

namespace drivers
{

class dma2d final
{
public:
    dma2d() = delete;

    using transfer_cb_t = std::function<void(void)>;

    enum class mode
    {
        mem_to_mem,
        mem_to_mem_pfc,
        mem_to_mem_blend,
        reg_to_mem
    };

    enum class color
    {
        ARGB8888,
        RGB888,
        RGB565,
        ARGB1555,
        ARGB4444,
        L8,
        AL44,
        AL88,
        L4,
        A8,
        A4
    };

    enum class command
    {
        abort = 0,
        suspend,
        start
    };

    struct transfer_cfg
    {
        const transfer_cb_t &transfer_complete_cb;
        mode transfer_mode;
        color color_mode;
        uint8_t alpha;
        const void *src;
        void *dst;
        uint16_t width, height;
        int16_t x1, y1, x2, y2;
        bool rotate_90_deg;
    };

    static void enable(bool state);

    /**
     * @brief Enables dead time inserted between two consecutive accesses on the AHB master port.
     * @param [in] dead_time - dead time value expressed in the AHB clock cycles.
     * @note  This feature is used to limit the AHB bandwidth usage by DMA2D.
     */
    static void set_ahb_dead_time(uint8_t dead_time);
    static void transfer(const transfer_cfg &cfg);
    static void irq_handler(void);
private:
    static inline transfer_cb_t transfer_callback;

    static const inline std::unordered_map<color, size_t> pixel_size
    {
        { color::ARGB8888, 4 }, { color::RGB888, 4 },
        { color::RGB565, 2 },   { color::ARGB1555, 2 },
        { color::ARGB4444, 2 }, { color::L8, 1 },
        { color::AL44, 1 },     { color::AL88, 2 },
        { color::L4, 1 },       { color::A8, 1 },
        { color::A4, 1 },
    };

    static void set_mode(enum mode mode);
    static void send_command(enum command command);
};

}

#endif /* STM32F7_DMA2D_HPP_ */
