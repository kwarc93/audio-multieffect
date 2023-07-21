/*
 * lcd_rk043fn48h.hpp
 *
 *  Created on: 20 lip 2023
 *      Author: kwarc
 */

#ifndef STM32F7_LCD_RK043FN48H_HPP_
#define STM32F7_LCD_RK043FN48H_HPP_

#include <array>
#include <cstddef>

#include <drivers/stm32f7/gpio.hpp>

namespace drivers
{

class lcd_rk043fn48h
{
public:
    lcd_rk043fn48h(const std::array<const gpio::io, 28> &gpios, void *framebuf);
    ~lcd_rk043fn48h();

    void set_framebuf(void *addr);

    static constexpr size_t width(void) { return 480; }
    static constexpr size_t height(void) { return 272; }
    static constexpr size_t bpp(void) { return 16; }
};

}

#endif /* STM32F7_LCD_RK043FN48H_HPP_ */
