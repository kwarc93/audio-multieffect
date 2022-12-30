/*
 * delay.hpp
 *
 *  Created on: 24 paź 2020
 *      Author: kwarc
 */

#ifndef STM32F7_DELAY_HPP_
#define STM32F7_DELAY_HPP_

#include <cstdint>

#include <cmsis/stm32f7xx.h>

#include <hal/hal_system.hpp>

namespace drivers
{
    class delay final
    {
    public:
        delay() = delete;

        __attribute__((always_inline))
        static inline void ms(uint32_t ms)
        {
            const uint32_t end = DWT->CYCCNT + ms * cycles_per_ms;
            while (DWT->CYCCNT < end);
        }

        __attribute__((always_inline))
        static inline void us(uint32_t us)
        {
            const uint32_t end = DWT->CYCCNT + us * cycles_per_us;
            while (DWT->CYCCNT < end);
        }

        __attribute__((always_inline))
        static inline void clock(uint32_t c)
        {
            const uint32_t end = DWT->CYCCNT + c;
            while (DWT->CYCCNT < end);
        }

        template <uint32_t N>
        __attribute__((always_inline))
        static inline void nop(void)
        {
            nop<N - 1>();
            asm volatile ("MOV R0, R0");
        }
    private:
        static constexpr uint32_t cycles_per_ms = hal::system::system_clock / 1000ul;
        static constexpr uint32_t cycles_per_us = hal::system::system_clock / 1000000ul;
    };

    template <>
    inline void delay::nop<0>(void) {}
}

#endif /* STM32F7_DELAY_HPP_ */
