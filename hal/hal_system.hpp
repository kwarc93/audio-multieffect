/*
 * hal.hpp
 *
 *  Created on: 20 paź 2020
 *      Author: kwarc
 */

#ifndef HAL_SYSTEM_HPP_
#define HAL_SYSTEM_HPP_

#include <chrono>

#include <cmsis/stm32f7xx.h>

#include <drivers/stm32f7/core.hpp>

#define HAL_SYSTEM_RTOS_ENABLED

namespace hal::system
{
    static constexpr uint32_t hsi_clock = 16000000;
    static constexpr uint32_t hse_clock = 25000000;
    static constexpr uint32_t system_clock = 200000000;
    static constexpr uint32_t systick_freq = 1000;
    extern volatile  uint32_t systick;

    /* Custom implementation of steady_clock */
    struct clock
    {
        typedef std::chrono::milliseconds duration;
        typedef duration::rep rep;
        typedef duration::period period;
        typedef std::chrono::time_point<clock, duration> time_point;

        static constexpr bool is_steady = true;

        static uint32_t cycles(void)
        {
            return drivers::core::get_cycles_counter();
        }

        static time_point now(void) noexcept
        {
            return time_point{ duration{ systick } };
        }

        template<class rep, class period>
        static bool is_elapsed(time_point start, const std::chrono::duration<rep, period>& duration)
        {
            return (start + duration) < now();
        }
    };

    void init(void);
}


#endif /* HAL_SYSTEM_HPP_ */
