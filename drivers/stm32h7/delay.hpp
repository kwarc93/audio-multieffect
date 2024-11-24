/*
 * delay.hpp
 *
 *  Created on: 24 paź 2020
 *      Author: kwarc
 */

#ifndef STM32F7_DELAY_HPP_
#define STM32F7_DELAY_HPP_

#include <cstdint>

#include <drivers/stm32h7/core.hpp>

namespace drivers
{
    class delay final
    {
    public:
        delay() = delete;

        static void init(uint32_t system_clock)
        {
            drivers::core::enable_cycles_counter();
            cycles_per_ms = system_clock / 1000ul;
            cycles_per_us = system_clock / 1000000ul;
        }

        __attribute__((always_inline))
        static inline void ms(uint32_t ms)
        {
            const uint32_t start = core::get_cycles_counter();
            const uint32_t cycles = ms * cycles_per_ms;
            while ((core::get_cycles_counter() - start) < cycles);
        }

        __attribute__((always_inline))
        static inline void us(uint32_t us)
        {
            const uint32_t start = core::get_cycles_counter();
            const uint32_t cycles = us * cycles_per_us;
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

        static inline uint32_t cycles_per_ms;
        static inline uint32_t cycles_per_us;
    };

    template <>
    inline void delay::nop<0>(void) {}
}

#endif /* STM32F7_DELAY_HPP_ */
