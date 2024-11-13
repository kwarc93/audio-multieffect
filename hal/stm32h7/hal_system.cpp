/*
 * hal.cpp
 *
 *  Created on: 20 paź 2020
 *      Author: kwarc
 */

#include "hal_system.hpp"
#include "hal_sdram.hpp"

#include <cassert>

#include <cmsis/stm32h7xx.h>

#include <drivers/stm32h7/core.hpp>
#include <drivers/stm32h7/rcc.hpp>
#include <drivers/stm32h7/flash.hpp>
#include <drivers/stm32h7/exti.hpp>

using namespace hal;

//-----------------------------------------------------------------------------

volatile uint32_t system::systick = 0;

//-----------------------------------------------------------------------------

void system::init(void)
{
    /* Number of group priorities: 16, subpriorities: 16. */
    NVIC_SetPriorityGrouping(0x07 - __NVIC_PRIO_BITS);

#ifdef CORE_CM7
    /* Wait until Cortex-M4 boots and enters in stop mode */
    while (RCC->CR & RCC_CR_D2CKRDY);
#endif

    drivers::core::enable_cycles_counter();

    drivers::flash::set_wait_states(system::system_clock / 2);

    drivers::rcc::set_oscillators_values(system::hsi_clock, system::hse_clock);

    const drivers::rcc::pll_cfg pll
    {
        5,
        160,
        2,
        4,
        2
    };

    const drivers::rcc::bus_presc presc
    {
        RCC_D1CFGR_D1CPRE_DIV1,
        RCC_D1CFGR_HPRE_DIV2,
        RCC_D2CFGR_D2PPRE1_DIV2,
        RCC_D2CFGR_D2PPRE2_DIV2,
        RCC_D3CFGR_D3PPRE_DIV2
    };

    drivers::rcc::set_main_pll(RCC_PLLCKSELR_PLLSRC_HSE, pll, presc);

    assert(drivers::rcc::get_sysclk_freq() == system::system_clock);

    SystemCoreClock = system::system_clock;

#ifndef HAL_SYSTEM_RTOS_ENABLED
    /* Set System Tick interrupt */
    SysTick_Config(system::system_clock / system::systick_freq);
#endif

#ifdef CORE_CM7
    /* Enable instruction & data caches */
    SCB_EnableICache();
    SCB_EnableDCache();
#endif

    hal::sdram::init();

    /* Enable EXTI event (wake-up from stop mode) */
    drivers::exti::configure(true,
                             drivers::exti::line::line13,
                             drivers::exti::port::portc,
                             drivers::exti::mode::event,
                             drivers::exti::edge::rising,
                             {});
}

void system::sleep(void)
{
    drivers::core::enter_sleep_mode();
}

void system::stop(void)
{
    drivers::core::enter_stop_mode();
}

void system::reset(void)
{
    NVIC_SystemReset();
}

