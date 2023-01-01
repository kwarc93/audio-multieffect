/*
 * hal.cpp
 *
 *  Created on: 20 paź 2020
 *      Author: kwarc
 */

#include "hal_system.hpp"

#include <cassert>

#include <drivers/stm32f7/core.hpp>
#include <drivers/stm32f7/rcc.hpp>
#include <drivers/stm32f7/flash.hpp>

using namespace hal;

//-----------------------------------------------------------------------------

volatile uint32_t system::systick = 0;

//-----------------------------------------------------------------------------

void system::init(void)
{
    /* Number of group priorities: 16, subpriorities: 16. */
    NVIC_SetPriorityGrouping(0x07 - __NVIC_PRIO_BITS);

#ifndef HAL_SYSTEM_RTOS_ENABLED
    /* Set System Tick interrupt */
    SysTick_Config(system::system_clock / system::systick_freq);
#endif

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

    SystemCoreClock = system::system_clock;
}

