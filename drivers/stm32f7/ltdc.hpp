/*
 * ltdc.hpp
 *
 *  Created on: 20 lip 2023
 *      Author: kwarc
 */

#ifndef STM32F7_LTDC_HPP_
#define STM32F7_LTDC_HPP_

#include <cstdint>

namespace drivers
{

class ltdc final
{
public:

    ltdc() = delete;

    struct cfg
    {
        struct horizontal_cfg
        {
            uint32_t width;
            uint32_t sync;
            bool sync_polarity;
            uint32_t back_porch;
            uint32_t front_porch;
        } h;

        struct vertical_cfg
        {
            uint32_t height;
            uint32_t sync;
            bool sync_polarity;
            uint32_t back_porch;
            uint32_t front_porch;
        } v;

        uint8_t bkgd_col_r, bkgd_col_g, bkgd_col_b;

        bool de_pol;
        bool pixel_clk_pol;
        bool err_irq_enable;
    };

    static void configure(const cfg &cfg);
    static void enable(bool state);

    class layer
    {
        layer() = delete;

        enum class id { layer1, layer2 };

        struct cfg
        {

        };

        static void configure(id layer, const cfg &cfg);
        static void enable(id layer, bool state);
    };

    static void irq_handler(void);
private:
    static void global_toggle(bool state);
};

}

#endif /* STM32F7_LTDC_HPP_ */
