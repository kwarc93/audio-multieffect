/*
 * hal_system_impl.hpp
 *
 *  Created on: 16 lis 2024
 *      Author: kwarc
 */

#ifndef STM32H7_HAL_SYSTEM_IMPL_HPP_
#define STM32H7_HAL_SYSTEM_IMPL_HPP_


#include <chrono>
#include <cassert>

#include <cmsis/stm32h7xx.h>

#include <drivers/stm32h7/core.hpp>
#include <drivers/stm32h7/rcc.hpp>
#include <drivers/stm32h7/flash.hpp>
#include <drivers/stm32h7/exti.hpp>
#include <drivers/stm32h7/hsem.hpp>

#include "hal_sdram_impl.hpp"
#include "hal_ipc_impl.hpp"

namespace hal::system
{
    constexpr inline uint32_t hsi_clock = 64000000;
    constexpr inline uint32_t hse_clock = 25000000;
#ifdef CORE_CM7
    constexpr inline uint32_t system_clock = 400000000;
#endif
#ifdef CORE_CM4
    constexpr inline uint32_t system_clock = 200000000;
#endif
    constexpr inline uint32_t systick_freq = 1000;
    volatile inline uint32_t systick = 0;

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

        const uint8_t hsem_id = 0;
        drivers::hsem::init();
#ifdef CORE_CM7
        /* Wait until Cortex-M4 boots and enters in stop mode */
        while (RCC->CR & RCC_CR_D2CKRDY);

        drivers::flash::set_wait_states(system::system_clock / 2);

        drivers::rcc::set_oscillators_values(system::hsi_clock, system::hse_clock);

        const drivers::rcc::pll_cfg pll
        {
            13,
            416,
            0,
            2,
            16,
            16
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

        /* Configure MPU */
        ARM_MPU_Disable();

        /* Configure the MPU as Strongly ordered for not defined regions */
        ARM_MPU_SetRegionEx(0, 0, ARM_MPU_RASR(1, ARM_MPU_AP_NONE, 0, 1, 0, 0, 0x87, ARM_MPU_REGION_SIZE_4GB));

        /* Configure the MPU for FMC control, QSPI, SDRAM and shared RAM to prevent speculative accesses by Cortex-M7 */
        ARM_MPU_SetRegionEx(1, 0xA0000000, ARM_MPU_RASR(1, ARM_MPU_AP_FULL, 0, 1, 0, 1, 0, ARM_MPU_REGION_SIZE_8KB));
        ARM_MPU_SetRegionEx(2, 0x90000000, ARM_MPU_RASR(1, ARM_MPU_AP_FULL, 1, 1, 1, 1, 0, ARM_MPU_REGION_SIZE_64MB));
        ARM_MPU_SetRegionEx(3, 0xD0000000, ARM_MPU_RASR(1, ARM_MPU_AP_FULL, 1, 0, 1, 1, 0, ARM_MPU_REGION_SIZE_8MB));
        ARM_MPU_SetRegionEx(4, 0x38000000, ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 0, 1, 0, 0, 0, ARM_MPU_REGION_SIZE_64KB));

        /* Enable MPU */
        ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);

        /* Enable instruction & data caches */
        SCB_EnableICache();
        SCB_EnableDCache();

        hal::sdram::init();

#ifdef DUAL_CORE_APP
        hal::ipc::init();

        /* Take, then release HSEM 0 in order to notify the Cortex-M4 */
        drivers::hsem::take(hsem_id, 0);
        drivers::hsem::release(hsem_id, 0);

        /* Wait until Cortex-M4 wakes up from stop mode */
        while (!(RCC->CR & RCC_CR_D2CKRDY));
#endif /* DUAL_CORE_APP */
#endif /* CORE_CM7 */

#ifdef CORE_CM4
        /* Configure Cortex-M4 Instruction cache through ART accelerator */
        drivers::rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB1, ART), true);
        MODIFY_REG(ART->CTR, ART_CTR_PCACHEADDR, ((FLASH_BANK2_BASE >> 12U) & 0x000FFF00UL));
        SET_BIT(ART->CTR, ART_CTR_EN);

        /* Activate HSEM notification */
        drivers::hsem::enable_notification(hsem_id);

        drivers::core::enter_stop_mode();

        SystemCoreClock = system::system_clock;

        /* Enable EXTI event (wake-up from stop mode) */
        drivers::exti::configure(true,
                                 drivers::exti::line::line13,
                                 drivers::exti::port::portc,
                                 drivers::exti::mode::event,
                                 drivers::exti::edge::rising,
                                 {});
#endif /* CORE_CM4 */

#ifndef HAL_SYSTEM_RTOS_ENABLED
        /* Set System Tick interrupt */
        SysTick_Config(system::system_clock / system::systick_freq);
#endif /* HAL_SYSTEM_RTOS_ENABLED */
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

#endif /* STM32H7_HAL_SYSTEM_IMPL_HPP_ */
