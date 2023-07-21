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

    size_t width(void) const;
    size_t height(void) const;
    size_t max_bpp(void) const;
    void set_framebuf(void *addr);
};

}

#endif /* STM32F7_LCD_RK043FN48H_HPP_ */
