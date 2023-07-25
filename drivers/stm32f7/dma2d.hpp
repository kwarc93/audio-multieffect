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

namespace drivers
{

class dma2d final
{
public:
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
        argb8888,
        rgb888,
        rgb565,
        arg1555,
        argb4444,
        l8,
        al44,
        al88,
        l4,
        a8,
        a4
    };

    enum class command
    {
        abort = 0,
        suspend,
        start
    };

    struct transfer_cfg
    {
        transfer_cb_t transfer_complete_cb;
        mode transfer_mode;
        color color_mode;
        uint8_t alpha;
        const void *src;
        void *dst;
        uint16_t x1;
        uint16_t y1;
        uint16_t x2;
        uint16_t y2;
    };

    static void init(void);
    static void deinit(void);

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

    static void set_mode(enum mode mode);
    static void send_command(enum command command);
};

}

#endif /* STM32F7_DMA2D_HPP_ */
