/*
 * core.cpp
 *
 *  Created on: 28 paź 2020
 *      Author: kwarc
 */

#include "core.hpp"

#include <cmsis/stm32h7xx.h>

using namespace drivers;

void core::enable_cycles_counter(void)
{
    /* Current inplementation: DWT-based cycles counter.
     * Not all Cortex-M cores have it so SysTick-based cycles
     * counter would be more generic (but has lower, 24bit resolution). */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
#ifdef CORE_CM7
    DWT->LAR = 0xC5ACCE55;
#endif /* CORE_CM7 */
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

core::core_id core::get_current_cpu_id(void)
{
    return (((SCB->CPUID & 0x000000F0U) >> 4) == 0x7U) ? core_id::cortex_m7 : core_id::cortex_m4;
}

void core::enter_sleep_mode(void)
{
    __DSB();
    __ISB();
    __WFI();

    /* Exit upon interrupt */
}

void core::enter_stop_mode(bool low_power)
{
    /* Clear pending event */
#ifdef CORE_CM4
        __SEV();
#endif /* CORE_CM4 */
    __WFE();

#ifdef CORE_CM7
    if (low_power)
    {
        /* Set regulator & flash in low power mode (STOP LP-FPD) */
        PWR->CR1 |= PWR_CR1_LPDS | PWR_CR1_FLPS;
    }
    else
    {
        PWR->CR1 &= ~(PWR_CR1_LPDS | PWR_CR1_FLPS);
    }
    /* Keep DSTOP mode when D1/D3 domain enters Deepsleep */
    PWR->CPUCR &= ~(PWR_CPUCR_PDDS_D1 | PWR_CPUCR_PDDS_D3);
#endif /* CORE_CM7 */
#if defined(DUAL_CORE) && defined(CORE_CM4)
    PWR->CPU2CR &= ~(PWR_CPU2CR_PDDS_D2 | PWR_CPU2CR_PDDS_D3);
#endif /* DUAL_CORE && CORE_CM4 */

    /* Set SLEEPDEEP bit of Cortex System Control Register */
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    /* Ensure that all instructions are done before entering STOP mode */
    __DSB();
    __ISB();

    __WFE();

    /* Exit upon interrupt or event and the clear SLEEPDEEP bit */
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
}

core_critical_section::core_critical_section(void)
{
    this->primask = __get_PRIMASK();
    __disable_irq();
}

core_critical_section::~core_critical_section(void)
{
    if (!this->primask)
        __enable_irq();
}

core_temperature_sensor::core_temperature_sensor(void)
{
    /* TODO: Implement internal temperature sensor initialization */
}

float core_temperature_sensor::read_temperature(void)
{
    /* TODO: Implement internal temperature sensor reading */
    return 27.6;
}
