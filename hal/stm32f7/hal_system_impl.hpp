/*
 * hal_system_impl.hpp
 *
 *  Created on: 16 lis 2024
 *      Author: kwarc
 */

#ifndef STM32F7_HAL_SYSTEM_IMPL_HPP_
#define STM32F7_HAL_SYSTEM_IMPL_HPP_


#include <chrono>
#include <cassert>

#include <cmsis/stm32f7xx.h>

#include <drivers/stm32f7/core.hpp>
#include <drivers/stm32f7/rcc.hpp>
#include <drivers/stm32f7/flash.hpp>
#include <drivers/stm32f7/exti.hpp>

#include "hal_sdram_impl.hpp"

namespace hal::system
{
    constexpr inline uint32_t hsi_clock = 16000000;
    constexpr inline uint32_t hse_clock = 25000000;
    constexpr inline uint32_t system_clock = 200000000;
    constexpr inline uint32_t systick_freq = 1000;
    volatile  inline uint32_t systick = 0;

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

    inline void init(void)
    {
        /* Number of group priorities: 16, subpriorities: 16. */
        NVIC_SetPriorityGrouping(0x07 - __NVIC_PRIO_BITS);

        drivers::core::enable_cycles_counter();

        drivers::flash::set_wait_states(system::system_clock);

        drivers::rcc::main_pll pll
        {
            RCC_PLLCFGR_PLLSRC_HSE,
            15,
            240,
            2,
            10
        };

        drivers::rcc::bus_presc presc
        {
            RCC_CFGR_HPRE_DIV1,
            RCC_CFGR_PPRE1_DIV4,
            RCC_CFGR_PPRE2_DIV4
        };

        drivers::rcc::set_main_pll(pll, presc);

        assert(drivers::rcc::get_sysclk_freq() == system::system_clock);

        SystemCoreClock = system::system_clock;

        /* Enable instruction & data caches */
        SCB_EnableICache();
        SCB_EnableDCache();

        hal::sdram::init();

        /* Enable EXTI event (wake-up from stop mode) */
        drivers::exti::configure(true,
                                 drivers::exti::line::line11,
                                 drivers::exti::port::porti,
                                 drivers::exti::mode::event,
                                 drivers::exti::edge::rising,
                                 {});

#ifndef HAL_SYSTEM_RTOS_ENABLED
        /* Set System Tick interrupt */
        SysTick_Config(system::system_clock / system::systick_freq);
#endif
    }

    inline void sleep(void)
    {
        drivers::core::enter_sleep_mode();
    }

    inline void stop(void)
    {
        drivers::core::enter_stop_mode();
    }

    inline void reset(void)
    {
        NVIC_SystemReset();
    }
}

#endif /* STM32F7_HAL_SYSTEM_IMPL_HPP_ */
