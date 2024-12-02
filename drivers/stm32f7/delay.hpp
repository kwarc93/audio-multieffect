/*
 * delay.hpp
 *
 *  Created on: 24 paź 2020
 *      Author: kwarc
 */

#ifndef STM32F7_DELAY_HPP_
#define STM32F7_DELAY_HPP_

#include <cstdint>

#include <drivers/stm32f7/core.hpp>

namespace drivers
{
class delay final
{
public:
    delay() = delete;

    __attribute__((always_inline))
    static inline void ms(uint32_t ms)
    {
        const uint32_t start = core::get_cycles_counter();
        const uint32_t cycles = ms * (core::clock / 1000ul);
        while ((core::get_cycles_counter() - start) < cycles);
    }

    __attribute__((always_inline))
    static inline void us(uint32_t us)
    {
        const uint32_t start = core::get_cycles_counter();
        const uint32_t cycles = us * (core::clock / 1000000ul);
        while ((core::get_cycles_counter() - start) < cycles);
    }

    __attribute__((always_inline))
    static inline void clock(uint32_t c)
    {
        const uint32_t start = core::get_cycles_counter();
        const uint32_t cycles = c;
        while ((core::get_cycles_counter() - start) < cycles);
    }

    template <uint32_t N>
    __attribute__((always_inline))
    static inline void nop(void)
    {
        nop<N - 1>();
        asm volatile ("MOV R0, R0");
    }
};

    template <>
    inline void delay::nop<0>(void) {}
}

#endif /* STM32F7_DELAY_HPP_ */
