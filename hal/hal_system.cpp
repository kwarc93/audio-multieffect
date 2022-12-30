/*
 * hal.cpp
 *
 *  Created on: 20 paź 2020
 *      Author: kwarc
 */

#include "hal_system.hpp"

#include <cassert>

#include <hal/hal_usart.hpp>

#include <drivers/stm32f7/core.hpp>
#include <drivers/stm32f7/rcc.hpp>
#include <drivers/stm32f7/flash.hpp>

using namespace hal;

//-----------------------------------------------------------------------------

void system::init(void)
{
    /* Number of group priorities: 16, subpriorities: 16. */
    NVIC_SetPriorityGrouping(0x07 - __NVIC_PRIO_BITS);

    /* Set System Tick interrupt */
//    SysTick_Config(system::system_clock / system::systick_freq);

    drivers::core::enable_cycles_counter();

    drivers::flash::set_wait_states(system::system_clock);

    drivers::rcc::main_pll pll =
    {
        RCC_PLLCFGR_PLLSRC_HSE,
        15,
        240,
        8,
        2
    };

    drivers::rcc::bus_presc presc =
    {
        RCC_CFGR_HPRE_DIV1,
        RCC_CFGR_PPRE1_DIV2,
        RCC_CFGR_PPRE2_DIV2
    };

    drivers::rcc::set_main_pll(pll, presc);

    assert(drivers::rcc::get_sysclk_freq() == system::system_clock);
}

//-----------------------------------------------------------------------------
/* syscalls */

extern "C" int _write (int fd, char *ptr, int len)
{
    auto &debug = usart::debug::get_instance();
    return debug.write(reinterpret_cast<std::byte*>(ptr), len);
}

extern "C" int _read (int fd, char *ptr, int len)
{
    auto &debug = usart::debug::get_instance();
    return debug.read(reinterpret_cast<std::byte*>(ptr), len);
}

extern "C" void _ttywrch(int ch)
{
    auto &debug = usart::debug::get_instance();
    debug.write(static_cast<std::byte>(ch));
}

//extern "C" void SysTick_Handler(void)
//{
//
//}
